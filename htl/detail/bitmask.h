#ifndef HTL_DETAIL_BITMASK_H_
#define HTL_DETAIL_BITMASK_H_

#include <type_traits>

namespace htl::detail {

template <class T>
    requires std::is_enum_v<T>
constexpr T bitmask_and(T a, T b) noexcept
{
    return static_cast<T>(
        static_cast<std::underlying_type_t<T>>(a) &
        static_cast<std::underlying_type_t<T>>(b));
}

template <class T>
    requires std::is_enum_v<T>
constexpr T bitmask_or(T a, T b) noexcept
{
    return static_cast<T>(
        static_cast<std::underlying_type_t<T>>(a) |
        static_cast<std::underlying_type_t<T>>(b));
}

template <class T>
    requires std::is_enum_v<T>
constexpr T bitmask_xor(T a, T b) noexcept
{
    return static_cast<T>(
        static_cast<std::underlying_type_t<T>>(a) ^
        static_cast<std::underlying_type_t<T>>(b));
}

template <class T>
    requires std::is_enum_v<T>
constexpr T bitmask_not(T a) noexcept
{
    return static_cast<T>(~static_cast<std::underlying_type_t<T>>(a));
}

} // namespace htl::detail

#endif
