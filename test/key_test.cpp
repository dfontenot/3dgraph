#include "key.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <gtest/gtest.h>
#include <unordered_set>
#include <variant>

class KeyTest : public ::testing::Test {
protected:
    static auto const any_scancode = SDL_SCANCODE_D;
    static auto const any_other_scancode = SDL_SCANCODE_T;
    static auto const any_keymod = SDL_KMOD_CTRL;
    static auto const any_keycode = SDLK_D;
};

TEST_F(KeyTest, CtorScancodeOnly) {
    auto const key = Key(any_scancode);
    EXPECT_FALSE(key.has_modifier());
}

TEST_F(KeyTest, CtorWithMod) {
    auto const key = Key(any_scancode, any_keymod);
    EXPECT_TRUE(key.has_modifier());
    EXPECT_FALSE(key.has_shift());
}

TEST_F(KeyTest, CtorWithAllCodes) {
    auto const e_key_scan_code = Key(SDL_SCANCODE_E);
    auto const e_key_key_code = Key(SDLK_E);
    EXPECT_EQ(e_key_key_code, e_key_scan_code);
}

TEST_F(KeyTest, CtorFromVariant) {
    // choosing key has consistent keyboard placement in qwerty and azerty
    Keyish const scan_code_variant{SDL_SCANCODE_E};
    Keyish const key_code_variant{SDLK_E};
    EXPECT_NE(scan_code_variant, key_code_variant);

    const Key key_from_scan_code{scan_code_variant};
    const Key key_from_key_code{key_code_variant};

    EXPECT_EQ(key_from_scan_code, key_from_key_code);
}

TEST_F(KeyTest, Getters) {
    auto const key = Key(any_scancode, any_keymod);
    auto const expected_keymod = any_keymod;
    auto const expected_scancode = any_scancode;
    auto const expected_keycode = any_keycode;

    /*
     * NOTE: these macros appear to be doing something, only when inside of a TEST_F,
     * that the linker doesn't like, appears that can't put anything in the
     * derived test into the assertion macros, have to copy it
     */
    EXPECT_EQ(key.get_key_mod(), expected_keymod);
    EXPECT_EQ(key.get_scan_code(), expected_scancode);
    EXPECT_EQ(key.get_key_code(), expected_keycode);
}

TEST_F(KeyTest, CopyShifted) {
    auto const key_no_shift_mod = Key(any_scancode);
    auto const key_no_shift_mod_also = Key(any_scancode, SDL_KMOD_NONE);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);

    EXPECT_EQ(key_no_shift_mod.copy_shifted(), key_shifted);
    EXPECT_EQ(key_no_shift_mod_also.copy_shifted(), key_shifted);
    EXPECT_EQ(key_shifted.copy_shifted(), key_shifted);
}

TEST_F(KeyTest, Eq) {
    auto const key = Key(any_scancode);
    auto const other_key = Key(any_other_scancode);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);
    auto const key_alt = Key(any_scancode, SDL_KMOD_ALT);

    EXPECT_FALSE(key == key_shifted);
    EXPECT_FALSE(key == other_key);
    EXPECT_TRUE(key == key);
    EXPECT_FALSE(key_shifted == key_alt);
}

TEST_F(KeyTest, WithoutMods) {
    auto const key = Key(any_scancode);
    auto const key_alt = Key(any_scancode, any_keymod);

    EXPECT_EQ(key_alt.without_mods(), key);
    EXPECT_EQ(key.without_mods(), key);
}

TEST_F(KeyTest, ShiftModComplement) {
    auto const key = Key(any_scancode);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);

    EXPECT_EQ(key, key_shifted.shift_mod_complement());
    EXPECT_EQ(key_shifted, key.shift_mod_complement());
}

TEST_F(KeyTest, Hash) {
    using std::unordered_set;

    unordered_set<Key, KeyHash> set;

    auto const key = Key(any_scancode);
    EXPECT_FALSE(set.contains(key));

    set.insert(key);
    EXPECT_TRUE(set.contains(key));
    EXPECT_EQ(1, set.size());

    set.insert(key);
    EXPECT_TRUE(set.contains(key));
    EXPECT_EQ(1, set.size());

    // ensure keys with different modifiers
    // are differentiated
    auto const key2 = Key(any_scancode, any_keymod);
    set.insert(key2);
    EXPECT_TRUE(set.contains(key));
    EXPECT_TRUE(set.contains(key2));
    EXPECT_EQ(2, set.size());

    // ensure can add keys only differing
    // in scancode
    auto const key3 = Key(any_other_scancode);
    set.insert(key3);
    EXPECT_TRUE(set.contains(key));
    EXPECT_TRUE(set.contains(key2));
    EXPECT_TRUE(set.contains(key3));
    EXPECT_EQ(3, set.size());
}
