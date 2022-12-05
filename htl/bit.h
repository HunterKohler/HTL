#ifndef HTL_BIT_H_
#define HTL_BIT_H_

#include <algorithm>
#include <bit>
#include <concepts>
#include <ranges>
#include <span>
#include <htl/config.h>
#include <htl/detail/bit.h>

namespace htl {

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
    } else if (std::is_constant_evaluated()) {
        auto array = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
        std::ranges::reverse(array);
        return std::bit_cast<T>(value);
    } else {
        auto address = reinterpret_cast<std::byte *>(std::addressof(value));
        std::ranges::reverse(address, address + sizeof(T));
        return value;
    }
}

} // namespace htl

#endif
