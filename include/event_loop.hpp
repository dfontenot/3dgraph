#pragma once

#include "function_params.hpp"
#include "tick_result.hpp"
#include "max_deque.hpp"
#include "glad/glad.h"
#include "mouse_loc.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <cstdint>
#include <cstddef>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <optional>

class EventLoop {
    SDL_Event evt;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;
    MaxDeque<uint64_t> event_poll_timings;

    // state stuff
    bool function_params_modified_;
    bool model_modified_;
    bool view_modified_;
    float rotational_axis_direction;
    std::optional<glm::vec3> rotational_axis;
    std::optional<MouseLoc> start_click;

    /**
     * drain the sdl event queue one time
     * returns true if should exit due to quit event
     */
    bool drain_event_queue_should_exit();

public:
    /**
     * returns how long the tick took to run
     */
    TickResult tick(uint64_t render_time_ns);

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
