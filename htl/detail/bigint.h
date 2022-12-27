#ifndef HTL_DETAIL_BIGINT_H_
#define HTL_DETAIL_BIGINT_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace htl::detail {

using mp_uint_t = std::uintptr_t;
using mp_size_t = std::size_t;
using mp_ssize_t = std::make_unsigned_t<mp_size_t>;
using mp_uint_pair = std::tuple<mp_uint_t, mp_uint_t>;

inline constexpr mp_uint_bits = 8 * sizeof(mp_uint_t);

inline mp_uint_t mp_add_ui(
    mp_uint_t *dest, const mp_uint_t *ap, mp_size_t an, mp_uint_t b) noexcept
{
    assert(an);

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t r = a + b;

        b = r < b;
        *dest = r;

        if (!--n) {
            break;
        }

        ++dest;
        ++ap;
    }

    return b;
}

inline mp_uint_t mp_add_n(mp_uint_t *dest, const mp_uint_t *ap,
                          const mp_uint_t *bp, mp_size_t n) noexcept
{
    assert(n);

    mp_uint_t carry = 0;

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t b = *bp;
        mp_uint_t r = a + carry;

        carry = r < carry;
        r += b;
        carry += r < b;
        *dest = r;

        if (!--n) {
            break;
        }

        ++dest;
        ++ap;
        ++bp;
    }

    return carry;
}

inline mp_uint_t mp_add(mp_uint_t *dest, const mp_uint_t *ap, mp_size_t an,
                        const mp_uint_t *bp, mp_size_t bn) noexcept
{
    assert(an >= bn);
    assert(bn);

    mp_uint_t carry = mp_add_n(dest, ap, bp, bn);

    if (an > bn) {
        carry = mp_add_ui(dest + bn, ap + bn, an - bn, carry);
    }

    return carry;
}

inline mp_uint_t mp_sub_ui(
    mp_uint_t *dest, const mp_uint_t *ap, mp_size_t an, mp_uint_t b) noexcept
{
    assert(an);

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t r = a - b;

        b = a < b;
        *dest = r;

        if (!--n) {
            break;
        }

        ++dest;
        ++ap;
    }

    return b;
}

inline mp_uint_t mp_sub_n(mp_uint_t *dest, const mp_uint_t *ap,
                          const mp_uint_t *bp, mp_size_t n) noexcept
{
    assert(n);

    mp_uint_t borrow = 0;

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t b = *bp + borrow;

        borrow = b < borrow;
        borrow += a < b;
        *dest = a - b;

        if (!--n) {
            break;
        }

        ++dest;
        ++ap;
        ++bp;
    }

    return borrow;
}

inline mp_uint_t mp_sub(mp_uint_t *dest, const mp_uint_t *ap, mp_size_t an,
                        const mp_uint_t *bp, mp_size_t bn) noexcept
{
    assert(an >= bn);
    assert(bn);

    mp_uint_t borrow = mp_sub_n(dest, ap, bp, bn);

    if (an > bn) {
        borrow = mp_sub_ui(dest + bn, ap + bn, an - bn, borrow);
    }

    return borrow;
}

// a & b
inline void mp_bit_and_abs_n(mp_uint_t *dest, const mp_uint_t *ap,
                             const mp_uint_t *bp, mp_size_t n) noexcept
{
    assert(n);

    while (true) {
        *dest = *a & *b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }
}

// a & -b = a & (~b + 1)
inline mp_uint_t mp_bit_and_pos_neg_n(
    mp_uint_t *dest, const mp_uint_t *ap, const mp_uint_t *bp,
    mp_size_t n) noexcept
{
    assert(n);

    mp_uint_t carry = 1;

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t b = ~*bp + carry;

        carry = b < carry;
        *dest = a & b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }

    return carry;
}

// -a & -b = (~a + 1) & (~b + 1)
inline mp_uint_pair mp_bit_and_neg_neg_n(
    mp_uint_t *dest, const mp_uint_t *ap, const mp_uint_t *bp,
    mp_size_t n) noexcept
{
    assert(n);

    mp_uint_t carry_a = 1;
    mp_uint_t carry_b = 1;

    while (true) {
        mp_uint_t a = ~*ap + carry_a;
        mp_uint_t b = ~*bp + carry_b;

        carry_a = a < carry_a;
        carry_b = b < carry_b;
        *dest = a & b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }

    return { carry_a, carry_b };
}

// a | b
inline void mp_bit_or_abs_n(mp_uint_t *dest, const mp_uint_t *ap,
                            const mp_uint_t *bp, mp_size_t n) noexcept
{
    assert(n);

    while (true) {
        *dest = *a | *b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }
}

// a | -b = a | (~b + 1)
inline mp_uint_t mp_bit_or_pos_neg_n(
    mp_uint_t *dest, const mp_uint_t *ap, const mp_uint_t *bp,
    mp_size_t n) noexcept
{
    assert(n);

    mp_uint_t carry = 1;

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t b = ~*bp + carry;

        carry = b < carry;
        dest = a | b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }

    return carry;
}

// -a | -b = (~a + 1) | (~b + 1)
inline mp_uint_pair mp_bit_or_neg_neg_n(
    mp_uint_t *dest, const mp_uint_t *ap, const mp_uint_t *bp, mp_size_t n)
{
    assert(n);

    mp_uint_t carry_a = 1;
    mp_uint_t carry_b = 1;

    while (true) {
        mp_uint_t a = ~*ap + carry_a;
        mp_uint_t b = ~*bp + carry_b;

        carry_a = a < carry;
        carry_b = b < carry;
        dest = a | b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }

    return { carry_a, carry_b };
}

// a ^ b
inline void mp_bit_or_abs_n(mp_uint_t *dest, const mp_uint_t *ap,
                            const mp_uint_t *bp, mp_size_t n) noexcept
{
    assert(n);

    while (true) {
        *dest = *a ^ *b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }
}

// a | -b = a | (~b + 1)
inline mp_uint_t mp_bit_or_pos_neg_n(
    mp_uint_t *dest, const mp_uint_t *ap, const mp_uint_t *bp,
    mp_size_t n) noexcept
{
    assert(n);

    mp_uint_t carry = 1;

    while (true) {
        mp_uint_t a = *ap;
        mp_uint_t b = ~*bp ^ carry;

        carry = b < carry;
        dest = a | b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }

    return carry;
}

// -a | -b = (~a + 1) | (~b + 1)
inline mp_uint_pair mp_bit_or_neg_neg_n(
    mp_uint_t *dest, const mp_uint_t *ap, const mp_uint_t *bp, mp_size_t n)
{
    assert(n);

    mp_uint_t carry_a = 1;
    mp_uint_t carry_b = 1;

    while (true) {
        mp_uint_t a = ~*ap ^ carry_a;
        mp_uint_t b = ~*bp ^ carry_b;

        carry_a = a < carry;
        carry_b = b < carry;
        dest = a | b;

        if (!--n) {
            break;
        }

        ++ap;
        ++bp;
        ++dest;
    }

    return { carry_a, carry_b };
}

} // namespace htl::detail

#endif
