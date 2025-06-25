#pragma once

#include <SDL3/SDL.h>
#include <format>
#include <iostream>
#include <variant>

using Keyish = std::variant<SDL_Scancode, SDL_Keycode>;

class KeyHash;

class Key {
    SDL_Scancode scan_code;
    SDL_Keycode key_code;
    SDL_Keymod key_mod;

    friend bool operator==(const Key &lhs, const Key &rhs);
    friend KeyHash;
    friend std::ostream &operator<<(std::ostream &stream, const Key &key);
    friend std::formatter<Key>;

public:
    Key() = delete;
    explicit Key(SDL_Scancode scan_code);
    explicit Key(SDL_Scancode scan_code, SDL_Keymod key_mod);

    constexpr Key(SDL_Scancode scan_code, SDL_Keycode key_code, SDL_Keymod key_mod)
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

    [[nodiscard]] constexpr SDL_Keycode get_key_code() const {
        return key_code;
    }

    [[nodiscard]] constexpr bool has_shift() const {
        return key_mod == SDL_KMOD_LSHIFT || key_mod == SDL_KMOD_RSHIFT || key_mod == SDL_KMOD_SHIFT;
    }

    [[nodiscard]] constexpr bool has_modifier() const {
        return key_mod != SDL_KMOD_NONE;
    }

    [[nodiscard]] constexpr bool is_scancode_shift() const {
        return scan_code == SDL_SCANCODE_LSHIFT || scan_code == SDL_SCANCODE_RSHIFT;
    }

    /**
     * new copy of this key but with the shift modifier applied
     */
    [[nodiscard]] Key copy_shifted(bool only_this_mod = true) const;

    /**
     * new copy of this key without any modifiers
     */
    [[nodiscard]] Key without_mods() const;

    /**
     * a new copy of this key with the shift modifier
     * if it isn't applied to this, or without it if it
     * already is applied
     */
    [[nodiscard]] Key shift_mod_complement() const;
};

struct KeyHash {
    size_t operator()(const Key &key) const;
};
