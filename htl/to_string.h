#ifndef HTL_TO_STRING_H_
#define HTL_TO_STRING_H_

#include <memory>
#include <string>
#include <htl/to_chars.h>

namespace htl {
template <class Alloc = std::allocator<char>>
constexpr std::basic_string<char, std::char_traits<char>, Alloc>
to_string(bool value, const Alloc &alloc = Alloc())
{
    return value ? { "true", alloc } : { "false", alloc };
}

template <class Alloc = std::allocator<char>, std::integral T>
constexpr std::basic_string<char, std::char_traits<char>, Alloc>
to_string(T value, const Alloc &alloc = Alloc(), int base = 10)
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res), base);
    return res;
}

template <class Alloc = std::allocator<char>, std::floating_point T>
constexpr std::basic_string<char, std::char_traits<char>, Alloc>
to_string(T value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res));
    return res;
}

template <class Alloc = std::allocator<char>, std::floating_point T>
constexpr std::basic_string<char, std::char_traits<char>, Alloc>
to_string(T value, const Alloc &alloc = Alloc(), std::chars_format fmt)
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res), fmt);
    return res;
}

template <class Alloc = std::allocator<char>, std::floating_point T>
constexpr std::basic_string<char, std::char_traits<char>, Alloc> to_string(
    T value, const Alloc &alloc = Alloc(), std::chars_format fmt, int precision)
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res), fmt, precision);
    return res;
}

} // namespace htl

#endif
