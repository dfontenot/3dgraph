#include "active_keys.hpp"
#include "key.hpp"

#include <SDL3/SDL.h>

#include <cassert>
#include <cstdint>
#include <expected>
#include <initializer_list>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

using std::expected;
using std::initializer_list;
using std::make_optional;
using std::make_pair;
using std::make_tuple;
using std::nullopt;
using std::optional;
using std::pair;
using std::span;
using std::string;
using std::unexpected;
using std::vector;

ActiveKeys::ActiveKeys(initializer_list<Key> keys_to_monitor) {
    for (auto const key : keys_to_monitor) {
        start_listen_to_key(key);
    }
}

ActiveKeys::ActiveKeys(initializer_list<SDL_Scancode> scan_codes) {
    for (auto const scan_code : scan_codes) {
        start_listen_to_key(Key{scan_code});
    }
}

ActiveKeys::ActiveKeys(initializer_list<pair<SDL_Scancode, SDL_Keymod>> scan_codes_with_mods) {
    for (auto const scan_code_with_mods : scan_codes_with_mods) {
        start_listen_to_key(Key{scan_code_with_mods});
    }
}

ActiveKeys::ActiveKeys(initializer_list<SDL_Keycode> key_codes) {
    for (auto const key_code : key_codes) {
        start_listen_to_key(Key{key_code});
    }
}

ActiveKeys::ActiveKeys(initializer_list<Keyish> keys_to_monitor) {
    for (auto const keyish : keys_to_monitor) {
        start_listen_to_key(Key{keyish});
    }
}

void ActiveKeys::start_listen_to_key(SDL_Scancode scan_code) {
    start_listen_to_key(Key{scan_code});
}

void ActiveKeys::start_listen_to_key(SDL_Keycode key_code) {
    start_listen_to_key(Key{key_code});
}

void ActiveKeys::start_listen_to_key(const Key &key) {
    // no current support for ctrl or alt
    if (key.has_alt() || key.has_ctrl()) {
        return;
    }

    key_timings.insert({key, nullopt});
    registered_scan_codes.insert(key.get_scan_code());

    if (!key.has_modifier() && key.is_alpha()) {
        key_timings.insert({key.copy_shifted(), nullopt});
    }
}

void ActiveKeys::press_key(const Key &key) {
    if (!registered_scan_codes.contains(key.get_scan_code())) {
        return;
    }

    auto const now_ms = SDL_GetTicks();
    _press_key(key.without_shift(), now_ms);

    if (key.has_shift()) {
        _press_key(key, now_ms);
    }

    if (!key.has_shift()) {
        auto const with_shift = key.copy_shifted();
        if (is_key_registered(with_shift) && key_timings[with_shift].has_value() &&
            !key_timings[with_shift]->second.has_value()) {
            _release_key(with_shift, now_ms);
        }
    }
}

void ActiveKeys::_press_key(const Key &key, uint64_t now_ms) {
    if (!is_key_registered(key)) {
        return;
    }

    auto const key_timing = key_timings[key];
    if (!key_timing.has_value()) {
        // this is the first time the key has ever been pressed
        key_timings[key] = make_optional(make_pair(now_ms, nullopt));
    }
    else {
        // if this key has been previously released, clear out the old
        // entry and start a new keypress from this time
        if (std::get<1>(*key_timing).has_value()) {
            key_timings[key]->first = now_ms;
            key_timings[key]->second = nullopt;
        }
    }
}

void ActiveKeys::release_key(const Key &key) {
    if (!registered_scan_codes.contains(key.get_scan_code())) {
        return;
    }

    auto const now_ms = SDL_GetTicks();
    _release_key(key, now_ms);
    _release_key(key.shift_mod_complement(), now_ms);
}

void ActiveKeys::_release_key(const Key &key, uint64_t now_ms) {
    if (!is_key_registered(key)) {
        return;
    }

    auto const key_timing = key_timings[key];
    if (key_timing.has_value()) {
        if (!std::get<1>(*key_timing).has_value()) {
            key_timings[key]->second = make_optional(now_ms);
        }
    }
    else {
        // original key press has been lost, make up key press start
        key_timings[key] = make_pair(now_ms, make_optional(now_ms));
    }
}

