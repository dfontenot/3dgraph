#include "key_mod.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <functional>
#include <iostream>

using std::ostream;
using std::size_t;

ostream &operator<<(ostream &stream, const KeyMod &key) {
    stream << key.val;
    return stream;
}

bool operator==(const KeyMod &lhs, const KeyMod &rhs) {
    return lhs.val == rhs.val;
}

size_t KeyModEquivalentHash::operator()(const KeyMod &key) const {
    return std::hash<KeyMod>{}(key.as_normalized());
}

KeyMod &KeyMod::set_lshift(bool bit_val) {
    if (bit_val) {
        val |= lshift;
    }
    else {
        val &= ~lshift;
    }

    return *this;
}

KeyMod &KeyMod::set_rshift(bool bit_val) {
    if (bit_val) {
        val |= rshift;
    }
    else {
        val &= ~rshift;
    }

    return *this;
}

KeyMod &KeyMod::set_lctrl(bool bit_val) {
    if (bit_val) {
        val |= lctrl;
    }
    else {
        val &= ~lctrl;
    }

    return *this;
}

KeyMod &KeyMod::set_rctrl(bool bit_val) {
    if (bit_val) {
        val |= rctrl;
    }
    else {
        val &= ~rctrl;
    }

    return *this;
}

KeyMod &KeyMod::set_lalt(bool bit_val) {
    if (bit_val) {
        val |= lalt;
    }
    else {
        val &= ~lalt;
    }

    return *this;
}

KeyMod &KeyMod::set_ralt(bool bit_val) {
    if (bit_val) {
        val |= ralt;
    }
    else {
        val &= ~ralt;
    }

    return *this;
}

/**
 * normalized in this context just means all relevant modifiers are only
 * pressed on the left side (arbitrary)
 */
KeyMod KeyMod::as_normalized() const {
    KeyMod copied{*this};

    if (copied.has_ralt()) {
        copied.set_lalt();
        copied.set_ralt(false);
    }

    if (copied.has_rshift()) {
        copied.set_lshift();
        copied.set_rshift(false);
    }

    if (copied.has_rctrl()) {
        copied.set_lctrl();
        copied.set_rctrl(false);
    }

    return copied;
}
