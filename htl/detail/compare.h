#ifndef HTL_DETAIL_COMPARE_H_
#define HTL_DETAIL_COMPARE_H_

#include <compare>
#include <htl/concepts.h>

namespace htl::detail {

// clang-format off
struct SynthThreeWayFn {
public:
    template <class T, class U>
    [[nodiscard]] constexpr auto operator()(const T &t, const U &u) const
        noexcept(std::three_way_comparable_with<T, U>
            ? noexcept(t <=> u)
            : noexcept(t < u) && noexcept(u < t))
        requires requires {
            { t < u } -> BooleanTestable;
            { u < t } -> BooleanTestable;
        }
    {
        if constexpr (std::three_way_comparable_with<T, U>) {
            return t <=> u;
        } else if (t < u) {
            return std::weak_ordering::less;
        } else if (u < t) {
            return std::weak_ordering::greater;
        } else {
            return std::weak_ordering::equivalent;
        }
    }
};
// clang-format on

} // namespace htl::detail

#endif
