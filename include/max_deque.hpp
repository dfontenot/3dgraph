#pragma once

#include <cstddef>
#include <deque>
#include <type_traits>

template <typename T>
    requires std::is_arithmetic_v<T>
class MaxDeque {
    std::size_t capacity;
    std::deque<T> deque;
    T sum;

public:
    MaxDeque(std::size_t capacity) : capacity(capacity) {
    }
    /**
     * average of the values stored in the deque
     */
    T get_avg() const {
        auto const num = deque.size();

        if (num == 0) {
            return 0;
        }

        return sum / num;
    }

    /**
     * add a new value
     */
    void add(T value) {
        auto const num = deque.size();

        if (num == capacity) {
            sum -= deque.front();
            deque.pop_front();
        }

        deque.push_back(value);
        sum += value;
    }
};
