#include "key.hpp"
#include "sdl_test.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <gtest/gtest.h>

#include <format>
#include <optional>
#include <sstream>
#include <unordered_set>
#include <utility>

using std::make_optional;
using std::make_pair;

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

TEST_F(KeyTest, CtorWithKeyCode) {
    auto const e_key_scan_code = Key(SDL_SCANCODE_E);
    auto const e_key_key_code = Key(SDLK_E);
    EXPECT_EQ(e_key_key_code, e_key_scan_code);
}

TEST_F(KeyTest, CtorFromVariant) {
    // choosing key has consistent keyboard placement in qwerty and azerty
    const Keyish scan_code_variant{SDL_SCANCODE_E};
    const Keyish key_code_variant{SDLK_E};
    EXPECT_NE(scan_code_variant, key_code_variant);

    const Key key_from_scan_code{scan_code_variant};
    const Key key_from_key_code{key_code_variant};

    EXPECT_EQ(key_from_scan_code, key_from_key_code);

    // ensure key mods are accounted for
    const Keyish plus_key_code_variant{SDLK_PLUS};
    const Keyish plus_scan_code_variant{make_pair(SDL_SCANCODE_EQUALS, SDL_KMOD_SHIFT)};
    const Key plus_scan_code{SDL_SCANCODE_EQUALS, SDL_KMOD_SHIFT};
    EXPECT_EQ(Key(plus_key_code_variant), plus_scan_code);
    EXPECT_EQ(Key(plus_scan_code_variant), plus_scan_code);
}

TEST_F(KeyTest, Format) {
    const Key any_key{any_scancode};
    auto const formatted = std::format("{}", any_key);
    EXPECT_TRUE(formatted.size() > 0);
}

TEST_F(KeyTest, StreamDisplay) {
    const Key any_key{any_scancode};
    std::stringstream s;
    s << any_key;
    EXPECT_TRUE(s.str().size() > 0);
}

TEST_F(KeyTest, FromKeyCodeHasCorrectMod) {
    const Key from_key_code{SDLK_PLUS};
    const Key from_scan_code{SDL_SCANCODE_EQUALS, SDL_KMOD_SHIFT};

    EXPECT_EQ(from_scan_code, from_key_code);
}

TEST_F(KeyTest, CtorNoKeyCode) {
    const Key from_scancode{SDL_SCANCODE_LSHIFT};
    EXPECT_FALSE(from_scancode.has_key_code());
}

TEST_F(KeyTest, Getters) {
    auto const key = Key(any_scancode, any_keymod);
    auto const expected_keymod = any_keymod;
    auto const expected_scancode = any_scancode;
    auto const expected_keycode = make_optional(any_keycode);

    /*
     * NOTE: these macros appear to be doing something, only when inside of a TEST_F,
     * that the linker doesn't like, appears that can't put anything in the
     * derived test into the assertion macros, have to copy it
     */
    EXPECT_EQ(key.get_key_mod(), expected_keymod);
    EXPECT_EQ(key.get_scan_code(), expected_scancode);
    EXPECT_TRUE(key.has_key_code());
    EXPECT_EQ(key.get_key_code(), expected_keycode);
}

