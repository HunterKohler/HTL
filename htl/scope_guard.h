#ifndef HTL_SCOPE_GUARD_H_
#define HTL_SCOPE_GUARD_H_

#include <cerrno>
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace htl {

/**
 * Runs the held invocable on destruction for scoped cleanup.
 *
 * Note that the destructor of `ScopeGuard` will not itself be `noexcept` if
 * the invocable is not `noexcept`. Of course, exceptions generally should not
 * be thrown in the destructor, so avoid that.
 *
 * The scope guard can be dismissed, and the operation elided with the
 * `dismiss()` method. Dismissal is idempotent.
 *
 * The guard itself should always be placed on the stack, not allocated, and
 * is a movable, non-copyable type. Generally its usage is tightly coupled as
 * cleanup for some local task within a function, and should not be passed
 * around many times.
 *
 * @tparam F The type of invocable to hold. Must be held by value.
 */
template <class F>
    requires(std::is_invocable_v<F> && std::is_same_v<std::decay_t<F>, F>)
class ScopeGuard {
public:
    /**
     * Default constructor.
     *
     * Value construct's the invocable if it is default constructible, and
     * not function pointer.
     */
    constexpr ScopeGuard() //
        noexcept(std::is_nothrow_default_constructible_v<F>) //
        requires(std::is_default_constructible_v<F> && !std::is_function_v<F>)
        : _active(true), _f()
    {}

    /**
     * Converting constructor.
     *
     * Initializes contained invocable as if by direct-initialization from the
     * argument if possible.
     */
    template <class U = F>
        requires(std::is_constructible_v<F, U>)
    constexpr explicit ScopeGuard(U &&u) //
        noexcept(std::is_nothrow_constructible_v<F, U>) //
        : _active(true), _f(std::forward<U>(u))
    {}

    /**
     * Converting move constructor.
     *
     * Initializes contained invocable as if by direct-initialization from the
     * held value of the other scope guard. The other scope guard will be
     * dismissed.
     *
     * @tparam U Invocable type of the other scope guard.
     */
    template <class U>
        requires(std::is_nothrow_constructible_v<F, U>)
    constexpr explicit(std::is_convertible_v<U, F>)
        ScopeGuard(ScopeGuard<U> &&other) noexcept
        : _active(std::exchange(other._active, false)),
          _f(std::move(other._active))
    {}

    /**
     * In-place constructor.
     *
     * Initializes the guard's invocable as if by direct initialization using
     * the arguments.
     */
    template <class... Args>
        requires(std::is_constructible_v<F, Args...>)
    constexpr explicit ScopeGuard(std::in_place_t, Args &&...args) //
        noexcept(std::is_nothrow_constructible_v<F, Args...>)
        : _active(true), _f(std::forward<Args>(args)...)
    {}

    /**
     * In-place list constructor.
     *
     * Initializes the guard's invocable as if by direct initialization using
     * the initializer list and arguments.
     */
    template <class U, class... Args>
        requires(std::is_constructible_v<
                    F, std::initializer_list<U> &, Args...>)
    constexpr ScopeGuard(
        std::in_place_t, std::initializer_list<U> ilist, Args &&...args) //
        noexcept(std::is_nothrow_constructible_v<
                 F, std::initializer_list<U> &, Args...>)
        : _active(true), _f(ilist, std::forward<Args>(args)...)
    {}

    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard &operator=(const ScopeGuard &) = delete;

    constexpr ScopeGuard(ScopeGuard &&other) noexcept
        requires(std::is_nothrow_move_constructible_v<F>)
        : _active(std::exchange(other._active, false)), _f(std::move(other._f))
    {}

    constexpr ScopeGuard &operator=(ScopeGuard &&other) noexcept
        requires(std::is_nothrow_move_assignable_v<F>)
    {
        _active = std::exchange(other._active, false);
        _f = std::move(other._active);
        return *this;
    }

    /**
     * Runs and destroys the contained invocable if active.
     */
    constexpr ~ScopeGuard() noexcept(std::is_nothrow_invocable_v<F>)
    {
        if (_active) {
            std::invoke(_f);
        }
    }

    /**
     * Dismisses the scope guard, so that it will not run the contained
     * invocable on destruction.
     */
    constexpr void dismiss() noexcept
    {
        _active = false;
    }

    template <class... Args>
    void *operator new(std::size_t, Args &&...args) = delete;

    template <class... Args>
    void operator delete(void *, Args &&...args) = delete;

private:
    bool _active;
    [[no_unique_address]] F _f;
};

template <class F>
ScopeGuard(F) -> ScopeGuard<F>;

/**
 * A scope guard that saves and restores the global `errno` value within the
 * scope.
 */
class ErrnoScopeGuard {
public:
    /**
     * Default constructor.
     *
     * Saves the current value of `errno`, and sets `errno` to zero.
     */
    ErrnoScopeGuard() noexcept : _active(true), _value(std::exchange(errno, 0))
    {}

    /**
     * Move constructor.
     *
     * Take's the other guard's errno value and dismissal status. The other
     * guard will be dismissed, if not already so.
     */
    ErrnoScopeGuard(ErrnoScopeGuard &&other) noexcept
        : _active(std::exchange(other._active, false)), _value(other._value)
    {}

    /**
     * Move assignment.
     *
     * Take's the other guard's errno value and dismissal status. The other
     * guard will be dismissed, if not already so, unless the guard is being
     * assigned to itself.
     */
    ErrnoScopeGuard &operator=(ErrnoScopeGuard &&other) noexcept
    {
        if (this != std::addressof(other)) {
            _active = std::exchange(other._active, false);
            _value = other._value;
        }

        return *this;
    }

    /**
     * Sets `errno` to the contained value if the scope guard has not been
     * dismissed.
     */
    ~ErrnoScopeGuard() noexcept
    {
        if (_active) {
            errno = _value;
        }
    }

    /**
     * Dismisses the scope guard. The value of `errno` will not be set at the
     * end of the scope.
     */
    void dismiss() noexcept
    {
        _active = false;
    }

    /**
     * Returns the contained value of `errno`.
     */
    int value() const noexcept
    {
        return _value;
    }

private:
    bool _active;
    int _value;
};

} // namespace htl

#endif
