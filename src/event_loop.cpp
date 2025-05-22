#include "event_loop.hpp"
#include "tick_result.hpp"
#include "function_params.hpp"
#include "mouse_loc.hpp"

#include <SDL/SDL_events.h>
#include <SDL_timer.h>
#include <cstdint>
#include <memory>
#include <optional>

#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

using std::optional;
using std::shared_ptr;
using std::make_optional;
using std::nullopt;

using glm::mat4;
using glm::radians;
using glm::rotate;
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
                    *model = rotate(model, radians(-1.0f), vec3(1.0f, 0.0f, 0.0f));
                }

                else if (evt.key.keysym.sym == SDLK_d) {
                    view_modified_ = true;
                    *model = rotate(model, radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
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

        if (is_mouse_rotating_surface && start_click_loc) {
            current_loc.update_loc();

            double dist = current_loc.distance(start_click_loc.value());
            if (dist >= 5) { // arbitrary
                             // TODO spin the model based off of how far the mouse
                             // has been moved since clicking
            }
        }
    }

    return TickResult(SDL_GetTicks() - start_ticks, false);
}
