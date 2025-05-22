#include "event_loop.hpp"
#include "function_params.hpp"
#include "mouse_loc.hpp"
#include "tick_result.hpp"

#include <SDL2/SDL.h>
#include <memory>
#include <optional>

#define GLM_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/vec3.hpp>

using std::make_optional;
using std::nullopt;
using std::optional;
using std::shared_ptr;

using glm::mat4;
using glm::radians;
using glm::rotate;
//using glm::swizzle;
using glm::translate;
using glm::vec3;

EventLoop::EventLoop(shared_ptr<mat4> model, shared_ptr<mat4> view, shared_ptr<mat4> projection,
                     shared_ptr<FunctionParams> function_params)
    : model(model), view(view), projection(projection), function_params(function_params),
      function_params_modified_(false), view_modified_(false), start_click(nullopt) {
}

bool EventLoop::function_params_modified() const {
    return function_params_modified_;
}

bool EventLoop::view_modified() const {
    return view_modified_;
}

TickResult EventLoop::tick() {
    function_params_modified_ = false;
    view_modified_ = false;
    float rotational_axis_direction = 0.0f;
    vec3 rotational_axis = vec3(0.0f, 0.0f, 0.0f);

    auto start_ticks = SDL_GetTicks();
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) {
            return TickResult(SDL_GetTicks() - start_ticks, true);
        }
        else if (evt.type == SDL_KEYDOWN) {
            if (evt.key.keysym.sym == SDLK_q) {
                return TickResult(SDL_GetTicks() - start_ticks, true);
            }

            if (!start_click.has_value()) {
                if (evt.key.keysym.sym == SDLK_a) {
                    view_modified_ = true;
                    rotational_axis_direction = -1.0f;
                    rotational_axis.x = 1.0f;
                    //rotational_axis.yz = 0.0f;
                    //*model = rotate(model, radians(-1.0f), vec3(1.0f, 0.0f, 0.0f));
                }
                else if (evt.key.keysym.sym == SDLK_d) {
                    view_modified_ = true;
                    rotational_axis_direction = 1.0f;
                    //*model = rotate(model, radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
                }
                else if (evt.key.keysym.sym == SDLK_w) {
                    view_modified_ = true;
                    rotational_axis_direction = 1.0f;
                    //*model = rotate(model, radians(1.0f), vec3(0.0f, 1.0f, 0.0f));
                }
                else if (evt.key.keysym.sym == SDLK_s) {
                    view_modified_ = true;
                    rotational_axis_direction = -1.0f;
                    //*model = rotate(model, radians(-1.0f), vec3(0.0f, 1.0f, 0.0f));
                }
            }

            // TODO: mouse events to make this more intuitive
            if (evt.key.keysym.sym == SDLK_LEFT) {
                if (evt.key.keysym.mod & KMOD_SHIFT) {
                    function_params->y_offset -= panning_delta;
                    function_params_modified_ = true;
                }
                else {
                    function_params->x_offset -= panning_delta;
                    function_params_modified_ = true;
                }
            }

            if (evt.key.keysym.sym == SDLK_RIGHT) {
                if (evt.key.keysym.mod & KMOD_SHIFT) {
                    function_params->y_offset += panning_delta;
                    function_params_modified_ = true;
                }
                else {
                    function_params->x_offset += panning_delta;
                    function_params_modified_ = true;
                }
            }

            if (evt.key.keysym.sym == SDLK_UP) {
                function_params->z_mult += 0.1;
                function_params_modified_ = true;
            }

            if (evt.key.keysym.sym == SDLK_DOWN) {
                function_params->z_mult -= 0.1;
                function_params_modified_ = true;
            }
        }
        else if (evt.type == SDL_MOUSEBUTTONDOWN) {
            start_click = make_optional<MouseLoc>(evt.motion.x, evt.motion.y);
        }
        else if (evt.type == SDL_MOUSEBUTTONUP) {
            start_click = nullopt;
        }
        else if (start_click.has_value() && evt.type == SDL_MOUSEMOTION) {
            MouseLoc current(evt.motion.x, evt.motion.y);
        }
    }

    return TickResult(SDL_GetTicks() - start_ticks, false);
}
