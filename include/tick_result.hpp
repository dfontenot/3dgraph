#pragma once
#include <cstdint>

struct TickResult {
    uint64_t elapsed_ticks_ms;
    bool should_exit;

    TickResult() : elapsed_ticks_ms(0), should_exit(false) {
    }

    TickResult(uint64_t elapsed_ticks_ms, bool should_exit) : elapsed_ticks_ms(elapsed_ticks_ms), should_exit(should_exit) {
    }
};
