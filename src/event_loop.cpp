#include "event_loop.hpp"
#include "active_keys.hpp"
#include "consts.hpp"
#include "function_params.hpp"
#include "key.hpp"
#include "mouse_loc.hpp"
#include "tessellation_settings.hpp"
#include "tick_result.hpp"

#include <SDL3/SDL_keycode.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <numbers>
#include <optional>
#include <tuple>

#include <SDL3/SDL.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using std::array;
using std::make_optional;
using std::make_tuple;
using std::nullopt;
using std::optional;
using std::shared_ptr;
using std::size_t;
using std::tuple;
using std::numbers::pi_v;

using glm::angleAxis;
using glm::mat4;
using glm::quat;
using glm::toMat4;
using glm::translate;
using glm::vec3;

/**
 * how much to rotate / orbit in any direction
 * NOTE: manually tuned
 */
static const constexpr double rotation_max_degrees_second = 20.0;
static const constexpr double rotation_max_rad_second = rotation_max_degrees_second * (pi_v<double> / 180.0);
static const constexpr double rotation_rad_millis = rotation_max_rad_second / 1000.0;

/**
 * how fast to rotate on the slowed rotation setting
 */
static const constexpr double slowed_rotation_max_degrees_second = 15.0;
static const constexpr double slowed_rotation_max_rad_second = slowed_rotation_max_degrees_second * (pi_v<double> / 180.0);
static const constexpr double slowed_rotation_rad_millis = slowed_rotation_max_rad_second / 1000.0;

static const constexpr vec3 x_axis = vec3(1.0f, 0.0f, 0.0f);
static const constexpr vec3 y_axis = vec3(0.0f, 1.0f, 0.0f);

/**
 * how much to zoom per scroll wheel click / notch? (whatever that is called)
 */
static const constexpr float zoom_amount_scroll_wheel = 0.5f;
static const constexpr vec3 zoom_out = vec3(0.0f, 0.0f, -zoom_amount_scroll_wheel);
static const constexpr vec3 zoom_in = vec3(0.0f, 0.0f, zoom_amount_scroll_wheel);

/**
 * how much to pan the 3d function per frame
 * NOTE: manually tuned
 */
static const constexpr GLfloat panning_delta_per_ms = 0.0005f;

/**
 * how much to change the 3d function z mult per frame
 * NOTE: manually tuned
 */
static const constexpr GLfloat z_mult_delta_per_ms = 0.001f;

/** how long in msec between tess level changes to allow */
static const constexpr uint64_t msec_between_tess_level_changes = 700;

/**
 * how long in msec between switching between wireframe only and mesh view
 * (opengl 4.1 only) */
static const constexpr uint64_t msec_between_toggle_wireframe_changes = 400;

static const constexpr array monitored_keys = {
    Keyish{SDL_SCANCODE_W},  Keyish{SDL_SCANCODE_A},    Keyish{SDL_SCANCODE_S},    Keyish{SDL_SCANCODE_D},
    Keyish{SDL_SCANCODE_UP}, Keyish{SDL_SCANCODE_DOWN}, Keyish{SDL_SCANCODE_LEFT}, Keyish{SDL_SCANCODE_RIGHT},
    Keyish{SDLK_PLUS},       Keyish{SDLK_MINUS},        Keyish{SDL_SCANCODE_E},
};

/**
 * the number of event timings to hold on to
 * for calculating historic timings of how long it takes
 * to drain the sdl event queue, in ns
 */
constexpr size_t num_event_timings_maintain = 10;

