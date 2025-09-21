#include "active_keys.hpp"
#include "key.hpp"
#include "sdl_test.hpp"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <iterator>
#include <list>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>
#include <version>

#if !__cpp_lib_ranges_as_const
#include <range/v3/all.hpp>
#endif

using std::list;
using std::make_pair;
using std::nullopt;
using std::pair;
using std::size_t;
using std::vector;

class ActiveKeysTest : public SDLTest {
protected:
    constexpr static auto const any_scancode = SDL_SCANCODE_D;
    constexpr static auto const any_other_scancode = SDL_SCANCODE_T;
    constexpr static auto const any_keymod = SDL_KMOD_CTRL;
    constexpr static auto const any_shift_scan_code = SDL_SCANCODE_LSHIFT;
    constexpr static const Key any_shift_key{any_shift_scan_code, SDLK_LSHIFT, SDL_KMOD_SHIFT};

    /** any keycode that does not translate to an unmodded scan code */
    constexpr static auto const any_keycode = SDLK_PLUS;
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

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(2, distance(non_shifted.begin(), non_shifted.end()));
#else
    EXPECT_EQ(2, distance(non_shifted.cbegin(), non_shifted.cend()));
#endif
}

TEST_F(ActiveKeysTest, CtorFromContainers) {
    // currently, via ranges is how this is supported so feature gate it
#if __cpp_lib_containers_ranges
    using std::distance;
    using std::ranges::iota_view;
    using std::views::filter;
    using std::views::transform;

    auto const lowercase_filter = filter([](auto key) { return !key.has_shift(); });

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
    auto non_shifted = no_dupes.get_monitored_keys() | lowercase_filter;

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(2, distance(non_shifted.begin(), non_shifted.end()));
#else
    EXPECT_EQ(2, distance(non_shifted.cbegin(), non_shifted.cend()));
#endif
#else
    GTEST_SKIP() << "initializing ActiveKeys from STL container not supported by this STL";
#endif
}

