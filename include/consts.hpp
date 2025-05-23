#pragma once

#include <cstdint>
#include <cstddef>

// TODO: use sdl to get the monitor's refresh rate and use that for the target fps
constexpr uint32_t target_fps = 30;
constexpr uint32_t max_sleep_ms_per_tick = 1000 / target_fps;
constexpr uint32_t max_sleep_ns_per_tick = 1e9 / target_fps;
constexpr std::size_t window_h = 800;
constexpr std::size_t window_w = 1200;