EventLoop::EventLoop(shared_ptr<mat4> const &model, shared_ptr<mat4> const &view, shared_ptr<mat4> const &projection,
                     shared_ptr<FunctionParams> const &function_params,
                     shared_ptr<TessellationSettings> const &tessellation_settings)
    : model(model), view(view), projection(projection), function_params(function_params), start_click(nullopt),
      event_poll_timings(num_event_timings_maintain), active_keys(ActiveKeys(monitored_keys)),
      tessellation_settings(tessellation_settings), last_tessellation_change_at_msec(nullopt),
      last_wireframe_only_change_at_msec(nullopt), logger(spdlog::stdout_color_mt("event_loop")),
      err(spdlog::stderr_color_mt("event_loop_err")) {
}

optional<KeyAtTime> EventLoop::which_key_variant_was_pressed_since(uint64_t start_ms, uint64_t end_ms,
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

TickResult EventLoop::process_function_mutation_keys(uint64_t start_ticks_ms, TickResult tick_result) {
    using std::get;

    auto const now_ms = SDL_GetTicks();
    auto const up_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_UP));
    auto const down_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_DOWN));

    auto const left_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_LEFT));
    auto const right_key_timing = which_key_variant_was_pressed_since(start_ticks_ms, now_ms, Key(SDL_SCANCODE_RIGHT));

    // xor
    tick_result.set_function_params_modified(false);
    if (left_key_timing.has_value() != right_key_timing.has_value()) {

        if (left_key_timing.has_value()) {
            tick_result.set_function_params_modified(true);
            auto const panning_movement =
                static_cast<GLfloat>(get<2>(*left_key_timing) - get<1>(*left_key_timing)) * panning_delta_per_ms;
            logger->debug("panning {} in negative direction", panning_movement);
            if (get<0>(*left_key_timing).has_shift()) {
                function_params->y_offset -= panning_movement;
            }
            else {
                function_params->x_offset -= panning_movement;
            }
        }

        if (right_key_timing.has_value()) {
            tick_result.set_function_params_modified(true);
            auto const panning_movement =
                static_cast<GLfloat>(get<2>(*right_key_timing) - get<1>(*right_key_timing)) * panning_delta_per_ms;

            logger->debug("panning {} in positive direction", panning_movement);
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
            tick_result.set_function_params_modified(true);
            auto const z_mult_movement =
                static_cast<GLfloat>(get<2>(*up_key_timing) - get<1>(*up_key_timing)) * z_mult_delta_per_ms;
            logger->debug("changing z by {} in positive direction", z_mult_movement);
            function_params->z_mult += z_mult_movement;
        }

        if (down_key_timing.has_value()) {
            tick_result.set_function_params_modified(true);
            auto const z_mult_movement =
                static_cast<GLfloat>(get<2>(*down_key_timing) - get<1>(*down_key_timing)) * z_mult_delta_per_ms;
            logger->debug("changing z by {} in negative direction", z_mult_movement);
            function_params->z_mult -= z_mult_movement;
        }
    }

    return tick_result;
}

TickResult EventLoop::process_render_setting_keys(uint64_t start_ticks_ms, TickResult tick_result) {
    tick_result.set_wireframe_display_mode_toggled(false);

    // TODO: de-duplicate between this and the tessellation change settings
    if (last_wireframe_only_change_at_msec.has_value() &&
        *last_wireframe_only_change_at_msec + msec_between_toggle_wireframe_changes > start_ticks_ms) {
        // not long enough since last toggle
        return tick_result;
    }

    // TODO: should fix this so that holding down the toggling key doesn't cycle between the two infinitely
    if (active_keys.was_key_pressed_since(SDL_SCANCODE_E, start_ticks_ms)) {
        tick_result.set_wireframe_display_mode_toggled(true);
        last_wireframe_only_change_at_msec = start_ticks_ms;
    }

    return tick_result;
}

