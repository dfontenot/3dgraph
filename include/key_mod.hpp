#pragma once

#include <SDL3/SDL.h>

#include <bitset>
#include <climits>
#include <format>
#include <iostream>

struct KeyMod {
    constexpr operator SDL_Keymod() const {
        return static_cast<SDL_Keymod>(val.to_ulong());
    }

    constexpr KeyMod() : val(SDL_KMOD_NONE) {
    }

    constexpr explicit KeyMod(SDL_Keymod mask) : val(mask) {
    }

    /**
     * same as bitset::test
     * @param[in] bit_position 0-indexed bit position to check if set
     */
    [[nodiscard]] constexpr bool test(std::size_t bit_position) const {
        return val.test(bit_position);
    }

    /**
     * same as bitset::operator[]
     * @param[in] bit_position 0-indexed bit position to check if set
     */
    constexpr void set(std::size_t bit_position, bool bit_val = true) {
        val[bit_position] = bit_val;
    }

    [[nodiscard]] constexpr bool has_no_mods() const {
        return val == SDL_KMOD_NONE;
    }

    [[nodiscard]] constexpr bool has_lshift() const {
        return (val & lshift).any();
    }

    [[nodiscard]] constexpr bool has_rshift() const {
        return (val & rshift).any();
    }

    [[nodiscard]] constexpr bool has_shift() const {
        return has_lshift() || has_rshift();
    }

    [[nodiscard]] constexpr bool has_lctrl() const {
        return (val & lctrl).any();
    }

    [[nodiscard]] constexpr bool has_rctrl() const {
        return (val & rctrl).any();
    }

    [[nodiscard]] constexpr bool has_ctrl() const {
        return has_lctrl() || has_rctrl();
    }

    [[nodiscard]] constexpr bool has_lalt() const {
        return (val & lalt).any();
    }

    [[nodiscard]] constexpr bool has_ralt() const {
        return (val & ralt).any();
    }

    [[nodiscard]] constexpr bool has_alt() const {
        return has_lalt() || has_ralt();
    }

private:
    constexpr static std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> lshift{SDL_KMOD_LSHIFT};
    constexpr static std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> rshift{SDL_KMOD_RSHIFT};
    constexpr static std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> lctrl{SDL_KMOD_LCTRL};
    constexpr static std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> rctrl{SDL_KMOD_RCTRL};
    constexpr static std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> lalt{SDL_KMOD_LALT};
    constexpr static std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> ralt{SDL_KMOD_RALT};

    std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> val;

    friend std::ostream &operator<<(std::ostream &stream, const KeyMod &key);
    friend std::formatter<KeyMod>;
};

namespace std {
template <> struct formatter<KeyMod> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const KeyMod &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{0}", obj.val.to_string());
    }
};
} // namespace std
