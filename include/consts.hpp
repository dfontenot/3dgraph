#pragma once

#include <cstdint>
#include <cstddef>

constexpr uint32_t target_fps = 30;
constexpr uint32_t max_sleep_per_tick = 1000 / target_fps;
constexpr std::size_t window_h = 800;
constexpr std::size_t window_w = 1200;
