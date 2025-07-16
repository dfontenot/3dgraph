#pragma once

#include "key.hpp"
#include <SDL3/SDL.h>

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

// TODO: refactor to make private
using Interval = std::pair<uint64_t, std::optional<uint64_t>>;
using KeyValue = std::optional<Interval>;

class ActiveKeys {
    /**
     * missing from map = key is not monitored
     */
    std::unordered_map<Key, KeyValue, KeyHash> key_timings;
    std::vector<SDL_Scancode> monitored_keys;

    bool is_key_registered(const Key &key) const;

public:
    ActiveKeys() = default;

    /**
     * monitor these scancodes for key presses
     */
    explicit ActiveKeys(std::vector<SDL_Scancode> &&keys_to_monitor);

    /**
     * if specifying a key w/o a modifier: it will also monitor the shift version of the key
     * if a key with a modifier is specified, it will only listen to that exact key
     */
    explicit ActiveKeys(std::initializer_list<Key> keys_to_monitor);
    explicit ActiveKeys(std::initializer_list<std::variant<SDL_Scancode, SDL_Keycode>> keys_to_monitor);
    explicit ActiveKeys(std::initializer_list<SDL_Scancode> scan_codes);

    /**
     * if specifying a key w/o a modifier: it will also monitor the shift version of the key
     * if a key with a modifier is specified, it will only listen to that exact key
     */
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, Key>
    explicit ActiveKeys(R &&keys_to_monitor)
        : key_timings(std::from_range, std::forward<R>(keys_to_monitor) | std::views::transform([](auto key) {
                                           return std::make_pair(key, std::nullopt);
                                       })),
          monitored_keys(std::from_range, std::forward<R>(keys_to_monitor) |
                                              std::views::transform([](auto key) { return key.get_scan_code(); })) {
        for (auto const key : std::forward<R>(keys_to_monitor)) {
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), std::nullopt});
        }
    }

    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, SDL_Scancode>
    explicit ActiveKeys(R &&keys_to_monitor)
        // clang-format off
        : key_timings(std::from_range, std::forward<R>(keys_to_monitor) |
                      std::views::transform([](auto scan_code) {
                          const Key key{scan_code};
                          return std::make_pair(key, std::nullopt);
                      })),
          // clang-format on
          monitored_keys(std::forward<R>(keys_to_monitor).cbegin(), std::forward<R>(keys_to_monitor).cend()) {
        for (auto const key_code : std::forward<R>(keys_to_monitor)) {
            const Key key{key_code};
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), std::nullopt});
        }
    }

    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, Keyish>
    explicit ActiveKeys(R &&keys_to_monitor)
        // clang-format off
        : key_timings(std::from_range, std::forward<R>(keys_to_monitor) |
                      std::views::transform([](auto keyish) {
                          const Key key{keyish};
                          return std::make_pair(key, std::nullopt);
                      })) {
        // clang-format on
        for (auto const keyish : std::forward<R>(keys_to_monitor)) {
            const Key key{keyish};
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), std::nullopt});
            monitored_keys.push_back(key.get_scan_code());
        }
    }

    /**
     * register a key for listening
     * likewise with the ctor: just a key will listen to all modifier versions of it,
     * if it has a modifier then will only listen to the key with that exact modifier
     */
    void start_listen_to_key(const Key &key);
    void start_listen_to_key(SDL_Scancode scan_code);
    void start_listen_to_key(SDL_Keycode key_code);

    /**
     * before first ever key press, monitored keys will have value nullopt
     * after first press and release, the value stored at the key will be whatever the
     * duration of the last key press was until the key is pressed again
     */
    void press_key(const Key &key);
    void release_key(const Key &key);

    /** get the key's timing records, nullopt if unmonitored */
    [[nodiscard]] const KeyValue &maybe_get_key(const Key &key) const;

    /**
     * did this key's press start time occur before start_ms
     */
    [[nodiscard]] bool was_key_pressed_since(const Key &key, uint64_t start_ms) const;
    [[nodiscard]] bool was_key_pressed_since(SDL_Scancode scan_code, uint64_t start_ms) const;
    [[nodiscard]] bool was_key_pressed_since(SDL_Keycode key_code, uint64_t start_ms) const;

    /**
     * sync the state with SDL_GetKeyboardState
     */
    void sync_key_state();

    /**
     * gets count of all keys monitored, not including keys
     * automatically monitored such as shift-variants
     */
    [[nodiscard]] std::size_t num_keys_monitored() const;
};
