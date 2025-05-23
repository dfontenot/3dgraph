#pragma once
#include <cstdint>

struct TickResult {
    uint64_t elapsed_ticks_ns;
    bool should_exit;

    TickResult() : elapsed_ticks_ns(0), should_exit(false) {
    }

    TickResult(uint64_t elapsed_ticks_ns, bool should_exit) : elapsed_ticks_ns(elapsed_ticks_ns), should_exit(should_exit) {
    }
};
