#include "event_loop.hpp"
#include "consts.hpp"
#include "formatters.hpp"
#include "function_params.hpp"
#include "mouse_loc.hpp"
#include "tick_result.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <memory>
#include <optional>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using std::make_optional;
using std::nullopt;
using std::shared_ptr;
using std::size_t;

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

constexpr GLfloat panning_delta = 0.01f;

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
      prev_event_poll_ns_sum(0), rotational_axis_direction(0.0f), rotational_axis(nullopt) {
}

uint64_t EventLoop::get_historic_event_poll_ns() const {

    auto const num_event_timings = prev_event_poll_ns.size();

    if (num_event_timings == 0) {
        return 0;
    }

    return prev_event_poll_ns_sum / num_event_timings;
}

void EventLoop::add_historic_event_poll_ns(uint64_t new_timing) {
    auto const num_event_timings = prev_event_poll_ns.size();

    if (num_event_timings == num_event_timings_maintain) {
        prev_event_poll_ns_sum -= prev_event_poll_ns.front();
        prev_event_poll_ns.pop_front();
    }

    prev_event_poll_ns.push_back(new_timing);
    prev_event_poll_ns_sum += new_timing;
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

bool EventLoop::drain_event_queue_should_exit() {
    rotational_axis_direction = 0.0f;
    rotational_axis = nullopt;

    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_EVENT_QUIT) {
            return true;
        }
        else if (evt.type == SDL_EVENT_KEY_DOWN) {
            if (evt.key.key == SDLK_Q) {
                return true;
            }

            if (!start_click.has_value()) {
                if (evt.key.key == SDLK_A) {
                    model_modified_ = true;
                    rotational_axis_direction = -1.0f;
                    rotational_axis = make_optional(x_axis);
                }
                else if (evt.key.key == SDLK_D) {
                    model_modified_ = true;
                    rotational_axis_direction = 1.0f;
                    rotational_axis = make_optional(x_axis);
                }
                else if (evt.key.key == SDLK_W) {
                    model_modified_ = true;
                    rotational_axis_direction = 1.0f;
                    rotational_axis = make_optional(y_axis);
                }
                else if (evt.key.key == SDLK_S) {
                    model_modified_ = true;
                    rotational_axis = make_optional(y_axis);
                    rotational_axis_direction = -1.0f;
                }
            }

            // TODO: mouse events to make this more intuitive
            if (evt.key.key == SDLK_LEFT) {
                if (evt.key.mod & SDL_KMOD_SHIFT) {
                    function_params->y_offset -= panning_delta;
                    function_params_modified_ = true;
                }
                else {
                    function_params->x_offset -= panning_delta;
                    function_params_modified_ = true;
                }
            }

            if (evt.key.key == SDLK_RIGHT) {
                if (evt.key.mod & SDL_KMOD_SHIFT) {
                    function_params->y_offset += panning_delta;
                    function_params_modified_ = true;
                }
                else {
                    function_params->x_offset += panning_delta;
                    function_params_modified_ = true;
                }
            }

            if (evt.key.key == SDLK_UP) {
                function_params->z_mult += 0.1;
                function_params_modified_ = true;
            }

            if (evt.key.key == SDLK_DOWN) {
                function_params->z_mult -= 0.1;
                function_params_modified_ = true;
            }
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

TickResult EventLoop::tick() {
    // reset state for new tick
    function_params_modified_ = false;
    view_modified_ = false;
    model_modified_ = false;

    auto const stdout = spdlog::get("stdout");

    auto const start_ticks_ms = SDL_GetTicks();
    auto const start_ticks_ns = SDL_GetTicksNS();

    // what ticks ns timestamp to not exceed to maintain fps
    auto const absolute_max_end_ticks_ns = start_ticks_ns + max_sleep_ns_per_tick;

    // what's the latest ticks ns that can do another sdl event queue drain
    // without likely going over the time allowed to process the frame
    auto end_ticks_ns = absolute_max_end_ticks_ns - get_historic_event_poll_ns();

    auto drain_start_ns = SDL_GetTicksNS();
    if (drain_start_ns >= end_ticks_ns) {
        stdout->debug("skipping input polling this tick");
        // not entirely accurate, is used to prevent a couple of slow input poll loops
        // from locking out all input polling by dropping down the average
        add_historic_event_poll_ns(0);
        return TickResult{SDL_GetTicks() - start_ticks_ms, false, true};
    }

    while ((drain_start_ns = SDL_GetTicksNS()) < end_ticks_ns) {
        stdout->debug("draining events queue");
        if (drain_event_queue_should_exit()) {
            return TickResult{SDL_GetTicks() - start_ticks_ms, true, false};
        }

        auto const drain_end_ns = SDL_GetTicksNS();

        // adjust timing expectations based on latest data
        add_historic_event_poll_ns(drain_end_ns - drain_start_ns);
        end_ticks_ns = absolute_max_end_ticks_ns - get_historic_event_poll_ns();
    }

    auto const elapsed_millis = SDL_GetTicks() - start_ticks_ms;

    if (model_modified_) {
        quat const current(*model);

        auto const rotations_rads = static_cast<float>(rotation_rad_millis * static_cast<double>(elapsed_millis));
        quat const rotation = angleAxis(rotations_rads * rotational_axis_direction, rotational_axis.value());
        quat const new_model_orientation = rotation * current;

        stdout->debug("will update model matrix from {0} to {1}", current, new_model_orientation);
        *model = toMat4(new_model_orientation);
    }

    return TickResult(SDL_GetTicks() - start_ticks_ms, false, false);
}
