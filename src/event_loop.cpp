#include "event_loop.hpp"
#include "active_keys.hpp"
#include "consts.hpp"
#include "formatters.hpp"
#include "function_params.hpp"
#include "key.hpp"
#include "mouse_loc.hpp"
#include "tick_result.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <initializer_list>
#include <memory>
#include <optional>
#include <tuple>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using std::initializer_list;
using std::make_optional;
using std::make_tuple;
using std::nullopt;
using std::optional;
using std::shared_ptr;
using std::size_t;
using std::tuple;

using glm::angleAxis;
using glm::mat4;
using glm::quat;
using glm::toMat4;
using glm::vec3;

constexpr double pi = 3.1415926;

/**
 * how much to rotate / orbit in any direction
 * NOTE: manually tuned
 */
constexpr double rotation_max_degrees_second = 20.0;
constexpr double rotation_max_rad_second = rotation_max_degrees_second * (pi / 180.0);
constexpr double rotation_rad_millis = rotation_max_rad_second / 1000.0;

constexpr vec3 x_axis = vec3(1.0f, 0.0f, 0.0f);
constexpr vec3 y_axis = vec3(0.0f, 1.0f, 0.0f);

/**
 * how much to pan the 3d function per frame
 * NOTE: manually tuned
 */
constexpr GLfloat panning_delta_per_ms = 0.0005f;

/**
 * how much to change the 3d function z mult per frame
 * NOTE: manually tuned
 */
constexpr GLfloat z_mult_delta_per_ms = 0.001f;

constexpr initializer_list<SDL_Scancode> function_param_mutation_keys = {
    SDL_SCANCODE_UP,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_RIGHT,
};

constexpr initializer_list<SDL_Scancode> model_mutation_keys = {
    SDL_SCANCODE_W,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
};

// TODO: constexpr combine these
constexpr initializer_list<SDL_Scancode> monitored_keys = {
    SDL_SCANCODE_W,  SDL_SCANCODE_A,    SDL_SCANCODE_S,    SDL_SCANCODE_D,
    SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
};

/**
 * the number of event timings to hold on to
 * for calculating historic timings of how long it takes
 * to drain the sdl event queue, in ns
 */
constexpr size_t num_event_timings_maintain = 10;

namespace {
auto logger = spdlog::stdout_color_mt("event_loop");
optional<vec3> no_rotational_axis{};
optional<vec3> x_rotation_axis{x_axis};
optional<vec3> y_rotation_axis{y_axis};
} // namespace

EventLoop::EventLoop(shared_ptr<mat4> model, shared_ptr<mat4> view, shared_ptr<mat4> projection,
                     shared_ptr<FunctionParams> function_params)
    : model(model), view(view), projection(projection), function_params(function_params),
      function_params_modified_(false), view_modified_(false), model_modified_(false), start_click(nullopt),
      event_poll_timings(num_event_timings_maintain), rotational_axis_direction(0.0f),
      rotational_axis(no_rotational_axis), active_keys(ActiveKeys(monitored_keys)) {
}

bool EventLoop::function_params_modified() const {
    return function_params_modified_;
}

bool EventLoop::view_modified() const {
    return view_modified_;
}

bool EventLoop::model_modified() const {
    return model_modified_;
}

