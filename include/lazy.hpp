#pragma once

#include <functional>
#include <optional>
#include <utility>

// ref: https://stackoverflow.com/a/41955881

/**
 * a lazy-evaluated (deferred) value
 */
template <typename Return> class Lazy {
    std::optional<Return> result;
    std::function<Return()> f;

    const Return &evaluate() {
        if (result.has_value()) {
            return *result;
        }

        result = std::make_optional(f());
        return *result;
    }

public:
    Lazy(std::function<Return()> &&f) : result(std::nullopt), f(std::move(f)) {
    }

    const Return &operator*() {
        return evaluate();
    }

    const Return *operator->() {
        return &evaluate();
    }

    const Return &operator()() {
        return evaluate();
    }
};
