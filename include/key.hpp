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
    friend std::ostream& operator<<(std::ostream& stream, const Key& key);
    friend std::formatter<Key>;

public:
    Key() = delete;
    explicit Key(SDL_Scancode scan_code);
    explicit Key(SDL_Scancode scan_code, SDL_Keymod key_mod);
    explicit constexpr Key(SDL_Scancode scan_code, SDL_Keycode key_code, SDL_Keymod key_mod);
    explicit Key(SDL_Keycode key_code);
    Key(Keyish const &keyish);

    SDL_Scancode get_scan_code() const;
    SDL_Keymod get_key_mod() const;
    SDL_Keycode get_key_code() const;
    bool has_modifier() const;
    bool has_shift() const;
    bool is_scancode_shift() const;

    /**
     * new copy of this key but with the shift modifier applied
     */
    Key copy_shifted(bool only_this_mod = true) const;

    /**
     * new copy of this key without any modifiers
     */
    Key without_mods() const;

    /**
     * a new copy of this key with the shift modifier
     * if it isn't applied to this, or without it if it
     * already is applied
     */
    Key shift_mod_complement() const;
};

struct KeyHash {
    size_t operator()(const Key &key) const;
};
