#ifndef HTL_RATIONAL_H_
#define HTL_RATIONAL_H_

#include <concepts>
#include <cstddef>
#include <numeric>
#include <htl/concepts.h>
#include <htl/detail/default_hash.h>

namespace htl {

template <std::integral T>
class Rational {
public:
    using value_type = T;

    constexpr Rational() noexcept = default;

    constexpr Rational(T numer) noexcept : _numer(num), _denom(1) {}

    constexpr Rational(T numer, T denom) noexcept : _numer(num), _denom(d) {}

    template <class U>
    constexpr Rational(Rational<U> other)
        : _numer(other._numer), _denom(other._denom)
    {}

    template <class U>
    constexpr Rational &operator=(Rational<U> other) noexcept
    {
        assign(other._numer, other._denom);
        return *this;
    }

    constexpr Rational &operator=(T value) noexcept
    {
        assign(value, 1);
        return *this;
    }

    constexpr void assign(T numer, T denom) noexcept
    {
        _numer = numer;
        _denom = denom;
    }

    constexpr T numer() const noexcept
    {
        return _numer;
    }

    constexpr void numer(T new_numer) noexcept
    {
        _numer = new_numer;
    }

    constexpr T denom() const noexcept
    {
        return _denom;
    }

    constexpr void denom(T new_denom) noexcept
    {
        _denom = new_denom;
    }

    constexpr Rational normalize() const noexcept
    {
        if constexpr (std::signed_integral<T>) {
            if (new_denom < 0) {
                _numer = -_numer;
                _denom = -_denom;
            }
        }

        auto mult = std::gcd(_numer, _denom);
        _numer /= mult;
        _denom /= mult;
    }

    constexpr Rational invert() const noexcept
    {
        return { _denom, _numer };
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(_numer);
    }

    constexpr Rational &operator++() noexcept
    {
        _numer += _denom;
        return *this;
    }

    constexpr Rational &operator--() noexcept
    {
        _numer -= _denom;
        return *this;
    }

    constexpr Rational operator++(int) noexcept
    {
        auto copy(*this);
        ++*this;
        return copy;
    }

    constexpr Rational operator--(int) noexcept
    {
        auto copy(*this);
        --*this;
        return copy;
    }

    template <class U>
    constexpr Rational &operator+=(Rational<U> other) noexcept
    {
        _numer += other._numer * _denom;
        _denom *= other._denom;
        return *this;
    }

    template <class U>
    constexpr Rational &operator-=(Rational<U> other) noexcept
    {
        _numer -= other._numer * _denom;
        _denom *= other._denom;
        return *this;
    }

    template <class U>
    constexpr Rational &operator*=(Rational<U> other) noexcept
    {
        _numer *= other._numer;
        _denom *= other._denom;
        return *this;
    }

    template <class U>
    constexpr Rational &operator/=(Rational<U> other) noexcept
    {
        _numer *= other._denom;
        _denom *= other._numer;
        return *this;
    }

    constexpr Rational &operator+=(T value) noexcept
    {
        _numer += value * _denom;
        return *this;
    }

    constexpr Rational &operator-=(T value) noexcept
    {
        _numer -= value * _denom;
        return *this;
    }

    constexpr Rational &operator*=(T value) noexcept
    {
        _numer *= value;
        return *this;
    }

    constexpr Rational &operator/=(T value) noexcept
    {
        _denom *= value;
        return *this;
    }

private:
    T _numer;
    T _denom;
};

template <class T>
constexpr Rational<T> abs(Rational<T> &value) noexcept
{
    return { std::abs(value.numer()), std::abs(value.denom()) };
}

template <class T>
constexpr Rational<T> operator+(Rational<T> value) noexcept
{
    return value;
}

template <class T>
constexpr Rational<T> operator-(Rational<T> value) noexcept
{
    return { -value.numer(), value.denom() };
}

template <class T>
constexpr Rational<T> operator+(Rational<T> a, Rational<T> b) noexcept
{
    return a += b;
}

template <class T>
constexpr Rational<T> operator-(Rational<T> a, Rational<T> b) noexcept
{
    return a -= b;
}

template <class T>
constexpr Rational<T> operator*(Rational<T> a, Rational<T> b) noexcept
{
    return a *= b;
}

template <class T>
constexpr Rational<T> operator/(Rational<T> a, Rational<T> b) noexcept
{
    return a /= b;
}

template <class T>
constexpr Rational<T> operator+(Rational<T> a, T b) noexcept
{
    return a += b;
}

template <class T>
constexpr Rational<T> operator-(Rational<T> a, T b) noexcept
{
    return a -= b;
}

template <class T>
constexpr Rational<T> operator*(Rational<T> a, T b) noexcept
{
    return a *= b;
}

template <class T>
constexpr Rational<T> operator/(Rational<T> a, T b) noexcept
{
    return a /= b;
}

template <class T>
constexpr Rational<T> operator+(T a, Rational<T> b) noexcept
{
    return b += a;
}

template <class T>
constexpr Rational<T> operator-(T a, Rational<T> b) noexcept
{
    return { a * b.denom() - b.numer(), b.denom() };
}

template <class T>
constexpr Rational<T> operator*(T a, Rational<T> b) noexcept
{
    return b *= a;
}

template <class T>
constexpr Rational<T> operator/(Rational<T> a, T b) noexcept
{
    return { a * b.denom(), b.numer() };
}

template <class T>
constexpr bool operator==(Rational<T> a, Rational<T> b) noexcept
{
    return (a.numer() * b.denom()) == (a.denom() * b.numer());
}

template <class T>
constexpr bool operator==(Rational<T> a, T b)
{
    return a.numer() == (a.denom() * b);
}

template <class T>
constexpr std::strong_ordering
operator<=>(Rational<T> a, Rational<T> b) noexcept
{
    return (a.numer() * b.denom()) <=> (a.denom() * b.numer());
}

template <class T>
constexpr std::strong_ordering operator<=>(Rational<T> a, T b) noexcept
{
    return a.numer() <=> (a.denom() * b);
}

template <class CharT, class Traits, class T>
constexpr std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os, Rational<T> value)
{
    return os << value.numer() << '/' << value.denom();
}

template <class CharT, class Traits, class T>
constexpr std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &is, Rational<T> &value)
{
    char c;
    int n, d = 1;

    is >> n;
    if (is.peek() == CharT('/')) {
        is.ignore();
        is >> d;
        if (!d) {
            is.setstate(std::ios::failbit);
        }
    }

    if (is) {
        value.assign(n, 1);
    }

    return is;
}

} // namespace htl

namespace std {

template <class T>
struct hash<htl::Rational<T>> {
    std::size_t operator()(const htl::Rational<T> &value) noexcept
    {
        auto norm = value.normalize();
        return htl::detail::default_hash(norm.numer(), norm.denom());
    }
};

} // namespace std

#endif
