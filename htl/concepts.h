#ifndef HTL_CONCEPTS_H_
#define HTL_CONCEPTS_H_

#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <htl/detail/concepts.h>

// clang-format off

namespace htl {

/**
 * Specifies a boolean-testable type. Specifically, that it the type `T`, and
 * its logical inverse are convertible to `bool`.
 *
 * @see https://en.cppreference.com/w/cpp/concepts/boolean-testable
 */
template <class T>
concept BooleanTestable =
    std::convertible_to<T, bool> &&
    requires (T &&t) { { !std::forward<T>(t) } -> std::convertible_to<bool>; };

/**
 * Specifies a tuple like type. That is, one which implements the standard
 * tuple-interface through `std::tuple_size`, `std::tuple_element`, and
 * `std::get`.
 */
template <class T>
concept TupleLike = requires {
    typename std::tuple_size<T>::type;
    requires !std::is_reference_v<T>;
    requires detail::is_tuple_element<T, std::tuple_size_v<T>>;
};

/**
 * Specifies a tuple like type constrained to size two, a pair.
 */
template <class T>
concept PairLike = TupleLike<T> && std::tuple_size_v<T> == 2;

/**
 * Specifies a bitmask type, like one used for bitmask flag options.
 *
 * @see https://en.cppreference.com/w/cpp/named_req/BitmaskType
 */
template <class T>
concept BitmaskType = requires(T &x, T &y)
{
    { ~x } -> std::same_as<T>;
    { x & y } -> std::same_as<T>;
    { x | y } -> std::same_as<T>;
    { x ^ y } -> std::same_as<T>;
    { x &= y } -> std::same_as<T &>;
    { x |= y } -> std::same_as<T &>;
    { x ^= y } -> std::same_as<T &>;
};

/**
 * Specifies the basic lockable protocol.
 *
 * @see https://en.cppreference.com/w/cpp/named_req/BasicLockable
 */
template <class T>
concept BasicLockable = requires(T &t)
{
    t.lock();
    t.unlock();
};

/**
 * Specifies the general lockable protocol.
 *
 * @see https://en.cppreference.com/w/cpp/named_req/Lockable
 */
template <class T>
concept Lockable = BasicLockable<T> && requires(T &t)
{
    { t.try_lock() } -> BooleanTestable;
};

/**
 * Specifies the timed lockable protocol.
 *
 * @see https://en.cppreference.com/w/cpp/named_req/TimedLockable
 */
template <class T>
concept TimedLockable = requires(
    T &t, std::chrono::duration<int> duration,
    std::chrono::time_point<std::chrono::system_clock> time)
{
    { t.try_lock_for(duration) } -> BooleanTestable;
    { t.try_lock_until(time) } -> BooleanTestable;
};

template <class T>
concept StringViewLike =
    std::is_convertible_v<const T &, std::string_view> &&
    !std::is_convertible_v<const T &, const char *>;

} // namespace htl

#endif
