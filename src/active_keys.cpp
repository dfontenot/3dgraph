#include "active_keys.hpp"
#include "key.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_timer.h>
#include <initializer_list>
#include <optional>
#include <utility>
#include <vector>

using std::initializer_list;
using std::make_optional;
using std::make_pair;
using std::nullopt;
using std::optional;

namespace {
KeyValue unmonitored_key = nullopt;
}

ActiveKeys::ActiveKeys() {
}

ActiveKeys::ActiveKeys(initializer_list<Key> keys_to_monitor) {
    for (auto const key : keys_to_monitor) {
        monitored_keys.push_back(key.get_scan_code());
        key_timings.insert({key, nullopt});

        if (! key.has_modifier()) {
            // TODO: another leaky abstraction, need to fix
            key_timings.insert({key.copy_shifted(), nullopt});
        }
    }
}

ActiveKeys::ActiveKeys(std::initializer_list<SDL_Scancode> scan_codes) {
    for (auto const scan_code : scan_codes) {
        monitored_keys.push_back(scan_code);
        auto const key = Key(scan_code);
        key_timings.insert({key, nullopt});
        key_timings.insert({key.copy_shifted(), nullopt});
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

void ActiveKeys::start_listen_to_key(const Key &key) {
    key_timings.insert({key, nullopt});
    monitored_keys.push_back(key.get_scan_code());

    if (!key.has_modifier()) {
        key_timings.insert({key.copy_shifted(), nullopt});
    }
}

void ActiveKeys::start_listen_to_key(Key &&key) {
    key_timings.insert({key, nullopt});
    monitored_keys.push_back(key.get_scan_code());

    if (!key.has_modifier()) {
        key_timings.insert({key.copy_shifted(), nullopt});
    }
}

void ActiveKeys::set_key_pressed(const Key &key) {
    if (!is_key_registered(key)) {
        return;
    }

    // set the start time for this specific key and modifier
    auto const now_ms = SDL_GetTicks();
    if (!maybe_get_key(key).has_value()) {
        key_timings[key] = make_optional(make_pair(now_ms, nullopt));
    }

    if (key.has_modifier()) {
        // if the regular key itself isn't marked as started, do so now
        auto const un_modded = key.without_mods();
        if (!maybe_get_key(un_modded).has_value()) {
            key_timings[un_modded]->first = now_ms;
            key_timings[un_modded]->second = nullopt;
            //key_timings[un_modded] = make_optional(make_pair(now_ms, nullopt));
        }
    }
    else {
        // see if there is a modifier version of this already pressed
        // and if so, mark it as no longer pressed
        // e.g., shift-D was held down at the start and now the shift has been
        // released while the d key is still held down
        auto const shift_modded = key.copy_shifted(true);
        auto const maybe_modded = maybe_get_key(shift_modded);
        if (maybe_modded.has_value()) {
            //key_timings[shift_modded] = make_optional(make_pair(maybe_modded->first, now_ms));
            key_timings[shift_modded]->first = maybe_modded->first;
            key_timings[shift_modded]->second = now_ms;
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
        //key_timings[key] = make_optional(make_pair(maybe_key_timing->first, now_ms));
        key_timings[key]->first = maybe_key_timing->first;
        key_timings[key]->second = now_ms;
    }

    if (key.has_modifier()) {
        auto const unmodded = key.without_mods();
        auto const maybe_key_timing_unmodded = maybe_get_key(unmodded);
        if (maybe_key_timing_unmodded.has_value()) {
            //key_timings[unmodded] = make_optional(make_pair(maybe_key_timing->first, now_ms));
            key_timings[unmodded]->first = maybe_key_timing->first;
            key_timings[unmodded]->second = now_ms;
        }
    }
}

void ActiveKeys::sync_key_state() {
    auto const key_states = SDL_GetKeyboardState(reinterpret_cast<int *>(monitored_keys.data()));

    for (auto i = 0; i < monitored_keys.size(); i++) {
        if (key_states[i]) {
            set_key_pressed(Key(monitored_keys[i]));
        }
        else {
            release_key(Key(monitored_keys[i]));
        }
    }
}

bool ActiveKeys::was_key_pressed_since(const Key &key, uint64_t start_ms) const {
    if (!is_key_registered(key)) {
        return false;
    }

    auto const maybe_key_timing = maybe_get_key(key);
    return maybe_key_timing.has_value() && start_ms <= maybe_key_timing->first;
}

bool ActiveKeys::was_key_pressed_since(SDL_Scancode scan_code, uint64_t start_ms) const {
    auto const key = Key(scan_code);
    return was_key_pressed_since(key, start_ms) || was_key_pressed_since(key.shift_mod_complement(), start_ms);
}
