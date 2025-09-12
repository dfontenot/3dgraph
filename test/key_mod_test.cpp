#include "key_mod.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <gtest/gtest.h>

#include <format>
#include <sstream>
#include <unordered_set>

using std::size_t;

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
    EXPECT_FALSE(shift.has_ctrl());
    EXPECT_FALSE(shift.has_alt());
    EXPECT_TRUE(shift_and_others.has_lshift());
    EXPECT_TRUE(shift_and_others.has_shift());
}

TEST_F(KeyModTest, Ctrl) {
    const KeyMod ctrl{SDL_KMOD_LCTRL};
    const KeyMod ctrl_and_others{SDL_KMOD_LCTRL | SDL_KMOD_LALT};

    EXPECT_TRUE(ctrl.has_lctrl());
    EXPECT_TRUE(ctrl.has_ctrl());
    EXPECT_FALSE(ctrl.has_shift());
    EXPECT_FALSE(ctrl.has_alt());
    EXPECT_TRUE(ctrl_and_others.has_lctrl());
    EXPECT_TRUE(ctrl_and_others.has_ctrl());
}

TEST_F(KeyModTest, Alt) {
    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod alt_and_others{SDL_KMOD_LCTRL | SDL_KMOD_LALT};

    EXPECT_TRUE(alt.has_lalt());
    EXPECT_TRUE(alt.has_alt());
    EXPECT_FALSE(alt.has_shift());
    EXPECT_FALSE(alt.has_ctrl());
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
    const KeyMod either_alt{SDL_KMOD_ALT};
    
    const KeyModEquivalentEqualTo<true> only_shift_considered;
    const KeyModEquivalentEqualTo<false> all_mods_considered;

    EXPECT_TRUE(only_shift_considered(alt, alt_also));
    EXPECT_TRUE(all_mods_considered(alt, alt_also));
    EXPECT_TRUE(only_shift_considered(alt, ctrl));
    EXPECT_FALSE(all_mods_considered(alt, ctrl));
    EXPECT_TRUE(only_shift_considered(alt, r_alt));
    EXPECT_TRUE(all_mods_considered(alt, r_alt));
    EXPECT_TRUE(only_shift_considered(either_alt, alt));
    EXPECT_TRUE(all_mods_considered(either_alt, alt));
    EXPECT_TRUE(only_shift_considered(either_alt, r_alt));
    EXPECT_TRUE(all_mods_considered(either_alt, r_alt));
    EXPECT_TRUE(only_shift_considered(either_alt, ctrl));
    EXPECT_FALSE(all_mods_considered(either_alt, ctrl));
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
    const KeyMod alt{SDL_KMOD_ALT};
    const KeyMod l_alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    const KeyMod l_mixed{SDL_KMOD_LALT | SDL_KMOD_LCTRL};
    const KeyMod r_mixed{SDL_KMOD_RALT | SDL_KMOD_RCTRL};
    const KeyMod mixed{SDL_KMOD_LALT | SDL_KMOD_RALT | SDL_KMOD_LCTRL | SDL_KMOD_RCTRL};

    EXPECT_EQ(alt, alt.as_normalized());
    EXPECT_EQ(alt, l_alt.as_normalized());
    EXPECT_EQ(alt, r_alt.as_normalized());
    EXPECT_NE(l_alt, l_alt.as_normalized());
    EXPECT_NE(r_alt, l_alt.as_normalized());
    EXPECT_NE(l_alt, r_alt.as_normalized());
    EXPECT_NE(l_mixed, l_mixed.as_normalized());
    EXPECT_NE(r_mixed, r_mixed.as_normalized());
    EXPECT_EQ(mixed, r_mixed.as_normalized());
    EXPECT_EQ(mixed, l_mixed.as_normalized());
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

TEST_F(KeyModTest, HashEquivalentOnlyConsiderShift) {
    using std::unordered_set;

    unordered_set<KeyMod, KeyModEquivalentHash<true>, KeyModEquivalentEqualTo<true>> only_shift_set;

    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    only_shift_set.insert(alt);
    only_shift_set.insert(r_alt);

    EXPECT_EQ(1, only_shift_set.size());
    EXPECT_TRUE(only_shift_set.contains(alt));
    EXPECT_TRUE(only_shift_set.contains(r_alt));

    const KeyMod alt_again{SDL_KMOD_LALT};
    const KeyMod ctrl{SDL_KMOD_LCTRL};
    const KeyMod r_ctrl{SDL_KMOD_RCTRL};
    const KeyMod either_ctrl{SDL_KMOD_CTRL};
    only_shift_set.insert(alt_again);
    only_shift_set.insert(ctrl);

    EXPECT_EQ(1, only_shift_set.size());
    EXPECT_TRUE(only_shift_set.contains(r_ctrl));
    EXPECT_TRUE(only_shift_set.contains(either_ctrl));

    only_shift_set.insert(either_ctrl);
    EXPECT_EQ(1, only_shift_set.size());
}

TEST_F(KeyModTest, HashEquivalentConsiderAllMods) {
    using std::unordered_set;

    unordered_set<KeyMod, KeyModEquivalentHash<false>, KeyModEquivalentEqualTo<false>> all_mods_set;

    const KeyMod alt{SDL_KMOD_LALT};
    const KeyMod r_alt{SDL_KMOD_RALT};
    all_mods_set.insert(alt);
    all_mods_set.insert(r_alt);

    EXPECT_EQ(1, all_mods_set.size());
    EXPECT_TRUE(all_mods_set.contains(alt));
    EXPECT_TRUE(all_mods_set.contains(r_alt));

    const KeyMod alt_again{SDL_KMOD_LALT};
    const KeyMod ctrl{SDL_KMOD_LCTRL};
    const KeyMod r_ctrl{SDL_KMOD_RCTRL};
    const KeyMod either_ctrl{SDL_KMOD_CTRL};
    all_mods_set.insert(alt_again);
    all_mods_set.insert(ctrl);

    EXPECT_EQ(2, all_mods_set.size());
    EXPECT_TRUE(all_mods_set.contains(r_ctrl));
    EXPECT_TRUE(all_mods_set.contains(either_ctrl));

    all_mods_set.insert(either_ctrl);
    EXPECT_EQ(2, all_mods_set.size());
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

TEST_F(KeyModTest, WithMoreMods) {
    const KeyMod mod{SDL_KMOD_LALT};

    EXPECT_TRUE(mod.has_alt());
    EXPECT_FALSE(mod.has_shift());

    auto const mod2 = mod.with_more_mods(SDL_KMOD_LSHIFT);
    EXPECT_TRUE(mod2.has_alt());
    EXPECT_TRUE(mod2.has_shift());
}

TEST_F(KeyModTest, AddMods) {
    KeyMod mod{SDL_KMOD_LALT};

    EXPECT_TRUE(mod.has_alt());
    EXPECT_FALSE(mod.has_shift());

    mod.add_mods(SDL_KMOD_LSHIFT);
    EXPECT_TRUE(mod.has_alt());
    EXPECT_TRUE(mod.has_shift());
}

TEST_F(KeyModTest, WithShifted) {
    const KeyMod mod{SDL_KMOD_LALT};

    EXPECT_TRUE(mod.has_alt());
    EXPECT_FALSE(mod.has_shift());

    auto const mod2 = mod.with_shifted();
    EXPECT_TRUE(mod2.has_alt());
    EXPECT_TRUE(mod2.has_lshift());
    EXPECT_TRUE(mod2.has_rshift());
}
