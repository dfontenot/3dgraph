#include "key.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <cstddef>
#include <functional>
#include <iostream>
#include <variant>

using std::size_t;
using std::variant;

std::ostream &operator<<(std::ostream &stream, const Key &key) {
    stream << "{ Key: scan " << key.scan_code << " mod " << key.key_mod << " key " << key.key_code << " }";
    return stream;
}

template <> struct std::formatter<Key> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const Key &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{ Key scan {0} mod {1} key {2} }", obj.scan_code, obj.key_mod, obj.key_code);
    }
};

bool operator==(const Key &lhs, const Key &rhs) {
    return lhs.scan_code == rhs.scan_code && lhs.key_mod == rhs.key_mod && lhs.key_code == rhs.key_code;
}

size_t KeyHash::operator()(const Key &key) const {
    size_t scan_code_hash = std::hash<SDL_Scancode>{}(key.scan_code);
    size_t key_mod_hash = std::hash<SDL_Keymod>{}(key.key_mod);
    return scan_code_hash ^ (key_mod_hash << 1);
}

Key::Key(SDL_Scancode scan_code)
    : scan_code(scan_code), key_mod(SDL_KMOD_NONE), key_code(SDL_GetKeyFromScancode(scan_code, SDL_KMOD_NONE, true)) {
}

Key::Key(SDL_Scancode scan_code, SDL_Keymod key_mod)
    : scan_code(scan_code), key_mod(key_mod), key_code(SDL_GetKeyFromScancode(scan_code, key_mod, true)) {
}

Key::Key(SDL_Keycode key_code) {
    this->key_code = key_code;
    this->scan_code = SDL_GetScancodeFromKey(key_code, &key_mod);
}

constexpr Key::Key(SDL_Scancode scan_code, SDL_Keycode key_code, SDL_Keymod key_mod)
    : scan_code(scan_code), key_code(key_code), key_mod(key_mod) {
}

Key::Key(Keyish const &keyish) {
    using std::holds_alternative;

    if (holds_alternative<SDL_Scancode>(keyish)) {
        this->scan_code = std::get<SDL_Scancode>(keyish);
        this->key_mod = SDL_KMOD_NONE;
        this->key_code = SDL_GetKeyFromScancode(this->scan_code, this->key_mod, true);
    }
    else {
        this->key_code = std::get<SDL_Keycode>(keyish);
        this->scan_code = SDL_GetScancodeFromKey(this->key_code, &key_mod);
    }
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

Key Key::shift_mod_complement() const {
    if (has_modifier()) {
        return Key(scan_code);
    }
    else {
        return Key(scan_code, SDL_KMOD_SHIFT);
    }
}

SDL_Keycode Key::get_key_code() const {
    return key_code;
}
