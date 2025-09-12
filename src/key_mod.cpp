#include "key_mod.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
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

KeyMod KeyMod::as_normalized() const {
    KeyMod copied{*this};

    if (copied.has_ralt() != copied.has_lalt()) {
        copied.set_lalt();
        copied.set_ralt();
    }

    if (copied.has_rctrl() != copied.has_lctrl()) {
        copied.set_lctrl();
        copied.set_rctrl();
    }

    if (copied.has_rshift() != copied.has_lshift()) {
        copied.set_lshift();
        copied.set_rshift();
    }

    return copied;
}

KeyMod &KeyMod::set_shift(bool bit_val) {
    set_lshift(bit_val);
    set_rshift(bit_val);

    return *this;
}

KeyMod &KeyMod::set_alt(bool bit_val) {
    set_lalt(bit_val);
    set_ralt(bit_val);

    return *this;
}

KeyMod &KeyMod::set_ctrl(bool bit_val) {
    set_lctrl(bit_val);
    set_rctrl(bit_val);

    return *this;
}
