#pragma once

#include "glad/glad.h"
#include "function_params.hpp"
#include <SDL/SDL_events.h>
#include <cstdint>
#include <memory>

#include <glm/mat4x4.hpp>

constexpr GLfloat panning_delta = 0.01f;

class EventLoop {
    SDL_Event evt;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;

    // state stuff
    bool function_params_modified_;
    bool view_modified_;
    bool is_mouse_rotating_surface;

public:
    /**
     * returns how long the tick took to run
     */
    uint32_t tick();

    EventLoop() = delete;
    EventLoop(std::shared_ptr<glm::mat4> model, std::shared_ptr<glm::mat4> view, std::shared_ptr<glm::mat4> projection, std::shared_ptr<FunctionParams> function_params);
    ~EventLoop();

    /**
     * if the view (zoom, rotate, pan, etc.) was changed in any way during the tick
     */
    bool view_modified() const;

    /**
     * if the parameters of the 3D function that is being displayed were changed during the tick
     */
    bool function_params_modified() const;
};
