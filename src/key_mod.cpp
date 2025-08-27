#include "key_mod.hpp"

#include <iostream>

using std::ostream;

ostream &operator<<(ostream &stream, const KeyMod &key) {
    stream << key.val;
    return stream;
}
