#pragma once
#include <cstdint>

struct TickResult {
    uint32_t elapsed_ticks;
    bool should_exit;

    TickResult() : elapsed_ticks(0), should_exit(false) {
    }

    TickResult(uint32_t elapsed_ticks, bool should_exit) : elapsed_ticks(elapsed_ticks), should_exit(should_exit) {
    }
};
