#pragma once

#include <SDL3/SDL.h>

#include <bitset>
#include <climits>

struct KeyMod {
    constexpr operator SDL_Keymod() const {
        return static_cast<SDL_Keymod>(val.to_ulong());
    }

    constexpr KeyMod() : val(SDL_KMOD_NONE) {
    }

    constexpr explicit KeyMod(SDL_Keymod mask) : val(mask) {
    }

    [[nodiscard]] constexpr bool test(std::size_t bit) const {
        return val.test(bit);
    }

    constexpr void set(std::size_t bit, bool bit_val = true) {
        val[bit] = bit_val;
    }

    [[nodiscard]] constexpr bool has_lshift() const {
        return val.test(SDL_KMOD_LSHIFT);
    }

    [[nodiscard]] constexpr bool has_rshift() const {
        return val.test(SDL_KMOD_RSHIFT);
    }

    [[nodiscard]] constexpr bool has_shift() const {
        return has_lshift() || has_rshift();
    }

private:
    std::bitset<CHAR_BIT * sizeof(SDL_Keymod)> val;
};
