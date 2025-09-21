#pragma once

#include "key.hpp"

#include <SDL3/SDL.h>

#include <cstdint>
#include <expected>
#include <initializer_list>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <version>

// TODO: refactor to make private
using Interval = std::pair<uint64_t, std::optional<uint64_t>>;
using KeyValue = std::optional<Interval>;
using KeySet = std::unordered_set<Key, KeyEquivalentHash<>, KeyEquivalentEqualTo<>>;
using KeyAtTime = std::tuple<Key, uint64_t, uint64_t>;

class ActiveKeys {
    /**
     * missing from map = key is not monitored
     */
    std::unordered_map<Key, KeyValue, KeyEquivalentHash<>, KeyEquivalentEqualTo<>> key_timings;
    std::unordered_set<SDL_Scancode> registered_scan_codes;

    void _press_key(const Key &key, uint64_t now_ms);
    void _release_key(const Key &key, uint64_t now_ms);

    /**
     * new copy of a key or nullopt
     */
    [[nodiscard]] std::optional<Key> registered_key_or_empty(const Key &key) const;

public:
    ActiveKeys() = default;

    /**
     * if specifying a key w/o a modifier: it will also monitor the shift version of the key
     * if a key with a modifier is specified, it will only listen to that exact key
     */
    explicit ActiveKeys(std::initializer_list<Key> keys_to_monitor);
    explicit ActiveKeys(std::initializer_list<Keyish> keys_to_monitor);
    explicit ActiveKeys(std::initializer_list<SDL_Scancode> scan_codes);
    explicit ActiveKeys(std::initializer_list<std::pair<SDL_Scancode, SDL_Keymod>> scan_codes_with_mods);
    explicit ActiveKeys(std::initializer_list<SDL_Keycode> key_codes);

#if __cpp_lib_containers_ranges
    /**
     * if specifying a key w/o a modifier: it will also monitor the shift version of the key
     * if a key with a modifier is specified, it will only listen to that exact key
     */
    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, Key>
    explicit ActiveKeys(R &&keys_to_monitor)
        // clang-format off
        : registered_scan_codes(std::from_range, std::forward<R>(keys_to_monitor) |
                                std::views::transform([](auto key) { return key.get_scan_code(); })), 
        key_timings(std::from_range, std::forward<R>(keys_to_monitor) |
                      std::views::transform([](auto key) {
                          return std::make_pair(key, std::nullopt);
                      })) {
        // clang-format on
        for (auto const key : std::forward<R>(keys_to_monitor)) {
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), std::nullopt});
        }
    }

    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, SDL_Scancode>
    explicit ActiveKeys(R &&scan_codes_to_monitor)
        // clang-format off
        : registered_scan_codes(std::from_range, std::forward<R>(scan_codes_to_monitor)), 
        key_timings(std::from_range, std::forward<R>(scan_codes_to_monitor) |
                      std::views::transform([](auto scan_code) {
                          const Key key{scan_code};
                          return std::make_pair(key, std::nullopt);
                      })) {
        // clang-format on
        for (auto const key_code : std::forward<R>(scan_codes_to_monitor)) {
            const Key key{key_code};
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), std::nullopt});
        }
    }

    template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, SDL_Keycode>
    explicit ActiveKeys(R &&key_codes_to_monitor)
        // clang-format off
        : key_timings(std::from_range, std::forward<R>(key_codes_to_monitor) |
                      std::views::transform([](auto key_code) {
                          const Key key{key_code};
                          return std::make_pair(key, std::nullopt);
                      })),
          registered_scan_codes(std::from_range, key_timings |
                                std::views::transform([](auto tpl) { return std::get<0>(tpl).get_scan_code(); })) {
        // clang-format on
        for (auto const key_code : std::forward<R>(key_codes_to_monitor)) {
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
                      })),
          registered_scan_codes(std::from_range, key_timings |
                                std::views::transform([](auto tpl) { return std::get<0>(tpl).get_scan_code(); })) {
        // clang-format on
        for (auto const keyish : std::forward<R>(keys_to_monitor)) {
            const Key key{keyish};
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), std::nullopt});
        }
    }
#endif

    /**
     * register a key for listening
     * likewise with the ctor: unmodded key listens to shifted and non shifted
     * shifted key listens to only the shifted key
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
     * return a set of the currently monitored keys
     */
    [[nodiscard]] KeySet get_monitored_keys() const;

    [[nodiscard]] bool is_key_registered(Key const &key) const;

    /**
     * see if the scancode of the key was used at all, and if so see which one was pressed for the longest for
     * this frame. the returned start time will always be greater than or equal to the start_ms
     * argument (so keys still held down on the last frame will be at start_ms).
     * keys that were released after the start time are excluded, and keys that are still currently pressed will
     * report end_ms as their end time
     */
    [[nodiscard]] std::optional<KeyAtTime> which_key_variant_was_pressed_since(uint64_t start_ms, uint64_t end_ms,
                                                                               const Key &key) const;

    [[nodiscard]] std::optional<KeyAtTime> which_key_variant_was_pressed_since(uint64_t start_ms, uint64_t end_ms,
                                                                               SDL_Scancode scan_code) const;

    [[nodiscard]] std::expected<KeyValue, std::string> get(const Key &key) const;
};
