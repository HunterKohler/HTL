#ifndef HTL_TO_CHARS_H_
#define HTL_TO_CHARS_H_

#include <charconv>
#include <concepts>
#include <iterator>
#include <limits>
#include <ranges>

namespace htl {

template <std::ostream_iterator<char> O>
constexpr O to_chars(bool value, O out)
{
    return value ? std::ranges::copy("true", std::move(out)).out
                 : std::ranges::copy("false", std::move(out)).out;
}

template <std::integral T, std::ostream_iterator<char> O>
constexpr O to_chars(T value, O out, int base = 10)
{
    char buf[std::numeric_limits<T>::digits10 + 10];
    auto res = std::to_chars(buf, std::end(buf), value, base);
    return std::ranges::copy(buf, res.ptr, std::move(out)).out;
}

template <std::floating_point T, std::ostream_iterator<char> O>
constexpr O to_chars(T value, O out)
{
    char buf[std::numeric_limits<T>::max_digits10 + 10];
    auto res = std::to_chars(buf, std::end(buf), value);
    return std::ranges::copy(buf, res.ptr, std::move(out)).out;
}

template <std::floating_point T, std::ostream_iterator<char> O>
constexpr O to_chars(T value, O out, std::chars_format fmt)
{
    char buf[std::numeric_limits<T>::max_digits10 + 10];
    auto res = std::to_chars(buf, std::end(buf), value);
    return std::ranges::copy(buf, res.ptr, std::move(out)).out;
}

template <std::floating_point T, std::ostream_iterator<char> O>
constexpr O to_chars(T value, O out, std::chars_format fmt, int precision)
{
    char buf[std::numeric_limits<T>::max_digits10 + 10];
    auto res = std::to_chars(buf, std::end(buf), value, fmt, precision);
    return std::ranges::copy(buf, res.ptr, std::move(out)).out;
}

} // namespace htl

#endif
