#pragma once

#include "function_params.hpp"
#include "tick_result.hpp"
#include "glad/glad.h"
#include "mouse_loc.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <cstdint>
#include <deque>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <optional>

class EventLoop {
    SDL_Event evt;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;
    std::deque<uint64_t> prev_event_poll_ns;
    uint64_t prev_event_poll_ns_sum;

    // state stuff
    bool function_params_modified_;
    bool model_modified_;
    bool view_modified_;
    float rotational_axis_direction;
    std::optional<glm::vec3> rotational_axis;
    std::optional<MouseLoc> start_click;

    /**
     * how long, on average, does it take to poll all of the events, in nanoseconds
     */
    uint64_t get_historic_event_poll_ns() const;

    /**
     * maintain a limited number of timings for event draining in order to determine
     * if another event poll can occur during the tick
     */
    void add_historic_event_poll_ns(uint64_t new_timing);


    /**
     * drain the sdl event queue one time
     * returns true if should exit due to quit event
     */
    bool drain_event_queue_should_exit();

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
