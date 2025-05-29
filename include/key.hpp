#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

class KeyHash;

class Key {
    SDL_Scancode scan_code;
    SDL_Keymod key_mod;

    friend bool operator==(const Key &lhs, const Key &rhs);
    friend KeyHash;

public:
    Key() = delete;
    Key(SDL_Scancode scan_code) : scan_code(scan_code), key_mod(SDL_KMOD_NONE) {
    }

    Key(SDL_Scancode scan_code, SDL_Keymod key_mod) : scan_code(scan_code), key_mod(key_mod) {
    }

    SDL_Scancode get_scan_code() const;
    SDL_Keymod get_key_mod() const;
    bool has_modifier() const;
    bool has_shift() const;

    /**
     * new copy of this key but with the shift modifier applied
     */
    Key copy_shifted(bool only_this_mod) const;

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
