#ifndef HTL_DETAIL_CONCEPTS_H_
#define HTL_DETAIL_CONCEPTS_H_

#include <cstdint>
#include <ranges>
#include <tuple>
#include <utility>

// clang-format off

namespace htl::detail {

template <class T, std::size_t N>
inline constexpr bool is_tuple_element = requires (T t) {
    typename std::tuple_element_t<N - 1, std::remove_const_t<T>>;
    { std::get<N - 1>(t) } ->
        std::convertible_to<std::tuple_element_t<N - 1, T>>;
    requires is_tuple_element<T, N - 1>;
};

template <class T>
inline constexpr bool is_tuple_element<T, 0> = true;

} // namespace htl::detail

#endif
