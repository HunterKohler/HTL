#ifndef HTL_RATIONAL_H_
#define HTL_RATIONAL_H_

#include <charconv>
#include <concepts>
#include <cstddef>
#include <limits>
#include <numeric>
#include <htl/concepts.h>
#include <htl/detail/default_hash.h>

namespace htl {

template <std::integral T>
class Rational {
public:
    using value_type = T;

    constexpr Rational() noexcept : _numer(), _denom(1) {}

    constexpr Rational(T numer) noexcept : _numer(numer), _denom(1) {}

    constexpr Rational(T numer, T denom) noexcept : _numer(numer), _denom(denom)
    {}

    template <class U>
    constexpr Rational(Rational<U> other)
        : _numer(other.numer()), _denom(other.denom())
    {}

    template <class U>
    constexpr Rational &operator=(Rational<U> other) noexcept
    {
        assign(other.numer(), other.denom());
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
        auto new_numer = _numer;
        auto new_denom = _denom;

        if constexpr (std::signed_integral<T>) {
            if (_denom < 0) {
                new_numer = -new_numer;
                new_denom = -new_denom;
            }
        }

        auto mult = std::gcd(new_numer, new_denom);

        new_numer /= mult;
        new_denom /= mult;

        return { new_numer, new_denom };
    }

    constexpr Rational invert() const noexcept
    {
        return { _denom, _numer };
    }

    constexpr Rational abs() const noexcept
    {
        return { std::abs(numer()), std::abs(denom()) };
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
        _numer = _numer * other.denom() + other.numer() * _denom;
        _denom *= other.denom();
        return *this;
    }

    template <class U>
    constexpr Rational &operator-=(Rational<U> other) noexcept
    {
        _numer = _numer * other.denom() - other.numer() * _denom;
        _denom *= other.denom();
        return *this;
    }

    template <class U>
    constexpr Rational &operator*=(Rational<U> other) noexcept
    {
        _numer *= other.numer();
        _denom *= other.denom();
        return *this;
    }

    template <class U>
    constexpr Rational &operator/=(Rational<U> other) noexcept
    {
        _numer *= other.denom();
        _denom *= other.numer();
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
constexpr Rational<T> operator/(T a, Rational<T> b) noexcept
{
    return { a * b.denom(), b.numer() };
}

template <class T, class U>
constexpr bool operator==(Rational<T> a, Rational<U> b) noexcept
{
    return (a.numer() * b.denom()) == (a.denom() * b.numer());
}

template <class T>
constexpr bool operator==(Rational<T> a, T b)
{
    return a.numer() == (a.denom() * b);
}

template <class T, class U>
constexpr std::strong_ordering
operator<=>(Rational<T> a, Rational<U> b) noexcept
{
    a = a.normalize();
    b = b.normalize();
    return (a.numer() * b.denom()) <=> (a.denom() * b.numer());
}

template <class T>
constexpr std::strong_ordering operator<=>(Rational<T> a, T b) noexcept
{
    a = a.normalize();
    return a.numer() <=> (a.denom() * b);
}

template <class T, std::output_iterator<char> O>
constexpr O to_chars(const Rational<T> &value, O out)
{
    char buf[2 * std::numeric_limits<T>::digits10 + 5];
    auto res = std::to_chars(buf, std::end(buf), value.numer());
    *res.ptr++ = '/';
    res = std::to_chars(res.ptr, std::end(buf), value.denom());
    return std::copy(buf, res.ptr, std::move(out));
}

template <class T, class Alloc = std::allocator<char>>
constexpr std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const Rational<T> &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res));
    return res;
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
    int n, d = 1;

    is >> n;
    if (Traits::eq(CharT('/'), Traits::to_char_type(is.peek()))) {
        is.ignore();
        is >> d;
        if (!d) {
            is.setstate(std::ios::failbit);
        }
    }

    if (is) {
        value.assign(n, d);
    }

    return is;
}

} // namespace htl

namespace std {

template <class T>
struct hash<htl::Rational<T>> {
    std::size_t operator()(const htl::Rational<T> &value) noexcept
    {
        auto n = value.normalize();
        return htl::detail::default_hash(n.numer(), n.denom());
    }
};

} // namespace std

#endif