TEST_F(KeyTest, Modifiers) {
    const Key no_mods{any_scancode};
    const Key left_shifted{any_scancode, SDL_KMOD_LSHIFT};
    const Key right_shifted{any_scancode, SDL_KMOD_RSHIFT};
    const Key both_shifted{any_scancode, SDL_KMOD_SHIFT};
    const Key both_ctrl{any_scancode, SDL_KMOD_CTRL};
    const Key both_alt{any_scancode, SDL_KMOD_ALT};
    const Key shift_and_ctrl{any_scancode, SDL_KMOD_SHIFT | SDL_KMOD_CTRL};

    // shift
    EXPECT_FALSE(no_mods.has_shift());
    EXPECT_TRUE(left_shifted.has_shift());
    EXPECT_TRUE(right_shifted.has_shift());
    EXPECT_TRUE(both_shifted.has_shift());
    EXPECT_TRUE(shift_and_ctrl.has_shift());
    EXPECT_FALSE(both_ctrl.has_shift());
    EXPECT_FALSE(both_alt.has_shift());

    // ctrl
    EXPECT_FALSE(no_mods.has_ctrl());
    EXPECT_FALSE(left_shifted.has_ctrl());
    EXPECT_FALSE(right_shifted.has_ctrl());
    EXPECT_FALSE(both_shifted.has_ctrl());
    EXPECT_TRUE(shift_and_ctrl.has_ctrl());
    EXPECT_TRUE(both_ctrl.has_ctrl());
    EXPECT_FALSE(both_alt.has_ctrl());

    // alt
    EXPECT_FALSE(no_mods.has_alt());
    EXPECT_FALSE(left_shifted.has_alt());
    EXPECT_FALSE(right_shifted.has_alt());
    EXPECT_FALSE(both_shifted.has_alt());
    EXPECT_FALSE(shift_and_ctrl.has_alt());
    EXPECT_FALSE(both_ctrl.has_alt());
    EXPECT_TRUE(both_alt.has_alt());
}

TEST_F(KeyTest, CopyShifted) {
    auto const key_no_shift_mod = Key(any_scancode);
    auto const key_no_shift_mod_also = Key(any_scancode, SDL_KMOD_NONE);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);

    EXPECT_EQ(key_no_shift_mod.copy_shifted(), key_shifted);
    EXPECT_EQ(key_no_shift_mod_also.copy_shifted(), key_shifted);
    EXPECT_EQ(key_shifted.copy_shifted(), key_shifted);

    // preserves other modifiers
    const Key key_with_alt{any_scancode, SDL_KMOD_ALT};
    auto const key_with_alt_copied = key_with_alt.copy_shifted(false);
    auto const key_with_alt_copied_shift_only = key_with_alt.copy_shifted(true);
    EXPECT_TRUE(key_with_alt_copied.has_shift());
    EXPECT_TRUE(key_with_alt_copied_shift_only.has_shift());
    EXPECT_TRUE(key_with_alt_copied.has_alt());
    EXPECT_FALSE(key_with_alt_copied_shift_only.has_alt());

    // test scancode that will change keycode on shift
    const Key plus{SDLK_PLUS};
    const Key equals{SDLK_EQUALS};

    EXPECT_EQ(plus.copy_shifted(), plus);
    EXPECT_EQ(equals.copy_shifted(), plus);
}

TEST_F(KeyTest, Eq) {
    const Key key{any_scancode};
    const Key other_key{any_other_scancode};
    const Key key_shifted{any_scancode, SDL_KMOD_SHIFT};
    const Key key_left_shifted{any_scancode, SDL_KMOD_LSHIFT};
    const Key key_right_shifted{any_scancode, SDL_KMOD_RSHIFT};
    const Key key_alt{any_scancode, SDL_KMOD_ALT};
    const Key left_shift_only{SDL_SCANCODE_LSHIFT};
    const Key right_shift_only{SDL_SCANCODE_RSHIFT};

    EXPECT_NE(key, key_shifted);
    EXPECT_NE(key, other_key);
    EXPECT_EQ(key, key);
    EXPECT_NE(key_shifted, key_alt);
    EXPECT_NE(key_left_shifted, key_shifted);
    EXPECT_NE(key_right_shifted, key_shifted);
    EXPECT_NE(key_left_shifted, key_right_shifted);
    EXPECT_NE(left_shift_only, right_shift_only);
}

