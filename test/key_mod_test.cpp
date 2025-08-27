#include "key_mod.hpp"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <sstream>
#include <format>
#include <iostream>

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
