#include "key.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <cstddef>
#include <functional>

using std::size_t;

bool operator==(const Key &lhs, const Key &rhs) {
    return lhs.scan_code == rhs.scan_code && lhs.key_mod == rhs.key_mod;
}

size_t KeyHash::operator()(const Key &key) const {
    size_t scan_code_hash = std::hash<SDL_Scancode>{}(key.scan_code);
    size_t key_mod_hash = std::hash<SDL_Keymod>{}(key.key_mod);
    return scan_code_hash ^ (key_mod_hash << 1);
}

SDL_Scancode Key::get_scan_code() const {
    return scan_code;
}
SDL_Keymod Key::get_key_mod() const {
    return key_mod;
}

bool Key::has_modifier() const {
    return key_mod != SDL_KMOD_NONE;
}

Key Key::copy_shifted(bool only_this_mod) const {
    if (only_this_mod) {
        return Key(scan_code, SDL_KMOD_SHIFT);
    }
    else {
        return Key(scan_code, key_mod | SDL_KMOD_SHIFT);
    }
}

Key Key::without_mods() const {
    return Key(scan_code);
}

bool Key::has_shift() const {
    return key_mod == SDL_KMOD_LSHIFT || key_mod == SDL_KMOD_RSHIFT;
}
