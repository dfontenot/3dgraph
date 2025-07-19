#include "active_keys.hpp"
#include "key.hpp"
#include "sdl_test.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <iterator>
#include <list>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

using std::list;
using std::make_pair;
using std::nullopt;
using std::pair;
using std::size_t;
using std::vector;

class ActiveKeysTest : public SDLTest {
protected:
    static auto const any_scancode = SDL_SCANCODE_D;
    static auto const any_other_scancode = SDL_SCANCODE_T;
    static auto const any_keymod = SDL_KMOD_CTRL;
    static auto const any_keycode = SDLK_PLUS;
};

TEST_F(ActiveKeysTest, Ctors) {
    using std::distance;
    using std::ranges::iota_view;
    using std::views::filter;
    using std::views::transform;

    auto const lowercase_filter = filter([](auto key) { return !key.has_shift(); });

    // initializer list
    const ActiveKeys from_initializer_list{SDL_SCANCODE_D, SDL_SCANCODE_F};
    auto non_shifted = from_initializer_list.get_monitored_keys() | lowercase_filter;
    EXPECT_EQ(2, distance(non_shifted.cbegin(), non_shifted.cend()));

    // from range
    const iota_view key_range{static_cast<size_t>(SDL_SCANCODE_A), static_cast<size_t>(SDL_SCANCODE_D)};
    const ActiveKeys other_keys{key_range | transform([](auto i) { return static_cast<SDL_Scancode>(i); })};
    non_shifted = other_keys.get_monitored_keys() | lowercase_filter;

    EXPECT_EQ(distance(non_shifted.cbegin(), non_shifted.cend()), key_range.size());

    // from container
    list<Keyish> keyishes{SDLK_PLUS, SDL_SCANCODE_A, make_pair(SDL_SCANCODE_D, SDL_KMOD_CTRL)};
    const ActiveKeys from_keyish{keyishes};
    for (auto const keyish : keyishes) {
        const Key key{keyish};
        EXPECT_TRUE(from_keyish.is_key_registered(key));
    }

    // container with duplicates
    list<SDL_Keycode> dupes{SDLK_D, SDLK_A, SDLK_D};
    const ActiveKeys no_dupes{dupes};
    non_shifted = no_dupes.get_monitored_keys() | lowercase_filter;

    EXPECT_EQ(2, distance(non_shifted.cbegin(), non_shifted.cend()));
}

TEST_F(ActiveKeysTest, GetMonitoredKeys) {
    using std::distance;
    using std::views::filter;

    auto const no_shift_filter = filter([](auto key) { return !key.has_shift(); });
    auto const has_shift_filter = filter([](auto key) { return key.has_shift(); });

    const ActiveKeys empty;
    EXPECT_EQ(0, empty.get_monitored_keys().size());

    ActiveKeys monitored{SDLK_E};
    auto monitored_keys = monitored.get_monitored_keys();
    auto no_shift_keys = monitored_keys | no_shift_filter;
    auto shifted_keys = monitored_keys | has_shift_filter;

    EXPECT_EQ(1, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(1, distance(shifted_keys.cbegin(), shifted_keys.cend()));

    monitored.start_listen_to_key(SDLK_E);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;
    EXPECT_EQ(1, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(1, distance(shifted_keys.cbegin(), shifted_keys.cend()));

    monitored.start_listen_to_key(SDLK_F);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;
    EXPECT_EQ(2, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(2, distance(shifted_keys.cbegin(), shifted_keys.cend()));

    monitored.start_listen_to_key(SDLK_0);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;
    EXPECT_EQ(3, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(2, distance(shifted_keys.cbegin(), shifted_keys.cend()));
}

TEST_F(ActiveKeysTest, StartListenToKey) {
    auto const key = Key(any_scancode);
    ActiveKeys active_keys;

    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));

    active_keys.press_key(key);
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));

    active_keys.start_listen_to_key(key);
    active_keys.press_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
}

TEST_F(ActiveKeysTest, SetKeyPressed) {
    auto const key = Key(any_scancode);
    auto const other_key = Key(any_other_scancode);
    ActiveKeys active_keys{key};

    active_keys.press_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));

    active_keys.press_key(other_key);
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(other_key));
}

TEST_F(ActiveKeysTest, MaybeGetKey) {
    ActiveKeys active_keys{any_scancode};
    auto const key = Key(any_scancode);

    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));
    active_keys.press_key(key);

    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    auto key_timing = *active_keys.maybe_get_key(key);
    auto const start_timing = std::get<0>(key_timing);
    EXPECT_EQ(nullopt, std::get<1>(key_timing));

    SDL_Delay(1);

    active_keys.release_key(key);
    key_timing = *active_keys.maybe_get_key(key);
    EXPECT_EQ(start_timing, std::get<0>(key_timing));
    EXPECT_NE(nullopt, std::get<1>(key_timing));
}