TickResult EventLoop::process_tessellation_mutation_keys(uint64_t start_ticks_ms, TickResult tick_result) {

    tick_result.set_tessellation_settings_modified(false);
    if (last_tessellation_change_at_msec.has_value() &&
        *last_tessellation_change_at_msec + msec_between_tess_level_changes > start_ticks_ms) {
        // not long enough since last tessellation level change occurred
        return tick_result;
    }

    // TODO: once pressed these will keep registering every loop even on key release
    bool const plus_key_timing = active_keys.was_key_pressed_since(SDLK_PLUS, start_ticks_ms);
    bool const minus_key_timing = active_keys.was_key_pressed_since(SDLK_MINUS, start_ticks_ms);

    bool level_changed = false;
    if (plus_key_timing != minus_key_timing) {
        if (plus_key_timing) {
            logger->debug("increasing tessellation level");
            tessellation_settings->increment_level();
            level_changed = true;
        }

        if (minus_key_timing) {
            logger->debug("decreasing tessellation level");
            tessellation_settings->decrement_level();
            level_changed = true;
        }
    }

    if (level_changed) {
        tick_result.set_tessellation_settings_modified(true);
        last_tessellation_change_at_msec = start_ticks_ms;
    }

    return tick_result;
}

TickResult EventLoop::process_model_mutation_keys(uint64_t start_ms, uint64_t end_ms, TickResult tick_result) {
    using std::get;

    auto const up_key_timing = active_keys.what_key_mods_pressed_since(SDL_SCANCODE_W, SDL_KMOD_SHIFT, start_ms);
    auto const down_key_timing = active_keys.what_key_mods_pressed_since(SDL_SCANCODE_S, SDL_KMOD_SHIFT, start_ms);

    auto const left_key_timing = active_keys.what_key_mods_pressed_since(SDL_SCANCODE_A, SDL_KMOD_SHIFT, start_ms);
    auto const right_key_timing = active_keys.what_key_mods_pressed_since(SDL_SCANCODE_D, SDL_KMOD_SHIFT, start_ms);

    auto const rotations_rads = static_cast<float>(rotation_rad_millis * static_cast<double>(end_ms - start_ms));
    auto const slowed_rotations_rads =
        static_cast<float>(slowed_rotation_rad_millis * static_cast<double>(end_ms - start_ms));
    quat current(*model);

    tick_result.set_model_modified(false);
    if (up_key_timing != down_key_timing) {
        if (up_key_timing) {
            tick_result.set_model_modified(true);
            quat const rotation =
                angleAxis(up_key_timing->has_shift() ? slowed_rotations_rads : rotations_rads, x_axis);
            current = rotation * current;
        }

        if (down_key_timing) {
            tick_result.set_model_modified(true);
            quat const rotation =
                angleAxis(-(down_key_timing->has_shift() ? slowed_rotations_rads : rotations_rads), x_axis);
            current = rotation * current;
        }
    }

    if (left_key_timing != right_key_timing) {
        if (left_key_timing) {
            tick_result.set_model_modified(true);
            quat const rotation =
                angleAxis(-(left_key_timing->has_shift() ? slowed_rotations_rads : rotations_rads), y_axis);
            current = rotation * current;
        }

        if (right_key_timing) {
            tick_result.set_model_modified(true);
            quat const rotation =
                angleAxis(right_key_timing->has_shift() ? slowed_rotations_rads : rotations_rads, y_axis);
            current = rotation * current;
        }
    }

    if (tick_result.model_modified()) {
        logger->debug("will update model quat to {0}, {1}, {2}, {3}", current.w, current.x, current.y, current.z);
        *model = toMat4(current);
    }

    return tick_result;
}

TickResult EventLoop::process_view_mutation_events(bool scrolled_toward_user, TickResult tick_result) {

    tick_result.set_view_modified(false);
    if (scrolled_toward_user) {
        // zoom out
        *view = translate(*view, zoom_out);
        tick_result.set_view_modified(true);
    }
    else {
        // zoom in
        *view = translate(*view, zoom_in);
        tick_result.set_view_modified(true);
    }

    return tick_result;
}

