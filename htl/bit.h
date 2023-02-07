#ifndef HTL_BIT_H_
#define HTL_BIT_H_

#include <bit>
#include <climits>
#include <concepts>
#include <span>
#include <htl/config.h>
#include <htl/detail/bit.h>

namespace htl {

inline constexpr std::size_t byte_size = CHAR_BIT;

// [bit.byteswap], byteswap (C++23)
template <std::integral T>
constexpr T byteswap(T value) noexcept
{
    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        return detail::byteswap16(value);
    } else if constexpr (sizeof(T) == 4) {
        return detail::byteswap32(value);
    } else if constexpr (sizeof(T) == 8) {
        return detail::byteswap64(value);
    } else if constexpr (sizeof(T) == 16) {
        return detail::byteswap128(value);
    } else {
        return detail::byteswap_any(value);
    }
}

} // namespace htl

#endif