TEST_F(ActiveKeysTest, CtorFromRange) {
#if __cpp_lib_containers_ranges
    using std::distance;
    using std::ranges::iota_view;
    using std::views::filter;
    using std::views::transform;

    auto const lowercase_filter = filter([](auto key) { return !key.has_shift(); });

    // from range
    const iota_view key_range{static_cast<size_t>(SDL_SCANCODE_A), static_cast<size_t>(SDL_SCANCODE_D)};
    const ActiveKeys other_keys{key_range | transform([](auto i) { return static_cast<SDL_Scancode>(i); })};
    auto non_shifted = other_keys.get_monitored_keys() | lowercase_filter;

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(distance(non_shifted.begin(), non_shifted.end()), key_range.size());
#else
    EXPECT_EQ(distance(non_shifted.cbegin(), non_shifted.cend()), key_range.size());
#endif
#else
    GTEST_SKIP() << "initializing ActiveKeys from range not supported by this STL";
#endif
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

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(1, distance(no_shift_keys.begin(), no_shift_keys.end()));
    EXPECT_EQ(1, distance(shifted_keys.begin(), shifted_keys.end()));
#else
    EXPECT_EQ(1, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(1, distance(shifted_keys.cbegin(), shifted_keys.cend()));
#endif

    monitored.start_listen_to_key(SDLK_E);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(1, distance(no_shift_keys.begin(), no_shift_keys.end()));
    EXPECT_EQ(1, distance(shifted_keys.begin(), shifted_keys.end()));
#else
    EXPECT_EQ(1, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(1, distance(shifted_keys.cbegin(), shifted_keys.cend()));
#endif

    monitored.start_listen_to_key(SDLK_F);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(2, distance(no_shift_keys.begin(), no_shift_keys.end()));
    EXPECT_EQ(2, distance(shifted_keys.begin(), shifted_keys.end()));
#else
    EXPECT_EQ(2, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(2, distance(shifted_keys.cbegin(), shifted_keys.cend()));
#endif

    monitored.start_listen_to_key(SDLK_0);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(3, distance(no_shift_keys.begin(), no_shift_keys.end()));
    EXPECT_EQ(2, distance(shifted_keys.begin(), shifted_keys.end()));
#else
    EXPECT_EQ(3, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(2, distance(shifted_keys.cbegin(), shifted_keys.cend()));
#endif

    monitored.start_listen_to_key(SDLK_EQUALS);
    monitored_keys = monitored.get_monitored_keys();
    no_shift_keys = monitored_keys | no_shift_filter;
    shifted_keys = monitored_keys | has_shift_filter;

#if !__cpp_lib_ranges_as_const
    EXPECT_EQ(4, distance(no_shift_keys.begin(), no_shift_keys.end()));
    EXPECT_EQ(2, distance(shifted_keys.begin(), shifted_keys.end()));
#else
    EXPECT_EQ(4, distance(no_shift_keys.cbegin(), no_shift_keys.cend()));
    EXPECT_EQ(2, distance(shifted_keys.cbegin(), shifted_keys.cend()));
#endif
}

TEST_F(ActiveKeysTest, StartListenToKey) {
    const Key key{any_scancode};
    ActiveKeys active_keys;

    auto const start_ms = SDL_GetTicks();
    EXPECT_FALSE(active_keys.was_key_pressed_since(key, start_ms));

    active_keys.press_key(key);
    EXPECT_FALSE(active_keys.was_key_pressed_since(key, start_ms));

    active_keys.start_listen_to_key(key);
    active_keys.press_key(key);
    EXPECT_TRUE(active_keys.was_key_pressed_since(key, start_ms));
}

TEST_F(ActiveKeysTest, PressKey) {
    const Key key{any_scancode};
    const Key other_key{any_other_scancode};
    ActiveKeys active_keys{key};

    active_keys.press_key(other_key);
    auto const start_ms = SDL_GetTicks();
    EXPECT_FALSE(active_keys.was_key_pressed_since(other_key, start_ms));

    auto const before_press_ms = SDL_GetTicks();
    SDL_Delay(1);
    active_keys.press_key(key);
    SDL_Delay(1);
    auto const after_press_ms = SDL_GetTicks();

    auto maybe_key_timing_res = active_keys.get(key);
    EXPECT_TRUE(maybe_key_timing_res);
    auto maybe_key_timing = *maybe_key_timing_res;
    EXPECT_NE(nullopt, maybe_key_timing);
    auto key_timing = *maybe_key_timing;
    auto const start_time = std::get<0>(key_timing);
    EXPECT_TRUE(start_time > before_press_ms);
    EXPECT_TRUE(start_time < after_press_ms);
    EXPECT_EQ(nullopt, std::get<1>(key_timing));

    SDL_Delay(1);
    active_keys.press_key(key);
    maybe_key_timing_res = active_keys.get(key);
    EXPECT_TRUE(maybe_key_timing_res);
    maybe_key_timing = *maybe_key_timing_res;
    EXPECT_NE(nullopt, maybe_key_timing_res);
    key_timing = *maybe_key_timing;
    EXPECT_EQ(std::get<0>(key_timing), start_time);
    EXPECT_EQ(nullopt, std::get<1>(key_timing));
}

TEST_F(ActiveKeysTest, ReleaseKey) {
    ActiveKeys active_keys{any_scancode};
    const Key key{any_scancode};

    // nothing pressed yet
    EXPECT_EQ(nullopt, active_keys.get(key));

    // frame 1: draining sdl event queue and processing key press event
    active_keys.press_key(key);
    auto first_key_press_res = active_keys.get(key);
    EXPECT_TRUE(first_key_press_res);
    auto maybe_first_key_press = *first_key_press_res;
    EXPECT_NE(nullopt, maybe_first_key_press);
    auto first_key_press = *maybe_first_key_press;
    auto const first_key_press_start_ms = std::get<0>(first_key_press);
    EXPECT_EQ(nullopt, std::get<1>(first_key_press));

    // frame 2: draining sdl event queue and processing key press event
    // key state should be the same
    SDL_Delay(1);
    active_keys.press_key(key);
    first_key_press_res = active_keys.get(key);
    EXPECT_TRUE(first_key_press_res);
    maybe_first_key_press = *first_key_press_res;
    EXPECT_NE(nullopt, maybe_first_key_press);
    first_key_press = *maybe_first_key_press;
    EXPECT_EQ(std::get<0>(first_key_press), first_key_press_start_ms);
    EXPECT_EQ(nullopt, std::get<1>(first_key_press));

    // frame 3: release the key
    SDL_Delay(1);
    active_keys.release_key(key);
    first_key_press_res = active_keys.get(key);
    EXPECT_TRUE(first_key_press_res);
    maybe_first_key_press = *first_key_press_res;
    EXPECT_NE(nullopt, maybe_first_key_press);
    first_key_press = *maybe_first_key_press;
    EXPECT_EQ(std::get<0>(first_key_press), first_key_press_start_ms);
    EXPECT_NE(nullopt, std::get<1>(first_key_press));
    auto const first_key_ress_end_ms = *std::get<1>(first_key_press);
    EXPECT_GT(first_key_ress_end_ms, first_key_press_start_ms);

    // frame 4: press the key again
    SDL_Delay(1);
    active_keys.press_key(key);
    auto const second_key_press_res = active_keys.get(key);
    EXPECT_TRUE(second_key_press_res);
    auto const maybe_second_key_press = *second_key_press_res;
    EXPECT_NE(nullopt, maybe_second_key_press);
    auto second_key_press = *maybe_second_key_press;
    auto const second_key_press_start_ms = std::get<0>(second_key_press);
    EXPECT_EQ(nullopt, std::get<1>(second_key_press));
    EXPECT_GT(second_key_press_start_ms, first_key_ress_end_ms);
}

TEST_F(ActiveKeysTest, ReleaseKeyTrackModifiers) {
    // hold down key, then also hold down a modifier, then release modifier, then release key
    ActiveKeys active_keys{any_scancode};
    const Key key{any_scancode};
    const Key shifted_key{any_scancode, SDL_KMOD_SHIFT};

    // nothing pressed yet
    auto maybe_key_timing_res = active_keys.get(key);
    auto maybe_shifted_key_timing_res = active_keys.get(shifted_key);
    EXPECT_TRUE(maybe_key_timing_res);
    EXPECT_TRUE(maybe_shifted_key_timing_res);
    auto maybe_key_timing = *maybe_key_timing_res;
    auto maybe_shifted_key_timing = *maybe_shifted_key_timing_res;
    EXPECT_EQ(nullopt, active_keys.get(key));
    EXPECT_EQ(nullopt, active_keys.get(shifted_key));

    // press just the key
    active_keys.press_key(key);
    maybe_key_timing_res = active_keys.get(key);
    maybe_shifted_key_timing_res = active_keys.get(shifted_key);
    EXPECT_TRUE(maybe_key_timing_res);
    EXPECT_TRUE(maybe_shifted_key_timing_res);
    maybe_key_timing = *maybe_key_timing_res;
    maybe_shifted_key_timing = *maybe_shifted_key_timing_res;
    EXPECT_NE(nullopt, maybe_key_timing);
    EXPECT_EQ(nullopt, maybe_shifted_key_timing);
    auto end_time_key = std::get<1>(*maybe_key_timing);
    EXPECT_EQ(nullopt, end_time_key);

    // also hold down the modifier
    SDL_Delay(1);
    active_keys.press_key(any_shift_key);
    active_keys.press_key(shifted_key);
    maybe_key_timing_res = active_keys.get(key);
    maybe_shifted_key_timing_res = active_keys.get(shifted_key);
    EXPECT_TRUE(maybe_key_timing_res);
    EXPECT_TRUE(maybe_shifted_key_timing_res);
    maybe_key_timing = *maybe_key_timing_res;
    maybe_shifted_key_timing = *maybe_shifted_key_timing_res;
    EXPECT_NE(nullopt, maybe_key_timing);
    EXPECT_NE(nullopt, maybe_shifted_key_timing);
    end_time_key = std::get<1>(*maybe_key_timing);
    auto end_time_shifted_key = std::get<1>(*maybe_shifted_key_timing);
    EXPECT_EQ(nullopt, end_time_key);
    EXPECT_EQ(nullopt, end_time_shifted_key);

    // release the modifier
    SDL_Delay(1);
    active_keys.release_key(any_shift_key);
    active_keys.press_key(key);

    maybe_key_timing_res = active_keys.get(key);
    maybe_shifted_key_timing_res = active_keys.get(shifted_key);
    EXPECT_TRUE(maybe_key_timing_res);
    EXPECT_TRUE(maybe_shifted_key_timing_res);
    maybe_key_timing = *maybe_key_timing_res;
    maybe_shifted_key_timing = *maybe_shifted_key_timing_res;
    EXPECT_NE(nullopt, maybe_key_timing);
    EXPECT_NE(nullopt, maybe_shifted_key_timing);
    end_time_key = std::get<1>(*maybe_key_timing);
    end_time_shifted_key = std::get<1>(*maybe_shifted_key_timing);
    EXPECT_EQ(nullopt, end_time_key);
    EXPECT_NE(nullopt, end_time_shifted_key);

    // release the key
    SDL_Delay(1);
    active_keys.release_key(key);
    maybe_key_timing_res = active_keys.get(key);
    maybe_shifted_key_timing_res = active_keys.get(shifted_key);
    EXPECT_TRUE(maybe_key_timing_res);
    EXPECT_TRUE(maybe_shifted_key_timing_res);
    maybe_key_timing = *maybe_key_timing_res;
    maybe_shifted_key_timing = *maybe_shifted_key_timing_res;
    EXPECT_NE(nullopt, maybe_key_timing);
    EXPECT_NE(nullopt, maybe_shifted_key_timing);
    end_time_key = std::get<1>(*maybe_key_timing);
    end_time_shifted_key = std::get<1>(*maybe_shifted_key_timing);
    EXPECT_NE(nullopt, end_time_key);
    EXPECT_NE(nullopt, end_time_shifted_key);

    EXPECT_GT(*end_time_key, *end_time_shifted_key);
}

TEST_F(ActiveKeysTest, ReleaseKeyHandleUnmonitored) {
    const Key plus_key{SDLK_PLUS};
    const Key equals_key{SDLK_EQUALS};
    ActiveKeys active_keys{plus_key};

    // pre-condition, key layout must match test case
    EXPECT_EQ(equals_key, plus_key.without_shift());

    // nothing pressed yet
    auto maybe_plus_key_timing_res = active_keys.get(plus_key);
    EXPECT_TRUE(maybe_plus_key_timing_res);
    EXPECT_EQ(nullopt, *maybe_plus_key_timing_res);

    // press the monitored key
    active_keys.press_key(any_shift_key);
    active_keys.press_key(plus_key);

    maybe_plus_key_timing_res = active_keys.get(plus_key);
    EXPECT_TRUE(maybe_plus_key_timing_res);
    auto maybe_plus_key_timing = *maybe_plus_key_timing_res;
    EXPECT_NE(nullopt, maybe_plus_key_timing);
    auto end_time_key = std::get<1>(*maybe_plus_key_timing);
    EXPECT_EQ(nullopt, end_time_key);

    // release the modifier
    active_keys.release_key(any_shift_key);
    active_keys.press_key(equals_key);

    EXPECT_FALSE(active_keys.get(equals_key));
    maybe_plus_key_timing_res = active_keys.get(plus_key);
    EXPECT_TRUE(maybe_plus_key_timing_res);
    maybe_plus_key_timing = *maybe_plus_key_timing_res;
    EXPECT_NE(nullopt, maybe_plus_key_timing);
    end_time_key = std::get<1>(*maybe_plus_key_timing);
    EXPECT_NE(nullopt, end_time_key);

    // release the unmonitored key
    SDL_Delay(1);
    active_keys.release_key(equals_key);

    maybe_plus_key_timing_res = active_keys.get(plus_key);
    EXPECT_TRUE(maybe_plus_key_timing_res);
    maybe_plus_key_timing = *maybe_plus_key_timing_res;
    EXPECT_NE(nullopt, maybe_plus_key_timing);
    end_time_key = std::get<1>(*maybe_plus_key_timing);
    EXPECT_NE(nullopt, end_time_key);
}

TEST_F(ActiveKeysTest, WasKeyPressedSince) {
    ActiveKeys active_keys{any_scancode};
    active_keys.start_listen_to_key(any_keycode);

    const Key any_key{any_scancode};
    const Key any_keycode_key{any_keycode};
    const Key any_other_scancode_key{any_other_scancode};

    EXPECT_FALSE(active_keys.was_key_pressed_since(any_other_scancode_key, SDL_GetTicks()));

    auto const before_press_ms = SDL_GetTicks();
    SDL_Delay(1);

    active_keys.press_key(any_key);
    active_keys.press_key(any_keycode_key);

    SDL_Delay(1);
    auto const after_press_ms = SDL_GetTicks();

    // check key timing while key is held
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_key, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_key, after_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode_key, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode_key, after_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode, after_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_scancode, before_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, after_press_ms));

    // register as pressed again (happens for held keys e.g., not holding just a modifier)
    auto const before_second_press_ms = SDL_GetTicks();
    SDL_Delay(1);

    active_keys.press_key(any_key);
    active_keys.press_key(any_keycode_key);

    SDL_Delay(1);
    auto const after_second_press_ms = SDL_GetTicks();

    // check key timing while key is held and has shown up in the event queue more than once
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_key, before_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_key, after_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode_key, before_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode_key, after_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode, before_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode, after_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, before_second_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, after_second_press_ms));

    auto const before_release_key_ms = SDL_GetTicks();
    SDL_Delay(1);

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

    // press the key again
    auto const before_third_press_ms = SDL_GetTicks();
    SDL_Delay(1);

    active_keys.press_key(any_key);
    active_keys.press_key(any_keycode_key);

    SDL_Delay(1);
    auto const after_third_press_ms = SDL_GetTicks();

    EXPECT_FALSE(active_keys.was_key_pressed_since(any_key, before_third_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_key, after_third_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode_key, before_third_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode_key, after_third_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_keycode, before_third_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_keycode, after_third_press_ms));
    EXPECT_FALSE(active_keys.was_key_pressed_since(any_scancode, before_third_press_ms));
    EXPECT_TRUE(active_keys.was_key_pressed_since(any_scancode, after_third_press_ms));
}

