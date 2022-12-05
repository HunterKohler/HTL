#ifndef HTL_DETAIL_BIT_H_
#define HTL_DETAIL_BIT_H_

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

} // namespace htl::detail

#endif