TEST_F(ActiveKeysTest, ReleaseKey) {
    ActiveKeys active_keys{any_scancode};
    auto const key = Key(any_scancode);

    // nothing pressed yet
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));

    // frame 1: draining sdl event queue and processing key press event
    active_keys.press_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    auto first_key_press = *active_keys.maybe_get_key(key);
    auto const first_key_press_start_ms = std::get<0>(first_key_press);
    EXPECT_EQ(nullopt, std::get<1>(first_key_press));

    // frame 2: draining sdl event queue and processing key press event
    // key state should be the same
    SDL_Delay(1);
    active_keys.press_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_EQ(std::get<0>(*active_keys.maybe_get_key(key)), first_key_press_start_ms);
    EXPECT_EQ(nullopt, std::get<1>(first_key_press));

    // frame 3: release the key
    SDL_Delay(1);
    active_keys.release_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_EQ(std::get<0>(*active_keys.maybe_get_key(key)), first_key_press_start_ms);
    first_key_press = *active_keys.maybe_get_key(key);
    auto const first_key_ress_end_ms = std::get<1>(first_key_press);
    EXPECT_GT(first_key_ress_end_ms, first_key_press_start_ms);

    // frame 4: press the key again
    SDL_Delay(1);
    active_keys.press_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    auto second_key_press = *active_keys.maybe_get_key(key);
    auto const second_key_press_start_ms = std::get<0>(second_key_press);
    EXPECT_EQ(nullopt, std::get<1>(second_key_press));
    EXPECT_GT(second_key_press_start_ms, first_key_ress_end_ms);
}

TEST_F(ActiveKeysTest, ReleaseKeyTrackModifiers) {
    // hold down key, then also hold down a modifier, then release modifier, then release key
    ActiveKeys active_keys{any_scancode};
    auto const key = Key(any_scancode);
    auto const shifted_key = Key(any_scancode, SDL_KMOD_SHIFT);

    // nothing pressed yet
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(shifted_key));

    // press just the key
    active_keys.press_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(shifted_key));
    auto end_time_key = std::get<1>(*active_keys.maybe_get_key(key));
    EXPECT_EQ(nullopt, end_time_key);

    // also hold down the modifier
    SDL_Delay(1);
    active_keys.press_key(shifted_key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_NE(nullopt, active_keys.maybe_get_key(shifted_key));
    end_time_key = std::get<1>(*active_keys.maybe_get_key(key));
    auto end_time_shifted_key = std::get<1>(*active_keys.maybe_get_key(shifted_key));
    EXPECT_EQ(nullopt, end_time_key);
    EXPECT_EQ(nullopt, end_time_shifted_key);

    // release the modifier
    SDL_Delay(1);
    active_keys.release_key(shifted_key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_NE(nullopt, active_keys.maybe_get_key(shifted_key));
    end_time_key = std::get<1>(*active_keys.maybe_get_key(key));
    end_time_shifted_key = std::get<1>(*active_keys.maybe_get_key(shifted_key));
    EXPECT_EQ(nullopt, end_time_key);
    EXPECT_NE(nullopt, end_time_shifted_key);

    // release the key
    SDL_Delay(1);
    active_keys.release_key(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_NE(nullopt, active_keys.maybe_get_key(shifted_key));
    end_time_key = std::get<1>(*active_keys.maybe_get_key(key));
    end_time_shifted_key = std::get<1>(*active_keys.maybe_get_key(shifted_key));
    EXPECT_NE(nullopt, end_time_key);
    EXPECT_NE(nullopt, end_time_shifted_key);

    EXPECT_GT(*end_time_key, *end_time_shifted_key);
}

TEST_F(ActiveKeysTest, WasKeyPressedSince) {
    ActiveKeys active_keys{any_scancode};
    active_keys.start_listen_to_key(any_keycode);

    const Key any_key{any_scancode};
    const Key any_keycode_key{any_keycode};
    const Key any_other_scancode_key{any_other_scancode};

    EXPECT_FALSE(active_keys.was_key_pressed_since(any_other_scancode_key, 0));

    auto const before_press_ms = SDL_GetTicks();
    SDL_Delay(1);

    active_keys.press_key(any_key);
    active_keys.press_key(any_keycode_key);
    auto const after_press_ms = SDL_GetTicks();

    SDL_Delay(1);

    // check key timing while key is held
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_key, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_key, after_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode_key, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode_key, after_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode, after_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_scancode, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, after_press_ms));

    auto const before_release_key_ms = SDL_GetTicks();
    active_keys.release_key(any_key);
    active_keys.release_key(any_keycode_key);

    SDL_Delay(1);
    auto const after_release_key_ms = SDL_GetTicks();

    // check key timing after released
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_key, after_release_key_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_key, before_release_key_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode_key, after_release_key_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode_key, before_release_key_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_scancode, after_release_key_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, before_release_key_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode, after_release_key_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode, before_release_key_ms));
}
