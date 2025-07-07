#include "active_keys.hpp"
#include "key.hpp"
#include "sdl_test.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

using std::nullopt;
using std::size_t;
using std::vector;

class ActiveKeysTest : public SDLTest {
protected:
    static auto const any_scancode = SDL_SCANCODE_D;
    static auto const any_other_scancode = SDL_SCANCODE_T;
    static auto const any_keymod = SDL_KMOD_CTRL;
};

TEST_F(ActiveKeysTest, Ctors) {
    using std::ranges::iota_view;

    vector<SDL_Scancode> codes{SDL_SCANCODE_D, SDL_SCANCODE_F};
    auto const keycodes_count = codes.size();
    const ActiveKeys keys{std::move(codes)};

    EXPECT_EQ(keycodes_count, keys.num_keys_monitored());

    const iota_view key_range{static_cast<size_t>(SDL_SCANCODE_A), static_cast<size_t>(SDL_SCANCODE_D)};
    const ActiveKeys other_keys{key_range | std::views::transform([](auto i) { return static_cast<SDL_Scancode>(i); })};

    EXPECT_EQ(std::distance(key_range.cbegin(), key_range.cend()), other_keys.num_keys_monitored());
}

TEST_F(ActiveKeysTest, StartListenToKey) {
    auto const key = Key(any_scancode);
    ActiveKeys active_keys;

    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));

    active_keys.set_key_pressed(key);
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));

    active_keys.start_listen_to_key(key);
    active_keys.set_key_pressed(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
}

TEST_F(ActiveKeysTest, SetKeyPressed) {
    auto const key = Key(any_scancode);
    auto const other_key = Key(any_other_scancode);
    ActiveKeys active_keys{key};

    active_keys.set_key_pressed(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));

    active_keys.set_key_pressed(other_key);
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(other_key));
}

TEST_F(ActiveKeysTest, MaybeGetKey) {
    ActiveKeys active_keys{any_scancode};
    auto const key = Key(any_scancode);

    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));
    active_keys.set_key_pressed(key);

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
    active_keys.set_key_pressed(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    auto first_key_press = *active_keys.maybe_get_key(key);
    auto const first_key_press_start_ms = std::get<0>(first_key_press);
    EXPECT_EQ(nullopt, std::get<1>(first_key_press));

    // frame 2: draining sdl event queue and processing key press event
    // key state should be the same
    SDL_Delay(1);
    active_keys.set_key_pressed(key);
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
    active_keys.set_key_pressed(key);
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
    active_keys.set_key_pressed(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(shifted_key));
    auto end_time_key = std::get<1>(*active_keys.maybe_get_key(key));
    EXPECT_EQ(nullopt, end_time_key);

    // also hold down the modifier
    SDL_Delay(1);
    active_keys.set_key_pressed(shifted_key);
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
    auto const key = Key(any_scancode);

    EXPECT_FALSE(active_keys.was_key_pressed_since(Key(any_other_scancode), 0));

    SDL_Delay(1);

    auto const before_press_ms = SDL_GetTicks();
    active_keys.set_key_pressed(key);

    SDL_Delay(1);

    EXPECT_TRUE(active_keys.was_key_pressed_since(key, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(key, SDL_GetTicks()));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, before_press_ms));
}
