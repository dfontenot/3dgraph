#pragma once

#include <SDL3/SDL.h>

#include <SDL3/SDL_keycode.h>
#include <format>
#include <iostream>
#include <variant>
#include <optional>
#include <utility>

using Keyish = std::variant<SDL_Scancode, SDL_Keycode, std::pair<SDL_Scancode, SDL_Keymod>>;

class KeyHash;

class Key {
    SDL_Scancode scan_code;
    std::optional<SDL_Keycode> key_code;
    SDL_Keymod key_mod;

    friend bool operator==(const Key &lhs, const Key &rhs);
    friend KeyHash;
    friend std::ostream &operator<<(std::ostream &stream, const Key &key);
    friend std::formatter<Key>;

public:
    Key() = delete;
    explicit Key(SDL_Scancode scan_code);
    explicit Key(SDL_Scancode scan_code, SDL_Keymod key_mod);
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
        return (key_mod & SDL_KMOD_LSHIFT) || (key_mod & SDL_KMOD_RSHIFT);
    }

    /**
     * were either of the ctrl keys pressed down
     */
    [[nodiscard]] constexpr bool has_ctrl() const {
        return (key_mod & SDL_KMOD_LCTRL) || (key_mod & SDL_KMOD_RCTRL);
    }

    /**
     * were either of the alt keys pressed down
     */
    [[nodiscard]] constexpr bool has_alt() const {
        return (key_mod & SDL_KMOD_LALT) || (key_mod & SDL_KMOD_RALT);
    }

    [[nodiscard]] constexpr bool has_modifier() const {
        return key_mod != SDL_KMOD_NONE;
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

    [[nodiscard]] constexpr Key copy_with_mods(SDL_Keymod mods) const {
        return {scan_code, key_code, mods};
    }

    /**
     * new copy of this key but with the shift modifier applied
     */
    [[nodiscard]] Key copy_shifted(bool only_keep_shift = true) const;

    /**
     * new copy of this key without any modifiers
     */
    [[nodiscard]] Key without_mods() const;

    /**
     * a new copy of this key with the shift modifier
     * if it isn't applied to this, or without it if it
     * already is applied
     */
    [[nodiscard]] Key shift_mod_complement(bool only_keey_shift = true) const;
};

struct KeyHash {
    size_t operator()(const Key &key) const;
};