optional<tuple<Key, uint64_t, uint64_t>> EventLoop::which_key_variant_was_pressed_since(uint64_t start_ms,
                                                                                        uint64_t end_ms,
                                                                                        const Key &key) const {
    using std::get;

    // TODO this is a bit of a leaky abstraction because eventloop knows that activekeys only
    // checks for the shift modifier, need to restructure the code to remove this implicit coupling

    auto const key_only = key.without_mods();
    auto const key_with_shift = key.copy_shifted();
    auto const maybe_this_key_timing = active_keys.maybe_get_key(key_only);
    auto const maybe_shift_key_timing = active_keys.maybe_get_key(key_with_shift);

    // were either of the keys pressed at all?
    if (!maybe_shift_key_timing.has_value() && !maybe_this_key_timing.has_value()) {
        return nullopt;
    }

    // xor
    if (maybe_shift_key_timing.has_value() != maybe_this_key_timing.has_value()) {

        if (maybe_shift_key_timing.has_value()) {
            auto const shift_key_timing = *maybe_shift_key_timing;
            auto const maybe_shift_key_end_ms = get<1>(shift_key_timing);
            auto const start_time_ms = std::max(get<0>(shift_key_timing), start_ms);

            // button was released before the start time under consideration
            if (maybe_shift_key_end_ms.has_value() && maybe_shift_key_end_ms.value() < start_ms) {
                return nullopt;
            }

            // button is still held down
            if (!maybe_shift_key_end_ms.has_value()) {
                return make_optional(make_tuple(key_with_shift, start_time_ms, end_ms));
            }
            else {
                return make_optional(make_tuple(key_with_shift, start_time_ms, *maybe_shift_key_end_ms));
            }
        }
        else {
            auto const this_key_timing = *maybe_this_key_timing;
            auto const maybe_shift_key_end_ms = get<1>(this_key_timing);
            auto const start_time_ms = std::max(get<0>(this_key_timing), start_ms);

            // button was released before the start time under consideration
            if (maybe_shift_key_end_ms.has_value() && maybe_shift_key_end_ms.value() < start_ms) {
                return nullopt;
            }

            // button is still held down
            if (!maybe_shift_key_end_ms.has_value()) {
                return make_optional(make_tuple(key_with_shift, start_time_ms, end_ms));
            }
            else {
                return make_optional(make_tuple(key_with_shift, start_time_ms, *maybe_shift_key_end_ms));
            }
        }
    }

    // tie-breaker if both keys were pressed
    auto const shift_key_timing = *maybe_shift_key_timing;
    auto const this_key_timing = *maybe_this_key_timing;

    // arbitrary: give shift key precedence if either are still
    // held down at the end of the frame
    if (!get<1>(shift_key_timing).has_value() && !get<1>(this_key_timing).has_value()) {
        return make_optional(make_tuple(key_with_shift, get<0>(shift_key_timing), end_ms));
    }
    else if (get<1>(shift_key_timing).has_value()) {
        return make_optional(make_tuple(key_only, get<0>(shift_key_timing), end_ms));
    }
    else {
        return make_optional(make_tuple(key_with_shift, get<0>(shift_key_timing), end_ms));
    }

    return nullopt;
}

void EventLoop::process_function_mutation_keys(uint64_t start_ticks_ms) {
    using std::get;

    auto const now_ms = SDL_GetTicks();
    auto const up_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_UP));
    auto const down_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_DOWN));

    auto const left_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_LEFT));
    auto const right_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_RIGHT));

    // xor
    if (left_key_timing.has_value() != right_key_timing.has_value()) {

        if (left_key_timing.has_value()) {
            function_params_modified_ = true;
            auto const panning_movement = (get<2>(*left_key_timing) - get<1>(*left_key_timing)) * panning_delta_per_ms;
            ::logger->debug("panning {} in negative direction", panning_movement);
            if (get<0>(*left_key_timing).has_shift()) {
                function_params->y_offset -= panning_movement;
            }
            else {
                function_params->x_offset -= panning_movement;
            }
        }

        if (right_key_timing.has_value()) {
            function_params_modified_ = true;
            auto const panning_movement =
                (get<2>(*right_key_timing) - get<1>(*right_key_timing)) * panning_delta_per_ms;
            ::logger->debug("panning {} in positive direction", panning_movement);
            if (get<0>(*right_key_timing).has_shift()) {
                function_params->y_offset += panning_movement;
            }
            else {
                function_params->x_offset += panning_movement;
            }
        }
    }

    if (up_key_timing.has_value() != down_key_timing.has_value()) {

        if (up_key_timing.has_value()) {
            function_params_modified_ = true;
            auto const z_mult_movement = (get<2>(*up_key_timing) - get<1>(*up_key_timing)) * z_mult_delta_per_ms;
            ::logger->debug("changing z by {} in positive direction", z_mult_movement);
            function_params->z_mult += z_mult_movement;
        }

        if (down_key_timing.has_value()) {
            function_params_modified_ = true;
            auto const z_mult_movement = (get<2>(*down_key_timing) - get<1>(*down_key_timing)) * z_mult_delta_per_ms;
            ::logger->debug("changing z by {} in negative direction", z_mult_movement);
            function_params->z_mult -= z_mult_movement;
        }
    }
}

