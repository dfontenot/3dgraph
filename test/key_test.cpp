#include "key.hpp"
#include "sdl_test.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <gtest/gtest.h>
#include <unordered_set>

class KeyTest : public SDLTest {
protected:
    static auto const any_scancode = SDL_SCANCODE_D;
    static auto const any_other_scancode = SDL_SCANCODE_T;
    static auto const yet_another_scancode = SDL_SCANCODE_Z;
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

TEST_F(KeyTest, FromKeyCodeHasCorrectMod) {
    const Key from_key_code{SDLK_PLUS};
    const Key from_scan_code{SDL_SCANCODE_EQUALS, SDL_KMOD_SHIFT};

    EXPECT_EQ(from_scan_code, from_key_code);
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
    auto const key_left_shifted = Key(any_scancode, SDL_KMOD_LSHIFT);
    auto const key_right_shifted = Key(any_scancode, SDL_KMOD_RSHIFT);
    auto const key_alt = Key(any_scancode, SDL_KMOD_ALT);
    auto const left_shift_only = Key(SDL_SCANCODE_LSHIFT);
    auto const right_shift_only = Key(SDL_SCANCODE_RSHIFT);

    EXPECT_FALSE(key == key_shifted);
    EXPECT_FALSE(key == other_key);
    EXPECT_TRUE(key == key);
    EXPECT_FALSE(key_shifted == key_alt);
    EXPECT_TRUE(key_left_shifted == key_shifted);
    EXPECT_TRUE(key_right_shifted == key_shifted);
    EXPECT_TRUE(key_left_shifted == key_right_shifted);
    EXPECT_TRUE(left_shift_only == right_shift_only);
}

TEST_F(KeyTest, IsScanCodeShift) {
    auto const key = Key(any_scancode);
    auto const left_shift_only = Key(SDL_SCANCODE_LSHIFT);
    auto const right_shift_only = Key(SDL_SCANCODE_RSHIFT);

    EXPECT_TRUE(left_shift_only.is_scancode_shift());
    EXPECT_TRUE(right_shift_only.is_scancode_shift());
    EXPECT_FALSE(key.is_scancode_shift());
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

TEST_F(KeyTest, HasShfit) {
    auto const key1 = Key(any_scancode, SDL_KMOD_SHIFT);
    auto const key2 = Key(any_scancode, SDL_KMOD_LSHIFT);
    auto const key3 = Key(any_scancode, SDL_KMOD_RSHIFT);

    EXPECT_TRUE(key1.has_shift());
    EXPECT_TRUE(key2.has_shift());
    EXPECT_TRUE(key3.has_shift());
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

    // for this application which shift doesn't matter
    auto const lshift = Key(SDL_SCANCODE_LSHIFT);
    auto const rshift = Key(SDL_SCANCODE_RSHIFT);
    set.insert(lshift);
    EXPECT_TRUE(set.contains(lshift));
    EXPECT_TRUE(set.contains(rshift));
    set.insert(rshift);
    EXPECT_EQ(4, set.size());

    // likewise if the key combo is more than just pressing a shift key
    auto const key4 = Key(yet_another_scancode, SDL_KMOD_LSHIFT);
    auto const key5 = Key(yet_another_scancode, SDL_KMOD_RSHIFT);
    set.insert(key4);
    EXPECT_TRUE(set.contains(key4));
    EXPECT_TRUE(set.contains(key5));
    set.insert(key5);
    EXPECT_EQ(5, set.size());
}
