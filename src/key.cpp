#include "key.hpp"
#include "key_mod.hpp"

#include <SDL3/SDL.h>

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <cstddef>
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
    auto key_code = SDL_GetKeyFromScancode(scan_code, key_mod, false);
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

bool operator==(const Key &lhs, const Key &rhs) {
    return lhs.scan_code == rhs.scan_code && lhs.key_mod == rhs.key_mod;
}

Key::Key(SDL_Scancode scan_code)
    : scan_code(scan_code), key_mod(SDL_KMOD_NONE), key_code(::maybe_key_from_scan_code(scan_code, SDL_KMOD_NONE)) {
}

Key::Key(SDL_Scancode scan_code, SDL_Keymod key_mod)
    : scan_code(scan_code), key_mod(key_mod), key_code(::maybe_key_from_scan_code(scan_code, key_mod)) {
}

Key::Key(SDL_Scancode scan_code, KeyMod key_mod) : scan_code(scan_code), key_mod(key_mod) {
}

Key::Key(pair<SDL_Scancode, SDL_Keymod> scan_code_with_mod)
    : scan_code(std::get<0>(scan_code_with_mod)), key_mod(std::get<1>(scan_code_with_mod)) {
    key_code = ::maybe_key_from_scan_code(scan_code, key_mod);
}

Key::Key(SDL_Keycode key_code) {
    this->key_code = key_code;
    SDL_Keymod mod = SDL_KMOD_NONE;
    this->scan_code = SDL_GetScancodeFromKey(key_code, &mod);
    this->key_mod = KeyMod{mod};
}

Key::Key(Keyish const &keyish) {
    using std::holds_alternative;

    if (holds_alternative<SDL_Scancode>(keyish)) {
        this->scan_code = std::get<SDL_Scancode>(keyish);
        this->key_mod = KeyMod::none();
        this->key_code = ::maybe_key_from_scan_code(this->scan_code, this->key_mod);
    }
    else if (holds_alternative<pair<SDL_Scancode, SDL_Keymod>>(keyish)) {
        auto const pair_ = std::get<pair<SDL_Scancode, SDL_Keymod>>(keyish);
        this->scan_code = std::get<0>(pair_);
        this->key_mod = KeyMod{std::get<1>(pair_)};
        this->key_code = ::maybe_key_from_scan_code(this->scan_code, this->key_mod);
    }
    else {
        this->key_code = std::get<SDL_Keycode>(keyish);
        SDL_Keymod mod = SDL_KMOD_NONE;
        this->scan_code = SDL_GetScancodeFromKey(*(this->key_code), &mod);
        this->key_mod = KeyMod{mod};
    }
}

Key Key::copy_shifted(bool only_keep_shift) const {
    if (only_keep_shift) {
        return Key{scan_code, KeyMod::shift()};
    }
    else {
        return Key{scan_code, key_mod.with_shifted()};
    }
}

Key Key::without_mods() const {
    return Key{scan_code};
}

Key Key::without_shift() const {
    KeyMod key_mod_copy{key_mod};
    key_mod_copy.set_lshift(false);
    key_mod_copy.set_rshift(false);
    return Key{scan_code, key_mod_copy};
}

Key Key::shift_mod_complement(bool only_modify_shift) const {
    if (only_modify_shift) {
        if (has_shift()) {
            return Key{scan_code};
        }
        else {
            return Key{scan_code, KeyMod::shift()};
        }
    }
    else {
        KeyMod key_mod_copy{key_mod};
        key_mod_copy.set_lshift(!has_shift());
        key_mod_copy.set_rshift(!has_shift());
        return Key{scan_code, key_mod_copy};
    }
}

Key Key::as_normalized() const {
    auto new_key_mod = key_mod.as_normalized();
    auto new_scan_code = get_equivalent_scan_code();

    if (new_scan_code == SDL_SCANCODE_LSHIFT) {
        new_key_mod.set_shift();
    }
    else if (new_scan_code == SDL_SCANCODE_LALT) {
        new_key_mod.set_alt();
    }
    else if (new_scan_code == SDL_SCANCODE_LCTRL) {
        new_key_mod.set_ctrl();
    }

    return Key{new_scan_code, new_key_mod};
}

SDL_Scancode Key::get_equivalent_scan_code() const {
    if (scan_code == SDL_SCANCODE_RSHIFT) {
        return SDL_SCANCODE_LSHIFT;
    }
    else if (scan_code == SDL_SCANCODE_RALT) {
        return SDL_SCANCODE_LALT;
    }
    else if (scan_code == SDL_SCANCODE_RCTRL) {
        return SDL_SCANCODE_LCTRL;
    }

    return scan_code;
}

Key &Key::set_shift(bool bit_val) {
    key_mod.set_shift(bit_val);
    key_code = ::maybe_key_from_scan_code(scan_code, key_mod);
    return *this;
}

Key &Key::set_alt(bool bit_val) {
    key_mod.set_alt(bit_val);
    return *this;
}

Key &Key::set_ctrl(bool bit_val) {
    key_mod.set_ctrl(bit_val);
    return *this;
}

Key &Key::set_lshift(bool bit_val) {
    key_mod.set_lshift(bit_val);
    key_code = ::maybe_key_from_scan_code(scan_code, key_mod);
    return *this;
}

Key &Key::set_rshift(bool bit_val) {
    key_mod.set_rshift(bit_val);
    key_code = ::maybe_key_from_scan_code(scan_code, key_mod);
    return *this;
}

Key &Key::set_lctrl(bool bit_val) {
    key_mod.set_lctrl(bit_val);
    return *this;
}

Key &Key::set_rctrl(bool bit_val) {
    key_mod.set_rctrl(bit_val);
    return *this;
}

Key &Key::set_lalt(bool bit_val) {
    key_mod.set_lalt(bit_val);
    return *this;
}

Key &Key::set_ralt(bool bit_val) {
    key_mod.set_ralt(bit_val);
    return *this;
}

Key Key::copy_with_mods(SDL_Keymod mods) const {
    return Key{scan_code, mods};
}
