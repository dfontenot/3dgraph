#pragma once

#include <cstddef>
#include <deque>
#include <iostream>
#include <format>
#include <type_traits>

template <typename T>
    requires std::is_arithmetic_v<T>
class MaxDeque {
    std::size_t capacity;
    std::deque<T> deque;
    T sum;

public:
    MaxDeque() = delete;
    constexpr MaxDeque(std::size_t capacity) : capacity(capacity), sum(0) {
    }

    /**
     * average of the values stored in the deque
     */
    constexpr T get_avg() const {
        auto const num = deque.size();

        if (num == 0) {
            return 0;
        }

        return sum / num;
    }

    /**
     * add a new value
     */
    constexpr void add(T value) {
        auto const num = deque.size();

        if (num == capacity) {
            sum -= deque.front();
            deque.pop_front();
        }

        deque.push_back(value);
        sum += value;
    }

    constexpr std::size_t size() const {
        return deque.size();
    }

    constexpr std::size_t get_capacity() const {
        return capacity;
    }
};

template <typename T>
std::ostream &operator<<(std::ostream &stream, const MaxDeque<T> &deque) {
    stream << "{ MaxDeque size " << deque.size() << " avg " << deque.get_avg() << " }";
    return stream;
}

template <typename T> struct std::formatter<MaxDeque<T>> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const MaxDeque<T> &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{ MaxDeque size {0} avg {1} }", obj.size(), obj.get_avg());
    }
};
