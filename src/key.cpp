#include "key.hpp"

#include <SDL3/SDL.h>

#include <cstddef>
#include <functional>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

using std::make_optional;
using std::nullopt;
using std::optional;
using std::ostream;
using std::pair;
using std::size_t;
using std::variant;

namespace {
optional<SDL_Keycode> maybe_key_from_scan_code(SDL_Scancode scan_code, SDL_Keymod key_mod = SDL_KMOD_NONE) {
    auto key_code = SDL_GetKeyFromScancode(scan_code, SDL_KMOD_NONE, true);
    if (key_code > SDLK_RHYPER) {
        return nullopt;
    }
    else {
        return make_optional(key_code);
    }
}
} // namespace

ostream &operator<<(ostream &stream, const Key &key) {
    stream << "{ Key " << SDL_GetScancodeName(key.scan_code) << " : scan " << key.scan_code << " mod " << key.key_mod
           << " key " << key.key_code.transform([](auto code) { return std::to_string(code); }).value_or("n/a") << " }";

    return stream;
}

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
    : scan_code(scan_code), key_mod(SDL_KMOD_NONE), key_code(::maybe_key_from_scan_code(scan_code, SDL_KMOD_NONE)) {
}

Key::Key(SDL_Scancode scan_code, SDL_Keymod key_mod)
    : scan_code(scan_code), key_mod(key_mod), key_code(::maybe_key_from_scan_code(scan_code, key_mod)) {
}

Key::Key(pair<SDL_Scancode, SDL_Keymod> scan_code_with_mod)
    : scan_code(std::get<0>(scan_code_with_mod)), key_mod(std::get<1>(scan_code_with_mod)),
      key_code(::maybe_key_from_scan_code(scan_code, key_mod)) {
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
        this->key_code = ::maybe_key_from_scan_code(this->scan_code, this->key_mod);
    }
    else if (holds_alternative<pair<SDL_Scancode, SDL_Keymod>>(keyish)) {
        auto const pair_ = std::get<pair<SDL_Scancode, SDL_Keymod>>(keyish);
        this->scan_code = std::get<0>(pair_);
        this->key_mod = std::get<1>(pair_);
        this->key_code = ::maybe_key_from_scan_code(this->scan_code, this->key_mod);
    }
    else {
        this->key_code = std::get<SDL_Keycode>(keyish);
        this->scan_code = SDL_GetScancodeFromKey(*(this->key_code), &key_mod);
    }
}

Key Key::copy_shifted(bool only_keep_shift) const {
    if (only_keep_shift) {
        return Key{scan_code, SDL_KMOD_SHIFT};
    }
    else {
        return Key{scan_code, static_cast<SDL_Keymod>(key_mod | SDL_KMOD_SHIFT)};
    }
}

Key Key::without_mods() const {
    return Key{scan_code};
}

Key Key::shift_mod_complement(bool only_keep_shift) const {
    if (only_keep_shift) {
        if (has_shift()) {
            return Key{scan_code};
        }
        else {
            return Key{scan_code, SDL_KMOD_SHIFT};
        }
    }
    else {
        if (has_shift()) {
            return Key{scan_code, static_cast<SDL_Keymod>(key_mod & ~SDL_KMOD_SHIFT)};
        }
        else {
            return Key{scan_code, static_cast<SDL_Keymod>(key_mod | SDL_KMOD_SHIFT)};
        }
    }
}
