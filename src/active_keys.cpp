#include "active_keys.hpp"
#include "key.hpp"
#include "key_mod.hpp"

#include <SDL3/SDL.h>

#include <cassert>
#include <initializer_list>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

using std::initializer_list;
using std::make_optional;
using std::make_pair;
using std::nullopt;
using std::optional;
using std::pair;
using std::span;
using std::vector;

namespace {
KeyValue const unmonitored_key = nullopt;
}

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

const KeyValue &ActiveKeys::maybe_get_key(const Key &key) const {
    auto const key_loc = key_timings.find(key);
    if (key_loc == key_timings.end()) {
        return unmonitored_key;
    }

    return key_loc->second;
}

bool ActiveKeys::is_key_registered(const Key &key) const {
    return key_timings.find(key) != key_timings.end();
}

void ActiveKeys::start_listen_to_key(SDL_Scancode scan_code) {
    start_listen_to_key(Key(scan_code));
}

void ActiveKeys::start_listen_to_key(SDL_Keycode key_code) {
    start_listen_to_key(Key(key_code));
}

void ActiveKeys::start_listen_to_key(const Key &key) {
    key_timings.insert({key, nullopt});

    if (!key.has_modifier() && key.is_alpha()) {
        key_timings.insert({key.copy_shifted(), nullopt});
    }
}

void ActiveKeys::press_key(const Key &key) {
    if (!is_key_registered(key)) {
        return;
    }

    auto const now_ms = SDL_GetTicks();
    auto const maybe_key = maybe_get_key(key);
    if (!maybe_key.has_value()) {
        // this is the first time the key has ever been pressed
        key_timings[key] = make_optional(make_pair(now_ms, nullopt));
    }
    else {
        // if this key has been previously released, clear out the old
        // entry and start a new keypress from this time
        if (std::get<1>(*maybe_key).has_value()) {
            key_timings[key]->first = now_ms;
            key_timings[key]->second = nullopt;
        }
    }

    if (key.has_modifier() && key.is_alpha()) {
        auto const un_modded = key.without_mods();
        if (!is_key_registered(un_modded)) {
            return;
        }

        auto const maybe_un_modded_key = maybe_get_key(un_modded);
        if (!maybe_un_modded_key.has_value()) {
            // this is the first time the key has ever been pressed
            key_timings[un_modded] = make_optional(make_pair(now_ms, nullopt));
        }
        else {
            // if the unmodded version of this key has already been released
            // then overwrite its value
            if (std::get<1>(*maybe_un_modded_key).has_value()) {
                key_timings[un_modded]->first = now_ms;
                key_timings[un_modded]->second = nullopt;
            }
        }
    }
    else {
        // see if there is a modifier version of this already pressed
        // and if so, mark it as no longer pressed
        // e.g., shift-D was held down at the start and now the shift has been
        // released while the d key is still held down
        auto const shift_modded = key.copy_shifted(true);
        if (!is_key_registered(shift_modded)) {
            return;
        }

        auto const maybe_modded = maybe_get_key(shift_modded);
        if (maybe_modded.has_value()) {
            key_timings[shift_modded]->second = make_optional(now_ms);
        }
    }
}

void ActiveKeys::release_key(const Key &key) {
    if (!is_key_registered(key)) {
        return;
    }

    auto const now_ms = SDL_GetTicks();
    auto const maybe_key_timing = maybe_get_key(key);

    if (maybe_key_timing.has_value()) {
        key_timings[key]->second = make_optional(now_ms);
    }

    if (!key.has_modifier()) {
        auto const modded = key.copy_shifted();
        if (!is_key_registered(modded)) {
            return;
        }

        auto const maybe_key_timing_modded = maybe_get_key(modded);

        if (maybe_key_timing_modded.has_value() && !key_timings[modded]->second.has_value()) {
            key_timings[modded]->second = make_optional(now_ms);
        }
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

    auto const maybe_key_timing = maybe_get_key(key);
    return maybe_key_timing
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

optional<KeyMod> ActiveKeys::what_key_mods_pressed_since(Key const &key, SDL_Keymod mask, uint64_t start_ms) const {
    const KeyMod key_mod_mask{mask};

    // TODO: make this work on non alpha keys such as checking mods used with plus
    auto const without_mods = key.without_mods();
    bool found = is_key_registered(without_mods);
    if (!was_key_pressed_since(without_mods, start_ms)) {
        return nullopt;
    }

    KeyMod found_mods;
    for (size_t bit = 0; bit < sizeof(SDL_Keymod); bit++) {
        if (key_mod_mask.test(bit)) {

            KeyMod only_this_mod;
            only_this_mod.set(bit);
            if (was_key_pressed_since(key.copy_with_mods(only_this_mod), start_ms)) {
                found_mods.set(bit);
            }
        }
    }

    if (!found) {
        return nullopt;
    }
    else {
        return make_optional(found_mods);
    }
}

optional<KeyMod> ActiveKeys::what_key_mods_pressed_since(SDL_Scancode scan_code, SDL_Keymod mask,
                                                         uint64_t start_ms) const {
    return what_key_mods_pressed_since(Key{scan_code}, mask, start_ms);
}

optional<KeyMod> ActiveKeys::what_key_mods_pressed_since(SDL_Keycode key_code, SDL_Keymod mask,
                                                         uint64_t start_ms) const {
    return what_key_mods_pressed_since(Key{key_code}, mask, start_ms);
}
