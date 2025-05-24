#pragma once
#include <cstdint>

struct TickResult {
    uint64_t elapsed_ticks_ms;
    bool should_exit;
    bool frame_skip;

    TickResult() : elapsed_ticks_ms(0), should_exit(false), frame_skip(false) {
    }

    TickResult(uint64_t elapsed_ticks_ms, bool should_exit, bool frame_skip)
        : elapsed_ticks_ms(elapsed_ticks_ms), should_exit(should_exit), frame_skip(frame_skip) {
    }
};