TEST_F(KeyTest, EqEquivalent) {
    const Key key{any_scancode};
    const Key other_key{any_other_scancode};
    const Key key_shifted{any_scancode, SDL_KMOD_SHIFT};
    const Key key_left_shifted{any_scancode, SDL_KMOD_LSHIFT};
    const Key key_right_shifted{any_scancode, SDL_KMOD_RSHIFT};
    const Key key_alt{any_scancode, SDL_KMOD_ALT};
    const Key left_shift_only{SDL_SCANCODE_LSHIFT};
    const Key right_shift_only{SDL_SCANCODE_RSHIFT};

    EXPECT_FALSE(KeyEquivalentEqualTo{}(key, key_shifted));
    EXPECT_FALSE(KeyEquivalentEqualTo{}(key, other_key));
    EXPECT_TRUE(KeyEquivalentEqualTo{}(key, key));
    EXPECT_FALSE(KeyEquivalentEqualTo{}(key_shifted, key_alt));
    EXPECT_TRUE(KeyEquivalentEqualTo{}(key_left_shifted, key_shifted));
    EXPECT_TRUE(KeyEquivalentEqualTo{}(key_right_shifted, key_shifted));
    EXPECT_TRUE(KeyEquivalentEqualTo{}(key_left_shifted, key_right_shifted));
    EXPECT_TRUE(KeyEquivalentEqualTo{}(left_shift_only, right_shift_only));
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

    // check preferences for also copying other modifiers
    const Key key_with_alt{any_scancode, SDL_KMOD_ALT};
    const Key shifted_key_with_alt{any_scancode, SDL_KMOD_ALT | SDL_KMOD_SHIFT};

    EXPECT_TRUE(key_with_alt.shift_mod_complement(true).has_shift());
    EXPECT_FALSE(key_with_alt.shift_mod_complement(true).has_alt());
    EXPECT_FALSE(shifted_key_with_alt.shift_mod_complement(true).has_shift());
    EXPECT_FALSE(shifted_key_with_alt.shift_mod_complement(true).has_alt());

    EXPECT_TRUE(key_with_alt.shift_mod_complement(false).has_shift());
    EXPECT_TRUE(key_with_alt.shift_mod_complement(false).has_alt());
    EXPECT_FALSE(shifted_key_with_alt.shift_mod_complement(false).has_shift());
    EXPECT_TRUE(shifted_key_with_alt.shift_mod_complement(false).has_alt());
}

TEST_F(KeyTest, HasShfit) {
    const Key key1{any_scancode, SDL_KMOD_SHIFT};
    const Key key2{any_scancode, SDL_KMOD_LSHIFT};
    const Key key3{any_scancode, SDL_KMOD_RSHIFT};
    const Key key4{any_scancode};

    EXPECT_TRUE(key1.has_shift());
    EXPECT_TRUE(key2.has_shift());
    EXPECT_TRUE(key3.has_shift());
    EXPECT_FALSE(key4.has_shift());
}

TEST_F(KeyTest, KeyTypes) {
    const Key a{SDLK_A};
    const Key z{SDLK_Z};
    const Key dollar_sign{SDLK_DOLLAR};
    const Key zero{SDLK_0};
    const Key nine{SDLK_9};

    EXPECT_TRUE(a.is_alpha());
    EXPECT_TRUE(a.copy_shifted().is_alpha());
    EXPECT_TRUE(z.copy_shifted().is_alpha());
    EXPECT_FALSE(zero.is_alpha());
    EXPECT_FALSE(dollar_sign.is_alpha());
    EXPECT_TRUE(zero.is_numeric());
    EXPECT_FALSE(zero.copy_shifted().is_numeric());
    EXPECT_TRUE(nine.is_numeric());
    EXPECT_FALSE(nine.copy_shifted().is_numeric());
    EXPECT_FALSE(dollar_sign.is_numeric());
    EXPECT_TRUE(Key{SDLK_5}.is_alphanum());
    EXPECT_TRUE(Key{SDLK_R}.is_alphanum());
    EXPECT_FALSE(dollar_sign.is_alphanum());
}

