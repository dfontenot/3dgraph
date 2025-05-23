#pragma once

#include "function_params.hpp"
#include "tick_result.hpp"
#include "glad/glad.h"
#include "mouse_loc.hpp"
#include <SDL3/SDL.h>
#include <memory>

#include <glm/mat4x4.hpp>
#include <optional>

constexpr GLfloat panning_delta = 0.01f;

class EventLoop {
    SDL_Event evt;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;

    // state stuff
    bool function_params_modified_;
    bool model_modified_;
    bool view_modified_;
    std::optional<MouseLoc> start_click;

public:
    /**
     * returns how long the tick took to run
     */
    TickResult tick();

    EventLoop() = delete;
    EventLoop(std::shared_ptr<glm::mat4> model, std::shared_ptr<glm::mat4> view, std::shared_ptr<glm::mat4> projection,
              std::shared_ptr<FunctionParams> function_params);

    /**
     * if the view (zoom, pan, etc.) was changed in any way during the tick
     */
    bool view_modified() const;

    /**
     * if the model orientation (orbit) was changed in any way during the tick
     */
    bool model_modified() const;

    /**
     * if the parameters of the 3D function that is being displayed were changed during the tick
     */
    bool function_params_modified() const;
};