void EventLoop::process_model_mutation_keys(uint64_t start_ms, uint64_t end_ms) {
    using std::get;

    bool const up_key_timing = active_keys.was_key_pressed_since(SDL_SCANCODE_W, start_ms);
    bool const down_key_timing = active_keys.was_key_pressed_since(SDL_SCANCODE_S, start_ms);

    bool const left_key_timing = active_keys.was_key_pressed_since(SDL_SCANCODE_A, start_ms);
    bool const right_key_timing = active_keys.was_key_pressed_since(SDL_SCANCODE_D, start_ms);

    // TODO: remove make_optional call each time, modify in place
    if (up_key_timing != down_key_timing) {
        if (up_key_timing) {
            model_modified_ = true;
            rotational_axis_direction = 1.0f;
            rotational_axis = y_rotation_axis;
        }

        if (down_key_timing) {
            model_modified_ = true;
            rotational_axis = y_rotation_axis;
            rotational_axis_direction = -1.0f;
        }
    }

    if (left_key_timing != right_key_timing) {
        if (left_key_timing) {
            model_modified_ = true;
            rotational_axis_direction = -1.0f;
            rotational_axis = x_rotation_axis;
        }

        if (right_key_timing) {
            model_modified_ = true;
            rotational_axis_direction = 1.0f;
            rotational_axis = x_rotation_axis;
        }
    }

    if (model_modified_ && rotational_axis.has_value()) {
        quat const current(*model);

        auto const rotations_rads = static_cast<float>(rotation_rad_millis * static_cast<double>(end_ms - start_ms));
        quat const rotation = angleAxis(rotations_rads * rotational_axis_direction, rotational_axis.value());
        quat const new_model_orientation = rotation * current;

        ::logger->debug("will update model matrix from {0} to {1}", current, new_model_orientation);
        *model = toMat4(new_model_orientation);
    }
}

bool EventLoop::drain_event_queue_should_exit() {
    rotational_axis_direction = 0.0f;
    rotational_axis = no_rotational_axis;

    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_EVENT_QUIT) {
            return true;
        }
        else if (evt.type == SDL_EVENT_KEY_UP) {
            active_keys.release_key(Key(evt.key.scancode, evt.key.mod));
        }
        else if (evt.type == SDL_EVENT_KEY_DOWN) {
            if (evt.key.key == SDLK_Q) {
                return true;
            }

            if (start_click.has_value()) {
                continue;
            }

            active_keys.set_key_pressed(Key(evt.key.scancode, evt.key.mod));
        }
        else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            start_click = make_optional<MouseLoc>(evt.motion.x, evt.motion.y);
        }
        else if (evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            start_click = nullopt;
        }
        else if (start_click.has_value() && evt.type == SDL_EVENT_MOUSE_MOTION) {
            // MouseLoc current(evt.motion.x, evt.motion.y);
        }
    }

    return false;
}

TickResult EventLoop::process_frame(uint64_t render_time_ns) {
    auto const start_ticks_ms = SDL_GetTicks();
    auto const start_ticks_ns = SDL_GetTicksNS();

    // what ticks ns timestamp to not exceed to maintain fps
    auto const absolute_max_end_ticks_ns = start_ticks_ns + max_sleep_ns_per_tick - render_time_ns;

    // what's the latest ticks ns that can do another sdl event queue drain
    // without likely going over the time allowed to process the frame
    auto end_ticks_ns = absolute_max_end_ticks_ns - event_poll_timings.get_avg();

    auto drain_start_ns = SDL_GetTicksNS();
    if (drain_start_ns >= end_ticks_ns) {
        ::logger->debug("skipping input polling this tick");
        // not entirely accurate, is used to prevent a couple of slow input poll loops
        // from locking out all input polling by dropping down the average
        event_poll_timings.add(0);
        SDL_FlushEvents(SDL_EVENT_QUIT + 1, SDL_EVENT_ENUM_PADDING);
        return TickResult{SDL_GetTicks() - start_ticks_ms, false, true};
    }

    while ((drain_start_ns = SDL_GetTicksNS()) < end_ticks_ns) {
        if (drain_event_queue_should_exit()) {
            return TickResult{SDL_GetTicks() - start_ticks_ms, true, false};
        }

        auto const drain_end_ns = SDL_GetTicksNS();

        // adjust timing expectations based on latest data
        event_poll_timings.add(drain_end_ns - drain_start_ns);
        end_ticks_ns = absolute_max_end_ticks_ns - event_poll_timings.get_avg();
    }

    // TODO: track this overhead separately and use to compute how much input to process
    // per frame
    process_function_mutation_keys(start_ticks_ms);
    process_model_mutation_keys(start_ticks_ms, SDL_GetTicks());

    return TickResult(SDL_GetTicks() - start_ticks_ms, false, false);
}
