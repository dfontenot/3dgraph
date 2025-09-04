#include "key_mod.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <gtest/gtest.h>

#include <sstream>
#include <format>
#include <iostream>
#include <unordered_set>

using std::size_t;
using std::cout;

class KeyModTest : public ::testing::Test {
protected:
    static auto const any_keymod = SDL_KMOD_LSHIFT;
    static auto const any_other_keymod = SDL_KMOD_LCTRL;
};

TEST_F(KeyModTest, Ctors) {
    const KeyMod empty;
    EXPECT_EQ(empty, SDL_KMOD_NONE);

    const SDL_Keymod any_mods_raw = any_keymod | any_other_keymod;
    const KeyMod any_mods{any_mods_raw};
    EXPECT_EQ(any_mods, any_mods_raw);
}

TEST_F(KeyModTest, Display) {
    const KeyMod any{any_keymod};
    std::stringstream s;
    s << any;

    EXPECT_TRUE(s.str().size() > 0);
}

TEST_F(KeyModTest, Format) {
    const KeyMod any{any_keymod};
    auto const formatted = std::format("{}", any);

    EXPECT_TRUE(formatted.size() > 0);
}

TEST_F(KeyModTest, Equality) {
    const SDL_Keymod sdl_keymod = any_keymod | any_other_keymod;
    const KeyMod keymod{any_keymod | any_other_keymod};
    const KeyMod keymod2{sdl_keymod};

    EXPECT_EQ(keymod, sdl_keymod);
    EXPECT_EQ(keymod2, sdl_keymod);
}

TEST_F(KeyModTest, BitTest) {
    const SDL_Keymod first_bit_position = 1;
    const KeyMod mod{first_bit_position};

    // test bits are 0-indexed
    EXPECT_TRUE(mod.test(static_cast<size_t>(first_bit_position - 1)));
    EXPECT_FALSE(mod.test(static_cast<size_t>(first_bit_position)));
}

TEST_F(KeyModTest, Set) {
    KeyMod empty;
    size_t const any_bit_position = 0;
    EXPECT_FALSE(empty.test(any_bit_position));
    empty.set(any_bit_position);
    EXPECT_TRUE(empty.test(any_bit_position));
}

TEST_F(KeyModTest, HasNoMods) {
    const KeyMod empty;
    EXPECT_TRUE(empty.has_no_mods());
}

TEST_F(KeyModTest, Shift) {
    const KeyMod shift{SDL_KMOD_LSHIFT};
    const KeyMod shift_and_others{SDL_KMOD_LSHIFT | SDL_KMOD_LALT};
    
    EXPECT_TRUE(shift.has_lshift());
    EXPECT_TRUE(shift.has_shift());
    EXPECT_TRUE(shift_and_others.has_lshift());
    EXPECT_TRUE(shift_and_others.has_shift());
}

TEST_F(KeyModTest, Ctrl) {
    const KeyMod ctrl{SDL_KMOD_LCTRL};
    const KeyMod ctrl_and_others{SDL_KMOD_LCTRL | SDL_KMOD_LALT};
    
    EXPECT_TRUE(ctrl.has_lctrl());
    EXPECT_TRUE(ctrl.has_ctrl());
    EXPECT_TRUE(ctrl_and_others.has_lctrl());
    EXPECT_TRUE(ctrl_and_others.has_ctrl());
}

TEST_F(KeyModTest, Alt) {
    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod alt_and_others{SDL_KMOD_LCTRL | SDL_KMOD_LALT};
    
    EXPECT_TRUE(alt.has_lalt());
    EXPECT_TRUE(alt.has_alt());
    EXPECT_TRUE(alt_and_others.has_lalt());
    EXPECT_TRUE(alt_and_others.has_alt());
}

TEST_F(KeyModTest, Eq) {
    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    const KeyMod alt_also{SDL_KMOD_LALT};
    const KeyMod ctrl{SDL_KMOD_LCTRL};

    EXPECT_EQ(alt, alt_also);
    EXPECT_NE(alt, ctrl);
    EXPECT_NE(alt, r_alt);
}

