#pragma once

#include "active_keys.hpp"
#include "function_params.hpp"
#include "glad/glad.h"
#include "key.hpp"
#include "max_deque.hpp"
#include "mouse_loc.hpp"
#include "tick_result.hpp"
#include <SDL3/SDL.h>
#include <cstdint>
#include <memory>
#include <tuple>

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
    ActiveKeys active_keys;

    // state stuff
    bool function_params_modified_;
    bool model_modified_;
    bool view_modified_;
    float rotational_axis_direction;
    std::optional<glm::vec3>& rotational_axis;
    std::optional<MouseLoc> start_click;

    /**
     * drain the sdl event queue one time
     * returns true if should exit due to quit event
     */
    bool drain_event_queue_should_exit();

    /**
     * see if the scancode of the key was used at all, and if so see which one was pressed for the longest for
     * this frame. the returned start time will always be greater than or equal to the start_ms
     * argument (so keys still held down on the last frame will be at start_ms).
     * keys that were released after the start time are excluded, and keys that are still currently pressed will
     * report end_ms as their end time
     */
    std::optional<std::tuple<Key, uint64_t, uint64_t>> which_key_variant_was_pressed_since(uint64_t start_ms,
                                                                                           uint64_t end_ms,
                                                                                           const Key &key) const;
    void process_function_mutation_keys(uint64_t start_ticks_ms);
    void process_model_mutation_keys(uint64_t start_ticks_ms);

public:
    /**
     * returns how long the tick took to run
     */
    TickResult process_frame(uint64_t render_time_ns);

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
