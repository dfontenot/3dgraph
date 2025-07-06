#include "tick_result.hpp"

#include <algorithm>
#include <cstdint>
#include <ranges>

using std::ranges::iota_view;

TickResult::TickResult(uint64_t elapsed_ticks_ms, bool should_exit, bool frame_skip)
    : elapsed_ticks_ms(elapsed_ticks_ms), state(0) {
    set_should_exit(should_exit);
    set_frame_skip(frame_skip);
}

bool TickResult::anything_modified() const noexcept {
    constexpr const iota_view bit_range{function_params_modified_bit, tessellation_settings_modified_bit + 1};

    return std::ranges::none_of(bit_range, [&](auto bit_idx) { return state.test(bit_idx); });
}

bool TickResult::should_exit() const noexcept {
    return state.test(should_exit_bit);
}

bool TickResult::frame_skip() const noexcept {
    return state.test(frame_skip_bit);
}

bool TickResult::function_params_modified() const noexcept {
    return state.test(function_params_modified_bit);
}

bool TickResult::model_modified() const noexcept {
    return state.test(model_modified_bit);
}

bool TickResult::view_modified() const noexcept {
    return state.test(view_modified_bit);
}

bool TickResult::tessellation_settings_modified() const noexcept {
    return state.test(tessellation_settings_modified_bit);
}

void TickResult::set_should_exit(bool should_exit) noexcept {
    state.set(should_exit_bit, should_exit);
}

void TickResult::set_frame_skip(bool frame_skip) noexcept {
    state.set(frame_skip_bit, frame_skip);
}

void TickResult::set_function_params_modified(bool function_params_modified) noexcept {
    state.set(function_params_modified_bit, function_params_modified);
}

void TickResult::set_model_modified(bool model_modified) noexcept {
    state.set(model_modified_bit, model_modified);
}

void TickResult::set_view_modified(bool view_modified) noexcept {
    state.set(view_modified_bit, view_modified);
}

void TickResult::set_tessellation_settings_modified(bool tessellation_settings_modified) noexcept {
    state.set(tessellation_settings_modified_bit, tessellation_settings_modified);
}
