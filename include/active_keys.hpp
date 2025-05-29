#pragma once

#include "key.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <unordered_map>
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
     * NOTE: all keys listed here and their shift modifiers will be monitored
     * TODO: functionality to just monitor the exact keys given?
     */
    ActiveKeys(std::initializer_list<Key> keys_to_monitor);
    ActiveKeys(std::initializer_list<SDL_Scancode> scan_codes);

    /**
     * register a key for listening
     */
    void start_listen_to_key(Key &&key);
    void set_key_pressed(const Key &key);
    void release_key(const Key &key);
    const KeyValue &maybe_get_key(const Key &key) const;
    bool was_key_pressed_during_time_range(const Key &key, uint64_t start_ms) const;
    bool was_key_pressed_during_time_range(SDL_Scancode scan_code, uint64_t start_ms) const;

    /**
     * sync the state with SDL_GetKeyboardState
     */
    void sync_key_state();
};
