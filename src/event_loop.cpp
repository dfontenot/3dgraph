#include "event_loop.hpp"
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

using glm::mat4;
using glm::radians;
using glm::rotate;
using glm::translate;
using glm::vec3;

EventLoop::EventLoop(shared_ptr<mat4> model, shared_ptr<mat4> view, shared_ptr<mat4> projection,
                     shared_ptr<FunctionParams> function_params)
    : model(model), view(view), projection(projection), function_params(function_params),
      function_params_modified_(false), view_modified_(false) {
}

bool EventLoop::function_params_modified() const {
    return function_params_modified_;
}

bool EventLoop::view_modified() const {
    return view_modified_;
}

uint32_t EventLoop::tick() {
    function_params_modified_ = false;
    view_modified_ = false;
    bool is_mouse_rotating_surface = false;
    optional<MouseLoc> start_click_loc;
    MouseLoc current_loc;

    auto start_ticks = SDL_GetTicks();
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) {
            return 0;
        }
        else if (evt.type == SDL_KEYDOWN) {
            if (evt.key.keysym.sym == SDLK_q) {
                return 0;
            }

            if (!is_mouse_rotating_surface) {
                if (evt.key.keysym.sym == SDLK_a) {
                    view_modified_ = true;
                    model = rotate(model, radians(-1.0f), vec3(1.0f, 0.0f, 0.0f));
                }

                else if (evt.key.keysym.sym == SDLK_d) {
                    view_modified_ = true;
                    model = rotate(model, radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
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
            is_mouse_rotating_surface = true;
            MouseLoc new_loc;
            start_click_loc = new_loc;
        }
        else if (evt.type == SDL_MOUSEBUTTONUP) {
            is_mouse_rotating_surface = false;
            start_click_loc.reset();
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

    auto end_ticks = SDL_GetTicks();
    return end_ticks - start_ticks;
}