TickResult EventLoop::drain_event_queue(TickResult tick_result) {

    while (SDL_PollEvent(&evt)) {
        // TODO: std::visit
        if (evt.type == SDL_EVENT_QUIT) {
            tick_result.set_should_exit(true);
            return tick_result;
        }
        else if (evt.type == SDL_EVENT_KEY_UP) {
            const Key released{evt.key.scancode, evt.key.key, evt.key.mod};

            logger->debug("released key {0}", released);
            active_keys.release_key(released);
        }
        else if (evt.type == SDL_EVENT_KEY_DOWN) {
            const Key pressed{evt.key.scancode, evt.key.key, evt.key.mod};

            if (pressed.get_key_code()
                    .transform([](SDL_Keycode code) { return code == SDLK_Q || code == SDLK_ESCAPE; })
                    .value_or(false)) {
                tick_result.set_should_exit(true);
                return tick_result;
            }

            logger->debug("pressed key {0}", pressed);

            // mouse clicks disable keys
            if (start_click.has_value()) {
                continue;
            }

            active_keys.press_key(pressed);
        }
        else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            // TODO: implement this
            start_click = nullopt;
            // start_click = make_optional<MouseLoc>(evt.motion.x, evt.motion.y);
        }
        else if (evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            start_click = nullopt;
        }
        else if (evt.type == SDL_EVENT_MOUSE_WHEEL) {
            tick_result = process_view_mutation_events(evt.wheel.integer_y < 0, tick_result);
        }
        else if (start_click.has_value() && evt.type == SDL_EVENT_MOUSE_MOTION) {
            // TODO: implement this
            // MouseLoc current(evt.motion.x, evt.motion.y);
        }
    }

    return tick_result;
}

TickResult EventLoop::process_frame(uint64_t render_time_ns) {
    auto const start_ticks_ms = SDL_GetTicks();
    auto const start_ticks_ns = SDL_GetTicksNS();

    /** what ticks ns timestamp to not exceed to maintain fps */
    auto const absolute_max_end_ticks_ns = start_ticks_ns + max_sleep_ns_per_tick - render_time_ns;

    // what's the latest ticks ns that can do another sdl event queue drain
    // without likely going over the time allowed to process the frame
    auto end_ticks_ns = absolute_max_end_ticks_ns - event_poll_timings.get_avg();

    auto drain_start_ns = SDL_GetTicksNS();
    if (drain_start_ns >= end_ticks_ns) {
        logger->warn("skipping input polling this tick");
        // not entirely accurate, is used to prevent a couple of slow input poll loops
        // from locking out all input polling by dropping down the average
        event_poll_timings.add(0);
        SDL_FlushEvents(SDL_EVENT_QUIT + 1, SDL_EVENT_ENUM_PADDING);
        return TickResult{SDL_GetTicks() - start_ticks_ms, false, true};
    }

    TickResult tick_result{SDL_GetTicks() - start_ticks_ms, false, false};
    while ((drain_start_ns = SDL_GetTicksNS()) < end_ticks_ns) {
        tick_result = drain_event_queue(tick_result);
        if (tick_result.should_exit()) {
            return tick_result;
        }

        auto const drain_end_ns = SDL_GetTicksNS();

        // adjust timing expectations based on latest data
        event_poll_timings.add(drain_end_ns - drain_start_ns);
        end_ticks_ns = absolute_max_end_ticks_ns - event_poll_timings.get_avg();
    }

    // TODO: track this overhead separately and use to compute how much input to process
    // per frame

    tick_result = process_function_mutation_keys(start_ticks_ms, tick_result);
    tick_result = process_model_mutation_keys(start_ticks_ms, SDL_GetTicks(), tick_result);
    tick_result = process_tessellation_mutation_keys(start_ticks_ms, tick_result);
    tick_result = process_render_setting_keys(start_ticks_ms, tick_result);

    tick_result.elapsed_ticks_ms = SDL_GetTicks() - start_ticks_ms;
    return tick_result;
}