TEST_F(KeyModTest, EqEquivalent) {
    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    const KeyMod alt_also{SDL_KMOD_LALT};
    const KeyMod ctrl{SDL_KMOD_LCTRL};

    EXPECT_TRUE(KeyModEquivalentEqualTo{}(alt, alt_also));
    EXPECT_FALSE(KeyModEquivalentEqualTo{}(alt, ctrl));
    EXPECT_TRUE(KeyModEquivalentEqualTo{}(alt, r_alt));
}

TEST_F(KeyModTest, IsEquivalent) {
    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    const KeyMod alt_also{SDL_KMOD_LALT};
    const KeyMod ctrl{SDL_KMOD_LCTRL};

    EXPECT_TRUE(alt.is_equivalent(alt_also));
    EXPECT_FALSE(alt.is_equivalent(ctrl));
    EXPECT_TRUE(alt.is_equivalent(r_alt));
}

TEST_F(KeyModTest, AsNormalized) {
    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    const KeyMod l_mult{SDL_KMOD_LALT | SDL_KMOD_LCTRL};
    const KeyMod mixed{SDL_KMOD_RALT | SDL_KMOD_RSHIFT | SDL_KMOD_LCTRL};
    const KeyMod l_mixed{SDL_KMOD_LALT | SDL_KMOD_LSHIFT | SDL_KMOD_LCTRL};

    EXPECT_EQ(alt, alt.as_normalized());
    EXPECT_NE(r_alt, alt.as_normalized());
    EXPECT_EQ(alt, r_alt.as_normalized());
    EXPECT_EQ(l_mult, l_mult.as_normalized());
    EXPECT_NE(mixed, mixed.as_normalized());
    EXPECT_EQ(l_mixed, mixed.as_normalized());
}

TEST_F(KeyModTest, Hash) {
    using std::unordered_set;

    unordered_set<KeyMod> set;

    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    set.insert(alt);
    set.insert(r_alt);

    EXPECT_EQ(2, set.size());
    EXPECT_TRUE(set.contains(alt));
    EXPECT_TRUE(set.contains(r_alt));

    const KeyMod alt_again{SDL_KMOD_LALT};
    set.insert(alt_again);
    EXPECT_EQ(2, set.size());
}

TEST_F(KeyModTest, HashEquivalent) {
    using std::unordered_set;

    unordered_set<KeyMod, KeyModEquivalentHash, KeyModEquivalentEqualTo> set;

    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    set.insert(alt);
    set.insert(r_alt);

    EXPECT_EQ(1, set.size());
    EXPECT_TRUE(set.contains(alt));
    EXPECT_TRUE(set.contains(r_alt));

    const KeyMod alt_again{SDL_KMOD_LALT};
    const KeyMod ctrl{SDL_KMOD_LCTRL};
    const KeyMod r_ctrl{SDL_KMOD_RCTRL};
    set.insert(alt_again);
    set.insert(ctrl);

    EXPECT_EQ(2, set.size());
    EXPECT_TRUE(set.contains(r_ctrl));
}

// TODO: the rest of the copy-paste tests
TEST_F(KeyModTest, SetLShift) {
    KeyMod mod{SDL_KMOD_LALT};
    EXPECT_TRUE(mod.has_lalt());
    EXPECT_FALSE(mod.has_lshift());
    EXPECT_FALSE(mod.has_rshift());

    mod.set_lshift();
    EXPECT_TRUE(mod.has_lalt());
    EXPECT_TRUE(mod.has_lshift());
    EXPECT_FALSE(mod.has_rshift());

    mod.set_rshift();
    EXPECT_TRUE(mod.has_lalt());
    EXPECT_TRUE(mod.has_lshift());
    EXPECT_TRUE(mod.has_rshift());

    mod.set_rshift(false);
    EXPECT_TRUE(mod.has_lalt());
    EXPECT_TRUE(mod.has_lshift());
    EXPECT_FALSE(mod.has_rshift());

    mod.set_lshift(false);
    EXPECT_TRUE(mod.has_lalt());
    EXPECT_FALSE(mod.has_lshift());
    EXPECT_FALSE(mod.has_rshift());
}
