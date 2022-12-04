#ifndef HTL_DETAIL_TYPE_TRAITS_H_
#define HTL_DETAIL_TYPE_TRAITS_H_

#include <cstddef>
#include <type_traits>

namespace htl::detail {

template <class>
struct is_char_helper : std::false_type {};

template <>
struct is_char_helper<char> : std::true_type {};

template <>
struct is_char_helper<signed char> : std::true_type {};

template <>
struct is_char_helper<unsigned char> : std::true_type {};

template <>
struct is_char_helper<wchar_t> : std::true_type {};

template <>
struct is_char_helper<char8_t> : std::true_type {};

template <>
struct is_char_helper<char16_t> : std::true_type {};

template <>
struct is_char_helper<char32_t> : std::true_type {};

template <class>
struct is_byte_helper : std::false_type {};

template <>
struct is_byte_helper<unsigned char> : std::true_type {};

template <>
struct is_byte_helper<std::byte> : std::true_type {};

} // namespace htl::detail

#endif
