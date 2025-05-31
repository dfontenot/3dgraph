#include "active_keys.hpp"
#include "key.hpp"
#include <SDL3/SDL.h>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <utility>

using std::nullopt;

class ActiveKeysTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!SDL_Init(SDL_INIT_EVENTS)) {
            throw std::runtime_error("could not init sdl");
        }
    }

    void TearDown() override {
        SDL_Quit();
    }
};

TEST_F(ActiveKeysTest, StartListenToKey) {
    auto const any_scancode = SDL_SCANCODE_D;
    auto key = Key(any_scancode);
    ActiveKeys active_keys;

    active_keys.set_key_pressed(key);
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(key));

    active_keys.start_listen_to_key(std::move(key));
    EXPECT_NE(nullopt, active_keys.maybe_get_key(Key(any_scancode)));
}

TEST_F(ActiveKeysTest, SetKeyPressed) {
    auto const any_scancode = SDL_SCANCODE_D;
    auto const any_other_scancode = SDL_SCANCODE_T;
    auto const key = Key(any_scancode);
    auto const other_key = Key(any_other_scancode);
    ActiveKeys active_keys{key};

    active_keys.set_key_pressed(key);
    EXPECT_NE(nullopt, active_keys.maybe_get_key(key));

    active_keys.set_key_pressed(other_key);
    EXPECT_EQ(nullopt, active_keys.maybe_get_key(other_key));
}
