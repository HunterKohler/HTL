#ifndef HTL_COMPARE_H_
#define HTL_COMPARE_H_

#include <compare>
#include <concepts>
#include <type_traits>
#include <htl/concepts.h>
#include <htl/detail/compare.h>

namespace htl {

/**
 * Synthetic three way comparison operation.
 *
 * Requires only that the two types compared have implemented the less than
 * operators (irrespective of order). Falls back to weakest implemented
 * ordering, or, finally @c std::weak_ordering if only the less than
 * comparisans can be made.
 *
 * @em (niebloid)
 */
inline constexpr detail::SynthThreeWayFn synth_three_way{};

/**
 * Resulting category of a synthetic three way comparison.
 */
template <class T, class U = T>
using SynthThreeWayResult =
    decltype(synth_three_way(std::declval<T &>(), std::declval<U &>()));

/**
 * Requirements for synthetic three way comparison for one type.
 *
 * @tparam The type to be compared.
 * @tparam Minimum ordering category to satisfy.
 */
// clang-format off
template <class T, class Cat = std::partial_ordering>
concept SynthThreeWayComparable =
    requires (const std::remove_reference_t<T> &a,
              const std::remove_reference_t<T> &b) {
        { a < b } -> BooleanTestable;
        { b < a } -> BooleanTestable;
    } && std::same_as<std::common_comparison_category_t<Cat,
        SynthThreeWayResult<T>>, Cat>;

/**
 * Requirements for synthetic three way comparison between two types
 *
 * @tparam The first type to be compared.
 * @tparam The second type to be compared.
 * @tparam Minimum ordering category to satisfy.
 */
template <class T, class U, class Cat = std::partial_ordering>
concept SynthThreeWayComparableWith =
    SynthThreeWayComparable<T, Cat> &&
    SynthThreeWayComparable<U, Cat> &&
    std::common_reference_with<
        const std::remove_reference_t<T> &,
        const std::remove_reference_t<U> &> &&
    SynthThreeWayComparable<
        std::common_reference_t<
            const std::remove_reference_t<T> &,
            const std::remove_reference_t<U> &>, Cat>;
// clang-format on

} // namespace htl

#endif
