#ifndef HTL_UTILITY_H_
#define HTL_UTILITY_H_

#include <bit>
#include <concepts>
#include <type_traits>
#include <htl/config.h>

namespace htl {

// C++ 23
template <class E>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

// C++ 23
[[noreturn]] inline void unreachable()
{
#if HTL_HAS_BUILTIN(__builtin_unreachable)
    __builtin_unreachable();
#elif HLIB_MSVC
    __assume(false);
#else
    std::abort();
#endif
}

} // namespace htl

#endif
