#pragma once
#include <gtest/gtest.h>
#include <SDL3/SDL.h>

class SDLTest : public ::testing::Test {
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
