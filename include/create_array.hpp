// original author: https://stackoverflow.com/a/68756413/854854

#include <array>
#include <type_traits>

namespace traits {

template<typename T, typename... Ts>
struct array_type {
    using type = T;
};

template<typename T, typename... Ts>
static constexpr bool are_same_type() {
    return std::conjunction_v<std::is_same<T, Ts>...>;
}

}

template<typename... T>
constexpr auto create_array(const T&&... values) {
    using array_type = typename traits::array_type<T...>::type;
    static_assert(sizeof...(T) > 0, "an array must have at least one element");
    static_assert(traits::are_same_type<T...>(), "all elements must have same type");
    return std::array<array_type, sizeof...(T)>{ values... };
}

template<typename T, typename... Ts>
constexpr auto create_array_t(const Ts&&... values) {
    using array_type = T;
    static_assert(sizeof...(Ts) > 0, "an array must have at least one element");
    static_assert(traits::are_same_type<Ts...>(), "all elements must have same type");
    return std::array<array_type, sizeof...(Ts)>{ static_cast<T>(values)... };
}
