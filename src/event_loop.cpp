#include "event_loop.hpp"
#include "formatters.hpp"
#include "function_params.hpp"
#include "mouse_loc.hpp"
#include "tick_result.hpp"

#include <SDL3/SDL.h>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <memory>
#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using std::make_optional;
using std::nullopt;
using std::shared_ptr;

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

EventLoop::EventLoop(shared_ptr<mat4> model, shared_ptr<mat4> view, shared_ptr<mat4> projection,
                     shared_ptr<FunctionParams> function_params)
    : model(model), view(view), projection(projection), function_params(function_params),
      function_params_modified_(false), view_modified_(false), model_modified_(false), start_click(nullopt) {
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

TickResult EventLoop::tick() {
    function_params_modified_ = false;
    view_modified_ = false;
    model_modified_ = false;
    float rotational_axis_direction = 0.0f;
    vec3 rotational_axis = vec3(1.0f, 0.0f, 0.0f);

    auto stdout = spdlog::get("stdout");
    auto start_ticks = SDL_GetTicks();
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_EVENT_QUIT) {
            return TickResult(SDL_GetTicks() - start_ticks, true);
        }
        else if (evt.type == SDL_EVENT_KEY_DOWN) {
            if (evt.key.key == SDLK_Q) {
                return TickResult(SDL_GetTicks() - start_ticks, true);
            }

            if (!start_click.has_value()) {
                if (evt.key.key == SDLK_A) {
                    model_modified_ = true;
                    rotational_axis_direction = -1.0f;
                    rotational_axis = x_axis;
                }
                else if (evt.key.key == SDLK_D) {
                    model_modified_ = true;
                    rotational_axis_direction = 1.0f;
                    rotational_axis = x_axis;
                }
                else if (evt.key.key == SDLK_W) {
                    model_modified_ = true;
                    rotational_axis_direction = 1.0f;
                    rotational_axis = y_axis;
                }
                else if (evt.key.key == SDLK_S) {
                    model_modified_ = true;
                    rotational_axis = y_axis;
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
            stdout->debug("started click at {0} {1}", evt.motion.x, evt.motion.y);
            start_click = make_optional<MouseLoc>(evt.motion.x, evt.motion.y);
        }
        else if (evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            stdout->debug("ended click at {0} {1}", evt.motion.x, evt.motion.y);
            start_click = nullopt;
        }
        else if (start_click.has_value() && evt.type == SDL_EVENT_MOUSE_MOTION) {
            //MouseLoc current(evt.motion.x, evt.motion.y);
        }
    }

    auto end_ticks = SDL_GetTicks();
    auto elapsed_millis = end_ticks - start_ticks;
    stdout->debug("that took {} millis", elapsed_millis);

    if (model_modified_) {
        quat current(*model);

        quat rotation =
            angleAxis((float)(rotation_rad_millis * elapsed_millis) * rotational_axis_direction, rotational_axis);
        quat new_model_orientation = rotation * current;

        stdout->debug("will update model matrix from {0} to {1}", current, new_model_orientation);
        *model = toMat4(new_model_orientation);
    }

    return TickResult(elapsed_millis, false);
}
