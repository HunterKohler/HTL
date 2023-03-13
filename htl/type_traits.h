#ifndef HTL_TYPE_TRAITS_H_
#define HTL_TYPE_TRAITS_H_

#include <type_traits>
#include <htl/detail/type_traits.h>

namespace htl {

/**
 * Identifies if the type `T` is a character. Namely, `char`,
 * `unsigned char`, `signed char`, `wchar_t`, `char8_t`, `char16_t`,
 * `char32_t`, or a const-volatile qualified version thereof.
 *
 * @tparam T The target type.
 */
template <class T>
struct is_char : detail::is_char_helper<std::remove_cv_t<T>> {};

/**
 * Identifies if the type `T` is a byte. Namely,
 * `unsigned char`, `std::byte`, or a const-volatile qualified type thereof.
 *
 * @tparam T The target type.
 */
template <class T>
struct is_byte : detail::is_byte_helper<std::remove_cv_t<T>> {};

template <class T>
inline constexpr bool is_char_v = is_char<T>::value;

template <class T>
inline constexpr bool is_byte_v = is_byte<T>::value;

} // namespace htl

#endif