TEST_F(KeyTest, Hash) {
    using std::unordered_set;

    unordered_set<Key> set;

    const Key key{any_scancode};
    EXPECT_FALSE(set.contains(key));

    set.insert(key);
    EXPECT_TRUE(set.contains(key));
    EXPECT_EQ(1, set.size());

    set.insert(key);
    EXPECT_TRUE(set.contains(key));
    EXPECT_EQ(1, set.size());

    // ensure keys with different modifiers
    // are differentiated
    const Key key2{any_scancode, any_keymod};
    set.insert(key2);
    EXPECT_TRUE(set.contains(key));
    EXPECT_TRUE(set.contains(key2));
    EXPECT_EQ(2, set.size());

    // ensure can add keys only differing
    // in scancode
    const Key key3{any_other_scancode};
    set.insert(key3);
    EXPECT_TRUE(set.contains(key));
    EXPECT_TRUE(set.contains(key2));
    EXPECT_TRUE(set.contains(key3));
    EXPECT_EQ(3, set.size());

    // assert can add keys that differ only in left vs. right modifier pressed
    const Key key4{any_other_scancode, SDL_KMOD_LCTRL};
    const Key key5{any_other_scancode, SDL_KMOD_RCTRL};
    set.insert(key4);
    EXPECT_TRUE(set.contains(key4));
    EXPECT_FALSE(set.contains(key5));
    EXPECT_EQ(4, set.size());

    set.insert(key5);
    EXPECT_TRUE(set.contains(key4));
    EXPECT_TRUE(set.contains(key5));
    EXPECT_EQ(5, set.size());
}

TEST_F(KeyTest, HashEquivalent) {
    using std::unordered_set;

    unordered_set<Key, KeyEquivalentHash, KeyEquivalentEqualTo> set;

    const Key key{any_scancode};
    EXPECT_FALSE(set.contains(key));

    set.insert(key);
    EXPECT_TRUE(set.contains(key));
    EXPECT_EQ(1, set.size());

    set.insert(key);
    EXPECT_TRUE(set.contains(key));
    EXPECT_EQ(1, set.size());

    // ensure keys with different modifiers
    // are differentiated
    const Key key2{any_scancode, any_keymod};
    set.insert(key2);
    EXPECT_TRUE(set.contains(key));
    EXPECT_TRUE(set.contains(key2));
    EXPECT_EQ(2, set.size());

    // ensure can add keys only differing
    // in scancode
    const Key key3{any_other_scancode};
    set.insert(key3);
    EXPECT_TRUE(set.contains(key));
    EXPECT_TRUE(set.contains(key2));
    EXPECT_TRUE(set.contains(key3));
    EXPECT_EQ(3, set.size());

    // for this application which shift doesn't matter
    const Key lshift{SDL_SCANCODE_LSHIFT};
    const Key rshift{SDL_SCANCODE_RSHIFT};
    set.insert(lshift);
    EXPECT_TRUE(set.contains(lshift));
    EXPECT_TRUE(set.contains(rshift));
    set.insert(rshift);
    EXPECT_EQ(4, set.size());

    // likewise if the key combo is more than just pressing a shift key
    const Key key4{yet_another_scancode, SDL_KMOD_LSHIFT};
    const Key key5{yet_another_scancode, SDL_KMOD_RSHIFT};
    set.insert(key4);
    EXPECT_TRUE(set.contains(key4));
    EXPECT_TRUE(set.contains(key5));
    set.insert(key5);
    EXPECT_EQ(5, set.size());

    // from keycode only hashes consistently
    const Key key6{SDLK_PLUS};
    const Key key7{SDL_SCANCODE_EQUALS, SDL_KMOD_SHIFT};
    set.insert(key6);
    EXPECT_TRUE(set.contains(key6));
    EXPECT_TRUE(set.contains(key7));
    set.insert(key7);
    EXPECT_EQ(6, set.size());
}
