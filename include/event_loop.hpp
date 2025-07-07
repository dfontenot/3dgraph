#pragma once

#include "active_keys.hpp"
#include "function_params.hpp"
#include "glad/glad.h"
#include "key.hpp"
#include "max_deque.hpp"
#include "mouse_loc.hpp"
#include "tessellation_settings.hpp"
#include "tick_result.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>

#include <SDL3/SDL.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <spdlog/spdlog.h>

using KeyAtTime = std::tuple<Key, uint64_t, uint64_t>;

class EventLoop {
    SDL_Event evt;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;
    std::shared_ptr<TessellationSettings> tessellation_settings;
    MaxDeque<uint64_t> event_poll_timings;
    ActiveKeys active_keys;

    // logger
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> err;

    // state stuff
    std::optional<uint64_t> last_tessellation_change_at_msec;
    std::optional<MouseLoc> start_click;

    /**
     * drain the sdl event queue one time
     * returns true if should exit due to quit event
     */
    TickResult drain_event_queue(TickResult tick_result);

    /**
     * see if the scancode of the key was used at all, and if so see which one was pressed for the longest for
     * this frame. the returned start time will always be greater than or equal to the start_ms
     * argument (so keys still held down on the last frame will be at start_ms).
     * keys that were released after the start time are excluded, and keys that are still currently pressed will
     * report end_ms as their end time
     */
    std::optional<KeyAtTime> which_key_variant_was_pressed_since(uint64_t start_ms, uint64_t end_ms,
                                                                 const Key &key) const;
    [[nodiscard]] TickResult process_function_mutation_keys(uint64_t start_ticks_ms, TickResult tick_result);
    [[nodiscard]] TickResult process_model_mutation_keys(uint64_t start_ticks_ms, uint64_t end_ticks_ms,
                                                         TickResult tick_result);
    [[nodiscard]] TickResult process_tessellation_mutation_keys(uint64_t start_ticks_ms, TickResult tick_result);
    [[nodiscard]] TickResult process_view_mutation_events(bool scrolled_toward_user, TickResult tick_result);
    [[nodiscard]] TickResult process_render_setting_keys(uint64_t start_ticks_ms, TickResult tick_result);

public:
    /**
     * @return how long the frame took to run
     */
    [[nodiscard]] TickResult process_frame(uint64_t render_time_ns);

    EventLoop() = delete;
    EventLoop(std::shared_ptr<glm::mat4> const &model, std::shared_ptr<glm::mat4> const &view,
              std::shared_ptr<glm::mat4> const &projection, std::shared_ptr<FunctionParams> const &function_params,
              std::shared_ptr<TessellationSettings> const &tessellation_settings);
};