TEST_F(ActiveKeysTest, WhichKeyVariantWasPressedSinceNonAlpha) {
    const Key plus{SDLK_PLUS};
    ActiveKeys active{plus};

    auto const start_ms = SDL_GetTicks();
    SDL_Delay(1);

    EXPECT_EQ(nullopt, active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), plus));

    active.press_key(any_shift_key);
    active.press_key(plus);
    auto const query_result = active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), plus);

    EXPECT_NE(nullopt, query_result);
    auto const queried_key = std::get<0>(*query_result);

    EXPECT_TRUE(queried_key.has_shift());
    EXPECT_TRUE(queried_key.has_shift());
    EXPECT_FALSE(queried_key.has_ctrl());

    active.release_key(any_shift_key);
    active.release_key(plus);
    SDL_Delay(1);
    auto const after_release_ms = SDL_GetTicks();

    EXPECT_EQ(nullopt, active.which_key_variant_was_pressed_since(after_release_ms, SDL_GetTicks(), plus));
    EXPECT_EQ(nullopt,
              active.which_key_variant_was_pressed_since(after_release_ms, SDL_GetTicks(), SDL_SCANCODE_EQUALS));
}

// TODO: for the key queries need to validate the returned start and end times too
TEST_F(ActiveKeysTest, WhichKeyVariantWasPressedSince) {
    const Key any_key{any_scancode};
    const Key any_shifted_key{any_scancode, KeyMod::shift()};
    const Key any_unregistered_key{any_other_scancode};
    ActiveKeys active{any_key, any_shifted_key};

    auto const start_ms = SDL_GetTicks();
    SDL_Delay(1);

    EXPECT_EQ(nullopt, active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), any_key));
    EXPECT_EQ(nullopt, active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), any_shifted_key));
    EXPECT_EQ(nullopt, active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), any_unregistered_key));

    // press only the key
    active.press_key(any_key);
    auto query_result = active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), any_key);

    EXPECT_NE(nullopt, query_result);
    auto queried_key = std::get<0>(*query_result);

    EXPECT_FALSE(queried_key.has_shift());
    EXPECT_FALSE(queried_key.has_alt());
    EXPECT_FALSE(queried_key.has_ctrl());
    EXPECT_EQ(nullopt, active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), any_unregistered_key));

    // press the key with shift
    auto const before_press_shift_ms = SDL_GetTicks();
    SDL_Delay(1);

    active.press_key(any_shift_key);
    active.press_key(any_shifted_key);
    query_result = active.which_key_variant_was_pressed_since(before_press_shift_ms, SDL_GetTicks(), any_key);

    EXPECT_NE(nullopt, query_result);
    queried_key = std::get<0>(*query_result);

    EXPECT_TRUE(queried_key.has_shift());
    EXPECT_FALSE(queried_key.has_alt());
    EXPECT_FALSE(queried_key.has_ctrl());
    EXPECT_EQ(nullopt,
              active.which_key_variant_was_pressed_since(before_press_shift_ms, SDL_GetTicks(), any_unregistered_key));

    // mods on the key in the query are ignored
    query_result = active.which_key_variant_was_pressed_since(start_ms, SDL_GetTicks(), any_key);

    EXPECT_NE(nullopt, query_result);
    queried_key = std::get<0>(*query_result);

    EXPECT_TRUE(queried_key.has_shift());
    EXPECT_FALSE(queried_key.has_alt());
    EXPECT_FALSE(queried_key.has_ctrl());

    // release shift key
    active.release_key(any_shift_key);
    active.press_key(any_key);

    SDL_Delay(1);
    auto const after_release_shift_key_ms = SDL_GetTicks();

    query_result = active.which_key_variant_was_pressed_since(after_release_shift_key_ms, SDL_GetTicks(), any_key);

    EXPECT_NE(nullopt, query_result);
    queried_key = std::get<0>(*query_result);
    EXPECT_FALSE(queried_key.has_shift());

    // release key
    active.release_key(any_key);

    SDL_Delay(1);
    auto const after_release_key_ms = SDL_GetTicks();

    query_result = active.which_key_variant_was_pressed_since(after_release_key_ms, SDL_GetTicks(), any_key);
    EXPECT_EQ(nullopt, query_result);
}
