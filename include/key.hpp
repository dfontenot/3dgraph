#pragma once

#include <key_mod.hpp>

#include <SDL3/SDL.h>

#include <format>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

using Keyish = std::variant<SDL_Scancode, SDL_Keycode, std::pair<SDL_Scancode, SDL_Keymod>>;

template <bool ignore_non_shift = true> struct KeyEquivalentHash;

template <bool ignore_non_shift = true> struct KeyEquivalentEqualTo;

class Key {
    SDL_Scancode scan_code;
    std::optional<SDL_Keycode> key_code;
    KeyMod key_mod;

    friend KeyEquivalentHash<>;
    friend KeyEquivalentEqualTo<>;
    friend bool operator==(const Key &lhs, const Key &rhs);
    friend std::ostream &operator<<(std::ostream &stream, const Key &key);
    friend std::formatter<Key>;

public:
    Key() = delete;
    // TODO: deferred sdl_keycode lookup for more constexpr stuff
    explicit Key(SDL_Scancode scan_code);
    explicit Key(SDL_Scancode scan_code, SDL_Keymod key_mod);
    explicit Key(SDL_Scancode scan_code, KeyMod key_mod);
    explicit Key(std::pair<SDL_Scancode, SDL_Keymod> scan_code_with_mod);

    constexpr Key(SDL_Scancode scan_code, SDL_Keycode key_code, SDL_Keymod key_mod)
        : scan_code(scan_code), key_code(key_code), key_mod(key_mod) {
    }

    constexpr Key(SDL_Scancode scan_code, std::optional<SDL_Keycode> key_code, SDL_Keymod key_mod)
        : scan_code(scan_code), key_code(key_code), key_mod(key_mod) {
    }

    explicit Key(SDL_Keycode key_code);
    Key(Keyish const &keyish);

    [[nodiscard]] constexpr SDL_Scancode get_scan_code() const {
        return scan_code;
    }

    [[nodiscard]] constexpr SDL_Keymod get_key_mod() const {
        return key_mod;
    }

    [[nodiscard]] constexpr bool has_key_code() const {
        return key_code.has_value();
    }

    [[nodiscard]] constexpr std::optional<SDL_Keycode> get_key_code() const {
        return key_code;
    }

    /**
     * were either of the shift keys pressed down
     */
    [[nodiscard]] constexpr bool has_shift() const {
        return key_mod.has_shift();
    }

    /**
     * were either of the ctrl keys pressed down
     */
    [[nodiscard]] constexpr bool has_ctrl() const {
        return key_mod.has_ctrl();
    }

    /**
     * were either of the alt keys pressed down
     */
    [[nodiscard]] constexpr bool has_alt() const {
        return key_mod.has_alt();
    }

    [[nodiscard]] constexpr bool has_modifier() const {
        return !key_mod.has_no_mods();
    }

    [[nodiscard]] constexpr bool is_scancode_shift() const {
        return scan_code == SDL_SCANCODE_LSHIFT || scan_code == SDL_SCANCODE_RSHIFT;
    }

    /**
     * is the key a letter key
     */
    [[nodiscard]] constexpr bool is_alpha() const {
        return scan_code >= SDL_SCANCODE_A && scan_code <= SDL_SCANCODE_Z;
    }

    /**
     * is the key a number key
     */
    [[nodiscard]] constexpr bool is_numeric() const {
        return (scan_code >= SDL_SCANCODE_1 && scan_code <= SDL_SCANCODE_0) && !has_shift();
    }

    [[nodiscard]] constexpr bool is_alphanum() const {
        return is_alpha() || is_numeric();
    }

    [[nodiscard]] Key copy_with_mods(SDL_Keymod mods) const;

    /**
     * new copy of this key but with the shift modifier applied
     * (both left and right)
     */
    [[nodiscard]] Key copy_shifted(bool only_keep_shift = true) const;

    /**
     * new copy of this key without any modifiers
     */
    [[nodiscard]] Key without_mods() const;

    /**
     * new copy of this key without shift modifiers
     */
    [[nodiscard]] Key without_shift() const;

    /**
     * new copy with normalized scan codes and modifiers
     */
    [[nodiscard]] Key as_normalized() const;

    /**
     * a new copy of this key with the shift modifier
     * if it isn't applied to this, or without it if it
     * already is applied
     */
    [[nodiscard]] Key shift_mod_complement(bool only_modify_shift = true) const;

    /**
     * account for differences in left and right equivalent keys
     */
    [[nodiscard]] SDL_Scancode get_equivalent_scan_code() const;

    Key &set_shift(bool bit_val = true);
    Key &set_alt(bool bit_val = true);
    Key &set_ctrl(bool bit_val = true);
    Key &set_lshift(bool bit_val = true);
    Key &set_rshift(bool bit_val = true);
    Key &set_lctrl(bool bit_val = true);
    Key &set_rctrl(bool bit_val = true);
    Key &set_lalt(bool bit_val = true);
    Key &set_ralt(bool bit_val = true);
};

namespace std {
template <> struct hash<Key> {
    std::size_t operator()(const Key &key) const {
        std::size_t scan_code_hash = std::hash<SDL_Scancode>{}(key.get_scan_code());
        std::size_t key_mod_hash = std::hash<SDL_Keymod>{}(key.get_key_mod());
        return scan_code_hash ^ (key_mod_hash << 1);
    }
};

template <> struct formatter<Key, char> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const Key &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "< Key {0} : scan {1} mod {2} key {3} >", SDL_GetScancodeName(obj.scan_code),
                              std::to_string(obj.scan_code), obj.key_mod,
                              obj.key_code.transform([](auto code) { return std::to_string(code); }).value_or("n/a"));
    }
};

} // namespace std

/**
 * use this hash specialization when the exact
 * scan code and modifiers that were pressed do not matter
 */
template <bool ignore_non_shift> struct KeyEquivalentHash {
    size_t operator()(const Key &key) const {
        std::size_t scan_code_hash = std::hash<SDL_Scancode>{}(key.get_equivalent_scan_code());
        std::size_t key_mod_hash = KeyModEquivalentHash<ignore_non_shift>{}(key.key_mod);
        return scan_code_hash ^ (key_mod_hash << 1);
    }
};

/**
 * use this equal_to when the exact
 * scan code and modifiers do not matter
 */
template <bool ignore_non_shift> struct KeyEquivalentEqualTo {
    constexpr bool operator()(const Key &lhs, const Key &rhs) const {
        auto lhs_norm = lhs.as_normalized();
        auto rhs_norm = rhs.as_normalized();

        if (ignore_non_shift) {
            lhs_norm.set_alt(false);
            rhs_norm.set_alt(false);
            lhs_norm.set_ctrl(false);
            rhs_norm.set_ctrl(false);
        }

        return lhs.as_normalized() == rhs.as_normalized();
    }
};
