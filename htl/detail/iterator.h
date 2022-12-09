#ifndef HTL_DETAIL_ITERATOR_H_
#define HTL_DETAIL_ITERATOR_H_

#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <htl/ascii.h>

namespace htl::detail {

template <class I, class O>
concept can_memcpy =
    std::contiguous_iterator<I> && //
    std::contiguous_iterator<O> && //
    std::input_iterator<I> && //
    std::output_iterator<O, std::iter_value_t<I>> && //
    std::same_as<std::iter_value_t<I>, std::iter_value_t<O>> && //
    std::is_trivially_copyable_v<std::iter_value_t<I>>;

template <class T, std::output_iterator<T> O>
void iter_write(T &&value, O &out)
{
    *out = std::forward<T>(value);
    ++out;
}

template <std::output_iterator<char> O>
void iter_write_string(std::string_view str, O &out)
{
    if constexpr (can_memcpy<std::string_view::iterator, O>) {
        std::memmove(std::to_address(out), str.data(), str.size());
        out += str.size();
    } else {
        for (auto &c: str) {
            iter_write(c, out);
        }
    }
}

template <std::output_iterator<char> O>
void iter_write_string(const char *str, O &out)
{
    for (; *str; ++str) {
        iter_write(*str, out);
    }
}

void iter_write_int(std::integral auto value, auto &out)
{
    char buf[std::numeric_limits<decltype(value)>::digits10 + 2];
    auto res = std::to_chars(buf, std::end(buf), value);
    for (auto ptr = buf; ptr != res.ptr; ++ptr) {
        iter_write(*ptr, out);
    }
}

void iter_write_float(std::floating_point auto value, auto &out)
{
    char buf[std::numeric_limits<decltype(value)>::max_digits10 + 2];
    auto res = std::to_chars(buf, std::end(buf), value);
    for (auto ptr = buf; ptr != res.ptr; ++ptr) {
        iter_write(*ptr, out);
    }
}

} // namespace htl::detail

#endif
