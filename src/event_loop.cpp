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
constexpr double rotation_max_degrees_second = 900.0;
constexpr double rotation_max_rad_second = rotation_max_degrees_second * (pi / 180.0);
constexpr double rotation_rad_millis = rotation_max_rad_second / 1000.0;

constexpr vec3 x_axis = vec3(1.0f, 0.0f, 0.0f);
constexpr vec3 y_axis = vec3(0.0f, 1.0f, 0.0f);

/**
 * how much to pan the 3d function per frame
 */
constexpr GLfloat panning_delta_per_ms = 0.01f;

/**
 * how much to change the 3d function z mult per frame
 */
constexpr GLfloat z_mult_delta_per_ms = 0.1f;

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

EventLoop::EventLoop(shared_ptr<mat4> model, shared_ptr<mat4> view, shared_ptr<mat4> projection,
                     shared_ptr<FunctionParams> function_params)
    : model(model), view(view), projection(projection), function_params(function_params),
      function_params_modified_(false), view_modified_(false), model_modified_(false), start_click(nullopt),
      event_poll_timings(num_event_timings_maintain), rotational_axis_direction(0.0f), rotational_axis(nullopt),
      active_keys(ActiveKeys(monitored_keys)) {
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
                                                                                        const Key &key) const {
    return nullopt;
}

void EventLoop::process_function_mutation_keys(uint64_t start_ticks_ms) {
    using std::get;

    auto const up_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, Key(SDL_SCANCODE_UP));
    auto const down_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, Key(SDL_SCANCODE_DOWN));

    auto const left_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, Key(SDL_SCANCODE_LEFT));
    auto const right_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, Key(SDL_SCANCODE_RIGHT));

    // xor
    if (left_key_timing.has_value() != right_key_timing.has_value()) {

        if (left_key_timing.has_value()) {
            function_params_modified_ = true;
            auto const panning_movement = (get<2>(*left_key_timing) - get<1>(*left_key_timing)) * panning_delta_per_ms;
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
            function_params->z_mult += z_mult_movement;
        }

        if (down_key_timing.has_value()) {
            function_params_modified_ = true;
            auto const z_mult_movement = (get<2>(*down_key_timing) - get<1>(*down_key_timing)) * z_mult_delta_per_ms;
            function_params->z_mult += z_mult_movement;
        }
    }
}

void EventLoop::process_model_mutation_keys(uint64_t start_ticks_ms) {
    using std::get;

    auto const up_key_timing = active_keys.get_key_press_duration(SDL_SCANCODE_W);
    auto const down_key_timing = active_keys.get_key_press_duration(SDL_SCANCODE_S);

    auto const left_key_timing = active_keys.get_key_press_duration(SDL_SCANCODE_A);
    auto const right_key_timing = active_keys.get_key_press_duration(SDL_SCANCODE_D);

    // TODO: remove make_optional call each time, modify in place
    if (up_key_timing.has_value() != down_key_timing.has_value()) {
        if (up_key_timing.has_value()) {
            model_modified_ = true;
            rotational_axis_direction = 1.0f;
            rotational_axis = make_optional(y_axis);
        }

        if (down_key_timing.has_value()) {
            model_modified_ = true;
            rotational_axis = make_optional(y_axis);
            rotational_axis_direction = -1.0f;
        }
    }

    if (left_key_timing.has_value() != right_key_timing.has_value()) {
        if (left_key_timing.has_value()) {
            model_modified_ = true;
            rotational_axis_direction = -1.0f;
            rotational_axis = make_optional(x_axis);
        }

        if (right_key_timing.has_value()) {
            model_modified_ = true;
            rotational_axis_direction = 1.0f;
            rotational_axis = make_optional(x_axis);
        }
    }
}

bool EventLoop::drain_event_queue_should_exit() {
    rotational_axis_direction = 0.0f;
    rotational_axis = nullopt;

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

TickResult EventLoop::tick(uint64_t render_time_ns) {
    auto const stdout = spdlog::get("stdout");

    auto const start_ticks_ms = SDL_GetTicks();
    auto const start_ticks_ns = SDL_GetTicksNS();

    // what ticks ns timestamp to not exceed to maintain fps
    auto const absolute_max_end_ticks_ns = start_ticks_ns + max_sleep_ns_per_tick - render_time_ns;

    // what's the latest ticks ns that can do another sdl event queue drain
    // without likely going over the time allowed to process the frame
    auto end_ticks_ns = absolute_max_end_ticks_ns - event_poll_timings.get_avg();

    auto drain_start_ns = SDL_GetTicksNS();
    if (drain_start_ns >= end_ticks_ns) {
        stdout->debug("skipping input polling this tick");
        // not entirely accurate, is used to prevent a couple of slow input poll loops
        // from locking out all input polling by dropping down the average
        event_poll_timings.add(0);
        SDL_FlushEvents(SDL_EVENT_QUIT + 1, SDL_EVENT_ENUM_PADDING);
        return TickResult{SDL_GetTicks() - start_ticks_ms, false, true};
    }

    // TODO: these variables are not correct
    // instead need to access keyboard state at the start each time
    // to determine what actually is pressed / not pressed

    while ((drain_start_ns = SDL_GetTicksNS()) < end_ticks_ns) {
        if (drain_event_queue_should_exit()) {
            return TickResult{SDL_GetTicks() - start_ticks_ms, true, false};
        }

        auto const drain_end_ns = SDL_GetTicksNS();

        // adjust timing expectations based on latest data
        event_poll_timings.add(drain_end_ns - drain_start_ns);
        end_ticks_ns = absolute_max_end_ticks_ns - event_poll_timings.get_avg();
    }

    auto const elapsed_millis = SDL_GetTicks() - start_ticks_ms;

    // TODO: track this overhead separately and use to compute how much input to process
    // per tick
    process_function_mutation_keys(start_ticks_ms);
    process_model_mutation_keys(start_ticks_ms);

    if (model_modified_ && rotational_axis.has_value()) {
        quat const current(*model);

        auto const rotations_rads = static_cast<float>(rotation_rad_millis * static_cast<double>(elapsed_millis));
        quat const rotation = angleAxis(rotations_rads * rotational_axis_direction, rotational_axis.value());
        quat const new_model_orientation = rotation * current;

        stdout->debug("will update model matrix from {0} to {1}", current, new_model_orientation);
        *model = toMat4(new_model_orientation);
    }

    return TickResult(SDL_GetTicks() - start_ticks_ms, false, false);
}
