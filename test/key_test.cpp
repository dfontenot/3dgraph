#include "key.hpp"
#include <SDL3/SDL_keycode.h>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include <unordered_set>

TEST(Key, CtorScancodeOnly) {
    auto const key = Key(SDL_SCANCODE_D);
    EXPECT_FALSE(key.has_modifier());
}

TEST(Key, CtorWithMod) {
    auto const key = Key(SDL_SCANCODE_D, SDL_KMOD_ALT);
    EXPECT_TRUE(key.has_modifier());
    EXPECT_FALSE(key.has_shift());
}

TEST(Key, Getters) {
    auto const any_mod = SDL_KMOD_ALT;
    auto const any_scancode = SDL_SCANCODE_E;
    auto const key = Key(any_scancode, any_mod);
    EXPECT_EQ(key.get_key_mod(), any_mod);
    EXPECT_EQ(key.get_scan_code(), any_scancode);
}

TEST(Key, CopyShifted) {
    auto const any_scancode = SDL_SCANCODE_E;
    auto const key_no_shift_mod = Key(any_scancode);
    auto const key_no_shift_mod_also = Key(any_scancode, SDL_KMOD_NONE);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);

    EXPECT_EQ(key_no_shift_mod.copy_shifted(), key_shifted);
    EXPECT_EQ(key_no_shift_mod_also.copy_shifted(), key_shifted);
    EXPECT_EQ(key_shifted.copy_shifted(), key_shifted);
}

TEST(Key, Eq) {
    auto const any_scancode = SDL_SCANCODE_E;
    auto const any_other_scancode = SDL_SCANCODE_T;
    auto const key = Key(any_scancode);
    auto const other_key = Key(any_other_scancode);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);
    auto const key_alt = Key(any_scancode, SDL_KMOD_ALT);

    EXPECT_FALSE(key == key_shifted);
    EXPECT_FALSE(key == other_key);
    EXPECT_TRUE(key == key);
    EXPECT_FALSE(key_shifted == key_alt);
}

TEST(Key, WithoutMods) {
    auto const any_scancode = SDL_SCANCODE_E;
    auto const key = Key(any_scancode);
    auto const key_alt = Key(any_scancode, SDL_KMOD_ALT);

    EXPECT_EQ(key_alt.without_mods(), key);
    EXPECT_EQ(key.without_mods(), key);
}

TEST(Key, ShiftModComplement) {
    auto const any_scancode = SDL_SCANCODE_E;
    auto const key = Key(any_scancode);
    auto const key_shifted = Key(any_scancode, SDL_KMOD_SHIFT);

    EXPECT_EQ(key, key_shifted.shift_mod_complement());
    EXPECT_EQ(key_shifted, key.shift_mod_complement());
}

TEST(Key, Hash) {
    using std::unordered_set;

    unordered_set<Key, KeyHash> set;

    auto const any_scancode = SDL_SCANCODE_E;
    auto const any_other_scancode = SDL_SCANCODE_D;
    auto const any_mod = SDL_KMOD_ALT;
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
    auto const key2 = Key(any_scancode, any_mod);
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
