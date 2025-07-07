#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>

class TickResult {
    // TODO: better mechanism for tracking this
    // TODO: update to constexpr std::map in C++26
    static constexpr const std::size_t should_exit_bit = 0;
    static constexpr const std::size_t frame_skip_bit = 1;
    static constexpr const std::size_t toggle_wireframe_display_bit = 2;
    static constexpr const std::size_t function_params_modified_bit = 3;
    static constexpr const std::size_t model_modified_bit = 4;
    static constexpr const std::size_t view_modified_bit = 5;
    static constexpr const std::size_t tessellation_settings_modified_bit = 6;

    // TODO: better mechanism for tracking this
    std::bitset<tessellation_settings_modified_bit + 1> state;

public:
    // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
    uint64_t elapsed_ticks_ms;

    TickResult() : elapsed_ticks_ms(0), state(0) {
    }

    TickResult(uint64_t elapsed_ticks_ms, bool should_exit, bool frame_skip);

    // getters
    [[nodiscard]] bool should_exit() const noexcept;
    [[nodiscard]] bool frame_skip() const noexcept;

    /** during this tick, did the user toggle wireframe only vs. mesh view */
    [[nodiscard]] bool wireframe_display_mode_changed() const noexcept;
    [[nodiscard]] bool any_uniforms_modified() const noexcept;
    [[nodiscard]] bool function_params_modified() const noexcept;
    [[nodiscard]] bool model_modified() const noexcept;
    [[nodiscard]] bool view_modified() const noexcept;
    [[nodiscard]] bool tessellation_settings_modified() const noexcept;

    // setters
    void set_should_exit(bool should_exit) noexcept;
    void set_frame_skip(bool should_exit) noexcept;
    void set_wireframe_display_mode_toggled(bool show_wireframe_only) noexcept;
    void set_function_params_modified(bool function_params_modified) noexcept;
    void set_model_modified(bool model_modfied) noexcept;
    void set_view_modified(bool view_modified) noexcept;
    void set_tessellation_settings_modified(bool tessellation_settings_modified) noexcept;
};