void ActiveKeys::sync_key_state() {
    int num_keys = -1;
    auto const key_states = SDL_GetKeyboardState(&num_keys);
    assert(num_keys > 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    span s{key_states, key_states + num_keys};

    for (auto const entry : key_timings) {
        auto const key = std::get<0>(entry);
        auto const scan_code = key.get_scan_code();
        if (s[static_cast<size_t>(scan_code)]) {
            press_key(key);
        }
        else {
            release_key(key);
        }
    }
}

bool ActiveKeys::was_key_pressed_since(const Key &key, uint64_t start_ms) const {
    if (!is_key_registered(key)) {
        return false;
    }

    auto const key_timing = key_timings.at(key);
    return key_timing
        .transform([start_ms](auto key_timing) {
            return std::get<1>(key_timing)
                .transform(
                    [&, start_ms](auto end_time) { return key_timing.first <= start_ms && end_time >= start_ms; })
                .value_or(key_timing.first <= start_ms);
        })
        .value_or(false);
}

bool ActiveKeys::was_key_pressed_since(SDL_Scancode scan_code, uint64_t start_ms) const {
    auto const key = Key(scan_code);
    return was_key_pressed_since(key, start_ms) || was_key_pressed_since(key.shift_mod_complement(), start_ms);
}

bool ActiveKeys::was_key_pressed_since(SDL_Keycode key_code, uint64_t start_ms) const {
    auto const key = Key(key_code);

    // if the keycode already represents a scancode that was shifted (e.g., plus)
    // then do not count pressing = as the same as + being released
    return was_key_pressed_since(key, start_ms) ||
           (!key.has_shift() && was_key_pressed_since(key.shift_mod_complement(), start_ms));
}

KeySet ActiveKeys::get_monitored_keys() const {
    KeySet keys;
    auto keys_only = key_timings | std::views::transform([](auto entry) { return std::get<0>(entry); });
    keys.insert_range(keys_only);

    return keys;
}

[[nodiscard]] bool ActiveKeys::is_key_registered(Key const &key) const {
    return key_timings.contains(key);
}

optional<KeyAtTime> ActiveKeys::which_key_variant_was_pressed_since(uint64_t start_ms, uint64_t end_ms,
                                                                    SDL_Scancode scan_code) const {
    return which_key_variant_was_pressed_since(start_ms, end_ms, Key{scan_code});
}

optional<KeyAtTime> ActiveKeys::which_key_variant_was_pressed_since(uint64_t start_ms, uint64_t end_ms,
                                                                    const Key &key) const {
    using std::get;

    if (!registered_scan_codes.contains(key.get_scan_code())) {
        return nullopt;
    }

    const Key with_shift = key.copy_shifted();
    const Key without_shift = key.without_mods();
    auto const maybe_unshifted_key = registered_key_or_empty(without_shift);
    auto const maybe_shifted_key = registered_key_or_empty(with_shift);

    auto const maybe_unshifted_key_timing = maybe_unshifted_key.and_then([&](auto key) { return key_timings.at(key); });
    auto const maybe_shifted_key_timing = maybe_shifted_key.and_then([&](auto key) { return key_timings.at(key); });

    if (!maybe_unshifted_key_timing.has_value() && !maybe_shifted_key_timing.has_value()) {
        return nullopt;
    }

    // xor
    if (maybe_unshifted_key_timing.has_value() != maybe_shifted_key_timing.has_value()) {

        if (maybe_shifted_key_timing.has_value()) {
            auto const shift_key_timing = *maybe_shifted_key_timing;
            auto const maybe_shift_key_end_ms = get<1>(shift_key_timing);
            auto const start_time_ms = std::max(get<0>(shift_key_timing), start_ms);

            // button was released before the start time under consideration
            if (maybe_shift_key_end_ms.has_value() && maybe_shift_key_end_ms.value() < start_ms) {
                return nullopt;
            }

            // button is still held down
            if (!maybe_shift_key_end_ms.has_value()) {
                return make_optional(make_tuple(with_shift, start_time_ms, end_ms));
            }
            else {
                return make_optional(make_tuple(with_shift, start_time_ms, *maybe_shift_key_end_ms));
            }
        }
        else {
            auto const this_key_timing = *maybe_unshifted_key_timing;
            auto const maybe_shift_key_end_ms = get<1>(this_key_timing);
            auto const start_time_ms = std::max(get<0>(this_key_timing), start_ms);

            // button was released before the start time under consideration
            if (maybe_shift_key_end_ms.has_value() && maybe_shift_key_end_ms.value() < start_ms) {
                return nullopt;
            }

            // button is still held down
            if (!maybe_shift_key_end_ms.has_value()) {
                return make_optional(make_tuple(without_shift, start_time_ms, end_ms));
            }
            else {
                return make_optional(make_tuple(without_shift, start_time_ms, *maybe_shift_key_end_ms));
            }
        }
    }

    // TODO: consider case of registered keys but neither were pressed

    // tie-breaker if both keys were pressed
    auto const shift_key_timing = *maybe_shifted_key_timing;
    auto const this_key_timing = *maybe_unshifted_key_timing;

    // arbitrary: give shift key precedence if either are still
    // held down at the end of the frame
    if (!get<1>(shift_key_timing).has_value() && !get<1>(this_key_timing).has_value()) {
        return make_optional(make_tuple(with_shift, get<0>(shift_key_timing), end_ms));
    }
    else {
        return make_optional(make_tuple(without_shift, get<0>(shift_key_timing), end_ms));
    }

    return nullopt;
}

optional<Key> ActiveKeys::registered_key_or_empty(const Key &key) const {
    if (is_key_registered(key)) {
        return make_optional(key);
    }

    return nullopt;
}

expected<KeyValue, string> ActiveKeys::get(const Key &key) const {
    if (!is_key_registered(key)) {
        return unexpected(std::format("key {0} not registered", key));
    }

    return key_timings.at(key);
}
