#ifndef HTL_MATH_H_
#define HTL_MATH_H_

#include <type_traits>

namespace htl {

template <class T>
    requires std::is_arithmetic_v<T>
constexpr int sign(T value) noexcept
{
    if constexpr (std::is_same_v<T, bool>) {
        return value;
    } else if constexpr (std::is_unsigned_v<T>) {
        return value > T{};
    } else if constexpr (std::is_signed_v<T>) {
        return (value > T{}) - (value < T{});
    }
}

} // namespace htl

#endif
