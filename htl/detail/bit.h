#ifndef HTL_DETAIL_BIT_H_
#define HTL_DETAIL_BIT_H_

#include <climits>
#include <cstdint>
#include <htl/config.h>

namespace htl::detail {

constexpr std::uint16_t byteswap16(std::uint16_t value) noexcept
{
#if HTL_HAS_BUILTIN(__builtin_bswap16)
    return __builtin_bswap16(value);
#else
    return ((value << 8) & 0xFF00) | ((value >> 8) & 0xFF);
#endif
}

constexpr std::uint32_t byteswap32(std::uint32_t value) noexcept
{
#if HTL_HAS_BUILTIN(__builtin_bswap32)
    return __builtin_bswap32(value);
#else
    return ((value << 24) & 0xFF000000) | ((value << 8) & 0xFF0000) |
           ((value >> 8) & 0xFF00) | ((value >> 24) & 0xFF);
#endif
}

constexpr std::uint64_t byteswap64(std::uint64_t value) noexcept
{
#if HTL_HAS_BUILTIN(__builtin_bswap64)
    return __builtin_bswap64(value);
#else
    return ((value << 56) & 0xFF00000000000000) |
           ((value << 40) & 0xFF000000000000) |
           ((value << 24) & 0xFF0000000000) | ((value << 8) & 0xFF00000000) |
           ((value >> 8) & 0xFF000000) | ((value >> 24) & 0xFF0000) |
           ((value >> 40) & 0xFF00) | ((value >> 56) & 0xFF);
#endif
}

template <class T>
constexpr T byteswap128(T value) noexcept
{
#if HTL_HAS_BUILTIN(__builtin_bswap128)
    return __builtin_bswap128(value);
#else
    return byteswap64(value >> 64) | (static_cast<T>(byteswap64(value)) << 64);
#endif
}

template <std::integral T>
constexpr T byteswap_any(T value) noexcept
{
    using uint_type = std::make_unsigned_t<T>;

    std::size_t n = sizeof(T) / 2;
    std::size_t shift = CHAR_BIT * (sizeof(T) - 1);
    uint_type low_mask = static_cast<unsigned char>(~0);
    uint_type high_mask = low_mask << shift;
    uint_type new_value = 0;
    uint_type uvalue = value;

    for (int i = 0; i < n; i++) {
        uint_type low = uvalue & low_mask;
        uint_type high = uvalue & high_mask;
        new_value |= low << shift;
        new_value |= high >> shift;
        low_mask <<= CHAR_BIT;
        high_mask >>= CHAR_BIT;
        shift -= 2 * CHAR_BIT;
    }

    return new_value;
}

} // namespace htl::detail

#endif
