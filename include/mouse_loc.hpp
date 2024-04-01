#pragma once

#include <cmath>

#include <SDL2/SDL.h>

class MouseLoc {
    int x;
    int y;
public:
    MouseLoc() {
        SDL_GetMouseState(&x, &y);
    }

    MouseLoc(int x, int y) : x(x), y(y) {}

    void update_loc() {
        SDL_GetMouseState(&x, &y);
    }

    double distance(const MouseLoc& other) const {
        return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
    }
};
