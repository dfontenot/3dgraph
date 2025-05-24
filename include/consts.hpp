#pragma once

#include <cstddef>
#include <cstdint>

// TODO: use sdl to get the monitor's refresh rate and use that for the target fps
constexpr uint8_t target_fps = 30;

// NOTE: went from a u32 to a u64 with SDL 3, but lots of
// search engine hits go to the SDL2 docs, this is definitely a u64
// docs: https://wiki.libsdl.org/SDL3/SDL_GetTicks
constexpr uint64_t max_sleep_ms_per_tick = 1000 / target_fps;
constexpr uint64_t max_sleep_ns_per_tick = 1e9 / target_fps;
constexpr std::size_t window_h = 800;
constexpr std::size_t window_w = 1200;
