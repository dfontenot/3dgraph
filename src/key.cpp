#include "key.hpp"
#include <SDL3/SDL.h>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <variant>

using std::size_t;
using std::variant;

std::ostream &operator<<(std::ostream &stream, const Key &key) {
    stream << "{ Key " << SDL_GetScancodeName(key.scan_code) << " : scan " << key.scan_code << " mod " << key.key_mod << " key " << key.key_code << " }";
    return stream;
}

template <> struct std::formatter<Key> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const Key &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{ Key {0} : scan {1} mod {2} key {3} }", SDL_GetScancodeName(obj.scan_code), obj.scan_code, obj.key_mod, obj.key_code);
    }
};

/**
 * for the purposes of this application, equality does not mean strict
 * equality, just means if the keys are equivalent
 */
bool operator==(const Key &lhs, const Key &rhs) {
    // TODO: fix the check if both are shifted isn't exactly correct as one could have
    // other modifiers applied as well
    return ((lhs.is_scancode_shift() && rhs.is_scancode_shift()) || lhs.scan_code == rhs.scan_code) &&
           ((lhs.has_shift() && rhs.has_shift()) || lhs.key_mod == rhs.key_mod);
}

/**
 * for the purposes of this application, equivalent keys will hash to the same value
 */
size_t KeyHash::operator()(const Key &key) const {
    const SDL_Scancode equivalent_scan_code = key.is_scancode_shift() ? SDL_SCANCODE_LSHIFT : key.scan_code;
    const SDL_Keymod equivalent_key_mod = key.has_shift() ? SDL_KMOD_SHIFT : key.key_mod;

    size_t scan_code_hash = std::hash<SDL_Scancode>{}(equivalent_scan_code);
    size_t key_mod_hash = std::hash<SDL_Keymod>{}(equivalent_key_mod);
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

Key Key::shift_mod_complement() const {
    if (has_modifier()) {
        return Key(scan_code);
    }
    else {
        return Key(scan_code, SDL_KMOD_SHIFT);
    }
}

