#pragma once

#include "key.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

// TODO: refactor to make private
using KeyValue = std::optional<std::pair<uint64_t, std::optional<uint64_t>>>;

class ActiveKeys {
    /**
     * missing from map = key is not monitored
     */
    std::unordered_map<Key, KeyValue, KeyHash> key_timings;
    std::vector<SDL_Scancode> monitored_keys;
    bool is_key_registered(const Key &key) const;

public:
    ActiveKeys();

    /**
     * if specifying a key w/o a modifier: it will also monitor the shift version of the key
     * if a key with a modifier is specified, it will only listen to that exact key
     */
    ActiveKeys(std::initializer_list<Key> keys_to_monitor);
    ActiveKeys(std::initializer_list<std::variant<SDL_Scancode, SDL_Keycode>> keys_to_monitor);
    ActiveKeys(std::initializer_list<SDL_Scancode> scan_codes);

    /**
     * register a key for listening
     * likewise with the ctor: just a key will listen to all modifier versions of it,
     * if it has a modifier then will only listen to the key with that exact modifier
     */
    void start_listen_to_key(Key &&key);
    void start_listen_to_key(const Key &key);
    void start_listen_to_key(SDL_Scancode scan_code);

    /**
     * before first ever key press, monitored keys will have value nullopt
     * after first press and release, the value stored at the key will be whatever the
     * duration of the last key press was until the key is pressed again
     */
    void set_key_pressed(const Key &key);
    void release_key(const Key &key);
    const KeyValue &maybe_get_key(const Key &key) const;

    /**
     * did this keys' press start time occur before start_ms
     */
    bool was_key_pressed_since(const Key &key, uint64_t start_ms) const;
    bool was_key_pressed_since(SDL_Scancode scan_code, uint64_t start_ms) const;

    /**
     * sync the state with SDL_GetKeyboardState
     */
    void sync_key_state();
};
