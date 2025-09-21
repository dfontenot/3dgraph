#pragma once

#include <SDL3/SDL.h>

#include <bitset>
#include <climits>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>
#include <version>

#if !__cpp_lib_ranges_join_with
#include <range/v3/all.hpp>
#else
#include <ranges>
#endif

/**
 * @brief std::equal_to for key mods
 * @details
 * use this when the exact modifiers (left and right side) do not matter
 * @tparam ignore_non_shift set to true to essentially ignore unsupported
 * modifiers ctrl and alt. usage is for considering these the same if a non-shift
 * modifier key is accidentally held
 */
template <bool ignore_non_shift = true> struct KeyModEquivalentEqualTo;

struct KeyMod {
    constexpr operator SDL_Keymod() const {
        return static_cast<SDL_Keymod>(val.to_ulong());
    }

    constexpr KeyMod() : val(SDL_KMOD_NONE) {
    }

    constexpr explicit KeyMod(SDL_Keymod mask) : val(mask) {
    }

    constexpr explicit KeyMod(std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> mask) : val(mask) {
    }

    [[nodiscard]] static constexpr KeyMod none() {
        return KeyMod{SDL_KMOD_NONE};
    }

    [[nodiscard]] static constexpr KeyMod shift() {
        return KeyMod{SDL_KMOD_SHIFT};
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

    KeyMod &set_shift(bool bit_val = true);
    KeyMod &set_alt(bool bit_val = true);
    KeyMod &set_ctrl(bool bit_val = true);
    KeyMod &set_lshift(bool bit_val = true);
    KeyMod &set_rshift(bool bit_val = true);
    KeyMod &set_lctrl(bool bit_val = true);
    KeyMod &set_rctrl(bool bit_val = true);
    KeyMod &set_lalt(bool bit_val = true);
    KeyMod &set_ralt(bool bit_val = true);

    /**
     * returns if the two key mods function the same for the
     * purposes of this application
     */
    [[nodiscard]] constexpr bool is_equivalent(KeyMod const &other) const {
        auto const other_has_ctrl = other.has_ctrl();
        auto const other_has_alt = other.has_alt();
        auto const other_has_shift = other.has_shift();

        return has_ctrl() == other_has_ctrl && has_alt() == other_has_alt && has_shift() == other_has_shift;
    }

    /**
     * new copy of this, but with normalized modifiers
     * meaning that the left and right side modifier keys
     * are pressed simultaneously so as to match the results of
     * SDL_GetScancodeFromKey
     */
    [[nodiscard]] KeyMod as_normalized() const;

    /**
     * adds modifiers to this keymod
     */
    KeyMod &add_mods(SDL_Keymod key_mod) {
        std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> new_mods{key_mod};
        val |= new_mods;
        return *this;
    }

    /**
     * new instance with the added on modifiers
     */
    [[nodiscard]] constexpr KeyMod with_more_mods(SDL_Keymod key_mod) const {
        std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> new_mods{key_mod};
        return KeyMod{val | new_mods};
    }

    /**
     * new instance with shif modifier active (both sides)
     */
    [[nodiscard]] constexpr KeyMod with_shifted() const {
        return with_more_mods(SDL_KMOD_SHIFT);
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
    friend bool operator==(const KeyMod &lhs, const KeyMod &rhs);
};

namespace std {
template <> struct hash<KeyMod> {
    std::size_t operator()(const KeyMod &key_mod) const {
        return std::hash<SDL_Keymod>{}(key_mod);
    }
};

template <> struct formatter<KeyMod> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const KeyMod &obj, FormatContext &ctx) const {
        using std::string;
        using std::unordered_set;

        unordered_set<string> mods;
        if (obj.has_alt()) {
            mods.insert("A");
        }

        if (obj.has_ctrl()) {
            mods.insert("C");
        }

        if (obj.has_shift()) {
            mods.insert("S");
        }

        if (mods.size() == 0) {
            mods.insert("N");
        }

#if !__cpp_lib_ranges_join_with
        return std::format_to(ctx.out(), "{0}", mods | ::ranges::views::join(',') | ::ranges::to<string>());
#else
        return std::format_to(ctx.out(), "{0}", mods | std::views::join_with(',') | std::ranges::to<string>());
#endif
    }
};
} // namespace std

/**
 */
template <bool ignore_non_shift> struct KeyModEquivalentEqualTo {
    constexpr bool operator()(const KeyMod &lhs, const KeyMod &rhs) const {
        if (ignore_non_shift) {
            return lhs.has_shift() == rhs.has_shift();
        }

        return lhs.is_equivalent(rhs);
    }
};

/**
 * use this hash specialization when the exact
 * modifiers that were pressed do not matter
 */
template <bool ignore_non_shift = true> struct KeyModEquivalentHash {
    std::size_t operator()(const KeyMod &key_mod) const {
        if (ignore_non_shift) {
            KeyMod copied{key_mod};
            copied.set_ctrl(false).set_alt(false);

            return std::hash<KeyMod>{}(copied.as_normalized());
        }

        return std::hash<KeyMod>{}(key_mod.as_normalized());
    }
};
