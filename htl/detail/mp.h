#ifndef HTL_DETAIL_MP_H_
#define HTL_DETAIL_MP_H_

#include <cassert>
#include <cfenv>
#include <climits>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <utility>
#include <htl/config.h>
#include <htl/detail/default_hash.h>
#include <htl/mpfwd.h>

namespace htl::mp::detail {

#ifdef HTL_ARCH_X86_64
using mp_int = std::uint64_t;
using mp_uint = std::uint64_t;
#elif HTL_ARCH_X86
using mp_int = std::int32_t;
using mp_uint = std::int32_t;
#else
using mp_int = unsigned long;
using mp_uint = unsigned long;
#endif

using mp_size = std::size_t;
using mp_ssize = std::make_signed_t<mp_size>;

template <class T>
concept SingleWordIntegral =
    (std::numeric_limits<T>::is_integer && std::numeric_limits<T>::radix == 2 &&
     std::numeric_limits<T>::is_bounded &&
     std::numeric_limits<T>::digits <=
         std::numeric_limits<std::conditional_t<
             std::numeric_limits<T>::is_signed, mp_int, mp_uint>>::digits);

inline constexpr mp_size mp_uint_bytes = sizeof(mp_uint);
inline constexpr mp_size mp_uint_bits = std::numeric_limits<mp_uint>::digits;
inline constexpr mp_size mp_uint_half_bits = mp_uint_bits >> 1;
inline constexpr mp_uint mp_uint_mask = static_cast<mp_uint>(-1);
inline constexpr mp_uint mp_uint_low_mask = mp_uint_mask >> mp_uint_half_bits;
inline constexpr mp_uint mp_uint_high_mask = mp_uint_mask >> mp_uint_half_bits;
inline constexpr mp_uint mp_uint_high_bit = mp_uint_mask << (mp_uint_bits - 1);

inline void mp_uint_add2x2(mp_uint *r1, mp_uint *r0, mp_uint a1, mp_uint a0,
                           mp_uint b1, mp_uint b0) noexcept
{
#if HTL_INLINE_ASM && (HTL_ARCH_X86_64 || HTL_ARCH_X86)
    asm("add %[b0], %[r0] \n\t"
        "adc %[b1], %[r1] \n\t"
        : [r1] "=rm"(*r1), [r0] "=rm"(r0)
        : "[r1]"(a1), "[r0]"(a0), [b1] "g"(b1), [b0] "g"(b0)
        : "cc");
#else
    a0 += b0;
    *r0 = a0;
    *r1 = a1 + b1 + (b0 < a0);
#endif
}

inline void mp_uint_sub2x2(mp_uint *r1, mp_uint *r0, mp_uint a1, mp_uint a0,
                           mp_uint b1, mp_uint b0) noexcept
{
#if HTL_INLINE_ASM && (HTL_ARCH_X86_64 || HTL_ARCH_X86)
    asm("sub %[b0], %[r0] \n\t"
        "sbb %[b1], %[r1] \n\t"
        : [r1] "=rm"(*r1), [r0] "=rm"(r0)
        : "[r1]"(a1), "[r0]"(a0), [b1] "g"(b1), [b0] "g"(b0)
        : "cc");
#else
    *r0 = a0 - b0 - (a1 < b1);
    *r1 = a1 - b1;
#endif
}

inline void mp_uint_mul1x1(
    mp_uint *r1, mp_uint *r0, mp_uint a, mp_uint b) noexcept
{
#if HTL_INLINE_ASM && (HTL_ARCH_X86_64 || HTL_ARCH_X86)
    asm("mul %[b]" : "=d"(*r1), "=a"(*r0) : "a"(a), [b] "rm"(b) : "cc");
#else
    mp_uint a0 = a & mp_uint_low_mask;
    mp_uint a1 = a >> mp_uint_half_bits;
    mp_uint b0 = b & mp_uint_low_mask;
    mp_uint b1 = b >> mp_uint_half_bits;
    mp_uint x0 = a0 * b0;
    mp_uint x1 = a0 * b1;
    mp_uint x2 = a1 * b0;
    mp_uint x3 = a1 * b1;

    x1 += (x0 >> mp_uint_half_bits) + x2;
    x3 += static_cast<mp_uint>(x1 < x2) << mp_uint_half_bits;
    *r0 = (x0 & mp_uint_low_mask) + (x1 << mp_uint_half_bits);
    *r1 = (x1 >> mp_uint_half_bits) + x3;
#endif
}

inline mp_uint mp_uint_mul1x1_high(mp_uint a, mp_uint b) noexcept
{
    mp_uint high;

#if HTL_INLINE_ASM && (HTL_ARCH_X86_64 || HTL_ARCH_X86)
    asm("mul %[b]" : "=d"(high) : "a"(a), [b] "rm"(b) : "cc");
#else
    mp_uint low;
    mp_uint_mul(&high, &low, a, b);
#endif

    return high;
}

inline mp_uint mp_uint_mul1x1_low(mp_uint a, mp_uint b) noexcept
{
    mp_uint low;

#if HTL_INLINE_ASM && (HTL_ARCH_X86_64 || HTL_ARCH_X86)
    asm("mul %[b]" : "=a"(lo) : "a"(a), [b] "rm"(b) : "cc");
#else
    mp_uint high;
    mp_uint_mul(&high, &low, a, b);
#endif

    return low;
}

inline void mp_uint_div2x1(
    mp_uint *quot, mp_uint *rem, mp_uint n1, mp_uint n0, mp_uint d) noexcept
{
    assert(n1 < d);
    assert(d & mp_uint_high_bit);

#if HTL_INLINE_ASM && (HTL_ARCH_X86_64 || HTL_ARCH_X86)
    asm("div %[d]"
        : "=a"(*quot), "=d"(*rem)
        : "a"(n1), "d"(n0), [d] "ri"(d)
        : "cc");
#else
    // Algorithm 1 from https://gmplib.org/~tege/division-paper.pdf

    mp_uint q = n1 + mp_uint_mul2x2_hi(n1, d);
    mp_uint r1, r0;

    mp_uint_mul2x2(&r1, &r0, q, d);
    mp_uint_sub2x2(&r1, &r0, n1, n0, r1, r0);

    while (r1 > 0 || r0 >= d) {
        ++q;
        mp_uint_sub2x2(&r1, &r0, r1, r0, 0, d);
    }

    *quot = q;
    *rem = r0;
#endif
}

// inline mp_uint mp_uint_invert(mp_uint value) noexcept
// {
//     assert(value);

//     mp_uint q, r;
//     mp_uint_div(&q, &r, ~value, mp_uint_mask, value);
//     return q;
// }

inline mp_uint mp_add_1(
    mp_uint *rp, const mp_uint *ap, mp_size an, mp_uint b) noexcept
{
    assert(an);

    do {
        mp_uint a = *ap++;
        mp_uint r = a + b;

        b = r < b;
        *rp++ = r;
    } while (--an);

    return b;
}

inline mp_uint mp_add_n(
    mp_uint *rp, const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);
    mp_uint carry = 0;

    do {
        mp_uint a = *ap++;
        mp_uint b = *bp++ + carry;
        mp_uint r = a + b;

        carry = (b < carry) + (r < b);
        *rp++ = r;
    } while (--n);

    return carry;
}

inline mp_uint mp_add(mp_uint *rp, const mp_uint *ap, mp_size an,
                      const mp_uint *bp, mp_size bn) noexcept
{
    assert(an >= bn);
    assert(bn);

    mp_uint carry = mp_add_n(rp, ap, bp, bn);

    if (an > bn) {
        carry = mp_add_1(rp + bn, ap + bn, an - bn, carry);
    }

    return carry;
}

inline mp_uint mp_sub_1(
    mp_uint *rp, const mp_uint *ap, mp_size an, mp_uint b) noexcept
{
    assert(an);

    do {
        mp_uint a = *ap++;
        mp_uint r = a - b;

        b = a < b;
        *rp++ = r;
    } while (--an);

    return b;
}

inline mp_uint mp_sub_n(
    mp_uint *rp, const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);
    mp_uint borrow = 0;

    do {
        mp_uint a = *ap++;
        mp_uint b = *bp++ + borrow;
        mp_uint r = a - b;

        borrow = (b < borrow) + (a < b);
        *rp++ = r;
    } while (--n);

    return borrow;
}

inline mp_uint mp_sub(mp_uint *rp, const mp_uint *ap, mp_size an,
                      const mp_uint *bp, mp_size bn) noexcept
{
    assert(an >= bn);
    assert(bn);

    mp_uint borrow = mp_sub_n(rp, ap, bp, bn);

    if (an > bn) {
        borrow = mp_sub_1(rp + bn, ap + bn, an - bn, borrow);
    }

    return borrow;
}

inline mp_uint mp_addmul_1(
    mp_uint *rp, const mp_uint *ap, mp_size an, mp_uint b) noexcept
{
    assert(an);
    mp_uint carry = 0;

    do {
        mp_uint hi, lo;
        mp_uint a = *ap++;
        mp_uint r = *rp;

        mp_uint_mul(&hi, &lo, a, b);
        lo += carry;
        r += lo;
        carry = hi + (lo < carry) + (r < lo);
        *rp++ = r;
    } while (--an);

    return carry;
}

inline mp_uint mp_mul_1(
    mp_uint *rp, const mp_uint *ap, mp_size an, mp_uint b) noexcept
{
    assert(an);
    mp_uint carry = 0;

    do {
        mp_uint hi, lo;
        mp_uint a = *ap++;

        mp_uint_mul(&hi, &lo, a, b);
        lo += carry;
        carry = hi + (lo < carry);
        *rp++ = lo;
    } while (--an);

    return carry;
}

inline void mp_mul(mp_uint *rp, const mp_uint *ap, mp_size an,
                   const mp_uint *bp, mp_size bn) noexcept
{
    assert(an >= bn);
    assert(bn);

    rp[an] = mp_mul_1(rp, ap, an, *bp);

    while (--bn) {
        ++rp;
        ++bp;
        rp[an] = mp_addmul_1(rp, ap, an, *bp);
    }
}

inline void mp_bit_and_n(
    mp_uint *rp, const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);

    do {
        *rp++ = *ap++ & *bp++;
    } while (--n);
}

inline void mp_bit_or_n(
    mp_uint *rp, const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);

    do {
        *rp++ = *ap++ | *bp++;
    } while (--n);
}

inline void mp_bit_xor_n(
    mp_uint *rp, const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);

    do {
        *rp++ = *ap++ ^ *bp++;
    } while (--n);
}

inline std::strong_ordering mp_cmp_1(
    const mp_uint *ap, mp_size an, mp_uint b) noexcept
{
    assert(an);
    return an == 1 ? *ap <=> b : std::strong_ordering::greater;
}

inline std::strong_ordering mp_cmp_n(
    const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);

    do {
        mp_uint a = ap[--n];
        mp_uint b = bp[n];

        if (a != b) {
            return a <=> b;
        }
    } while (n);

    return std::strong_ordering::equal;
}

inline std::strong_ordering mp_cmp(
    const mp_uint *ap, mp_size an, const mp_uint *bp, mp_size bn) noexcept
{
    assert(an);
    assert(bn);

    if (an != bn) {
        return an <=> bn;
    }

    do {
        if (*ap != *bp) {
            return *ap <=> *bp;
        }

        ++ap;
        ++bp;
    } while (--an);

    return std::strong_ordering::equal;
}

inline bool mp_equal_n(const mp_uint *ap, const mp_uint *bp, mp_size n) noexcept
{
    assert(n);

    do {
        if (*ap++ != *bp++) {
            return false;
        }
    } while (--n);

    return true;
}

template <std::integral T>
constexpr std::make_unsigned_t<T> mp_abs(T value) noexcept
{
    if constexpr (std::unsigned_integral<T>) {
        return value;
    } else {
        return std::abs(value);
    }
}

template <std::integral T>
constexpr std::make_signed_t<T> mp_negate(T value) noexcept
{
    return -static_cast<std::make_signed_t<T>>(value);
}

constexpr bool mp_same_sign(std::integral auto a, std::integral auto b) noexcept
{
    return a < 0 == b < 0;
}

template <class Alloc>
struct BigIntImpl {
public:
    Alloc alloc;
    mp_ssize ssize;
    mp_size capacity;
    mp_uint *data;

    BigIntImpl() noexcept(noexcept(Alloc()))
        : alloc(), ssize(), capacity(), data()
    {}

    explicit BigIntImpl(const Alloc &alloc) noexcept
        : alloc(alloc), ssize(), capacity(), data()
    {}

    BigIntImpl(std::integral auto value, const Alloc &alloc) : BigIntImpl(alloc)
    {
        assign(value);
    }

    BigIntImpl(std::floating_point auto value, const Alloc &alloc)
        : BigIntImpl(alloc)
    {
        assign(value);
    }

    BigIntImpl(const BigIntImpl &other)
        : BigIntImpl(other, other.select_allocator_on_copy())
    {}

    BigIntImpl(const BigIntImpl &other, const Alloc &alloc) : BigIntImpl(alloc)
    {
        copy_construct(other);
    }

    BigIntImpl(BigIntImpl &&other) noexcept : alloc(std::move(other.alloc))
    {
        move_data(std::move(other));
    }

    BigIntImpl(BigIntImpl &&other, const Alloc &alloc) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value)
        : alloc(alloc)
    {
        if (std::allocator_traits<Alloc>::is_always_equal::value ||
            this->alloc == other.alloc) {
            move_data(std::move(other));
        } else {
            copy_construct(other);
        }
    }

    ~BigIntImpl() noexcept
    {
        destroy();
    }

    BigIntImpl &operator=(const BigIntImpl &other)
    {
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;
        constexpr bool propagate = std::allocator_traits<
            Alloc>::propagate_on_container_copy_assignment::value;

        mp_size n = other.get_size();

        if (this == std::addressof(other)) {
            return *this;
        } else if (n > capacity ||
                   (propagate && !(is_always_equal || alloc == other.alloc))) {
            data = propagate ? other.allocate(n) : allocate(n);
            destroy();
            capacity = n;
        }

        if constexpr (propagate) {
            alloc = other.alloc;
        }

        std::copy_n(other.data, n, data);
        ssize = other.ssize;
        return *this;
    }

    BigIntImpl &operator=(BigIntImpl &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<
                     Alloc>::propagate_on_container_move_assignment::value)
    {
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;
        constexpr bool propagate = std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value;

        if (this == std::addressof(other)) {
            return *this;
        } else if (propagate || is_always_equal || alloc == other.alloc) {
            destroy();
            move_data(std::move(other));

            if constexpr (propagate) {
                alloc = std::move(other.alloc);
            }
        } else {
            mp_size n = other.get_size();

            if (n > capacity) {
                mp_uint *new_data = allocate(n);
                destroy();
                data = new_data;
                capacity = n;
            }

            std::copy_n(other.data, n, data);
            ssize = other.ssize;
        }

        return *this;
    }

    static mp_uint *allocate(Alloc &alloc, mp_size n)
    {
        if (!n) {
            return nullptr;
        }

        return std::to_address(
            std::allocator_traits<Alloc>::allocate(alloc, n));
    }

    static void deallocate(Alloc &alloc, mp_uint *p, mp_size n) noexcept
    {
        using pointer = typename std::allocator_traits<Alloc>::pointer;

        if (n) {
            std::allocator_traits<Alloc>::deallocate(
                alloc, std::pointer_traits<pointer>::pointer_to(*p), n);
        }
    }

    mp_uint *allocate(mp_size n)
    {
        return allocate(alloc, n);
    }

    void deallocate(mp_uint *p, mp_size n) noexcept
    {
        deallocate(alloc, p, n);
    }

    void destroy() noexcept
    {
        deallocate(data, capacity);
    }

    mp_size get_size() const noexcept
    {
        return mp_abs(ssize);
    }

    void set_size(mp_size new_size) const noexcept
    {
        ssize = ssize >= 0 ? new_size : mp_negate(new_size);
    }

    mp_size normal_size(mp_size n) const noexcept
    {
        while (n && !data[--n]) {
        }

        return n;
    }

    void assign_data() noexcept
    {
        assign_data(0, 0, nullptr);
    }

    void assign_data(mp_ssize new_ssize, mp_size new_capacity,
                     mp_uint *new_data) noexcept
    {
        ssize = new_ssize;
        capacity = new_capacity;
        data = new_data;
    }

    void move_data(BigIntImpl &&other) noexcept
    {
        assign_data(other.ssize, other.capacity, other.data);
        other.assign_data();
    }

    void copy_construct(const BigIntImpl &other)
    {
        mp_size n = other.get_size();
        assign_data(other.ssize, n, allocate(n));
        std::copy_n(other.data, n, data);
    }

    void reserve(mp_size n)
    {
        if (n > capacity) {
            mp_uint *new_data = allocate(n);
            std::copy_n(data, get_size(), new_data);
            destroy();
            data = new_data;
            capacity = n;
        }
    }

    template <std::integral T>
    void assign(T value)
    {
        if constexpr (SingleWordIntegral<T>) {
            if (value > 0) {
                assign_uint_pos(value);
            } else if (value < 0) {
                assign_uint_neg(-value);
            } else {
                ssize = 0;
            }
        } else {
            bool neg = value < 0;
            if (neg) {
                value = -venue;
            }

            constexpr mp_size max_words =
                (std::numeric_limits<T>::digits + mp_uint_bits - 1) /
                mp_uint_bits;

            reserve(max_words);
            mp_size n = 0;

            while (value) {
                data[n++] = value & mp_uint_mask;
                value >>= mp_uint_bits;
            }

            ssize = neg ? mp_negate(n) : n;
        }
    }

    void assign_uint_pos(mp_uint value)
    {
        reserve(1);
        ssize = 1;
        data[0] = value;
    }

    void assign_uint_neg(mp_uint value)
    {
        reserve(1);
        ssize = -1;
        data[0] = value;
    }

    void assign(std::floating_point auto value);

    void assign(const BigIntImpl &other)
    {
        if (this == std::addressof(other)) {
            return;
        }

        mp_size n = other.get_size();

        if (n > capacity) {
            mp_uint *new_data = allocate(n);
            destroy();
            data = new_data;
            capacity = n;
        }

        ssize = other.ssize;
        std::copy_n(other.data, n, data);
    }

    void assign(BigIntImpl &&other)
    {
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;

        if (this == std::addressof(other)) {
            return;
        } else if (is_always_equal || alloc == other.alloc) {
            move_data(std::move(other));
        } else {
            assign(other);
        }
    }

    Alloc select_allocator_on_copy() const noexcept
    {
        return std::allocator_traits<
            Alloc>::select_on_container_copy_construction(alloc);
    }

    template <std::integral T>
    auto promote_int(T value) const
    {
        if constexpr (SingleWordIntegral<T>) {
            if constexpr (std::signed_integral<T>) {
                return static_cast<mp_int>(value);
            } else {
                return static_cast<mp_uint>(value);
            }
        } else {
            return BigIntImpl(value, alloc);
        }
    }

    std::size_t hash() const noexcept
    {
        return DefaultHasher()
            .update(ssize)
            .update_range(data, get_size())
            .digest();
    }

    void negate() noexcept
    {
        ssize = mp_negate(ssize);
    }

    void abs() noexcept
    {
        ssize = std::abs(ssize);
    }

    void swap(BigIntImpl &other) noexcept
    {
        using std::swap;

        if constexpr (std::allocator_traits<
                          Alloc>::propagate_on_container_swap::value) {
            swap(alloc, other.alloc);
        }

        swap(ssize, other.ssize);
        swap(capacity, other.capacity);
        swap(data, other.data);
    }

    explicit operator bool() const noexcept
    {
        return ssize;
    }

    template <std::integral T>
    explicit operator T() const noexcept
    {
        return ssize > 0 ? data[0] : ssize < 0 ? -data[0] : 0;
    }

    template <std::floating_point T>
    explicit operator T() const noexcept;

    friend bool operator==(const BigIntImpl &a, const BigIntImpl &b) noexcept
    {
        return a.ssize == b.ssize &&
               (!a.ssize || mp_equal_n(a.data, b.data, a.get_size()));
    }

    friend bool operator==(const BigIntImpl &a, std::integral auto b) noexcept
    {
        if (b > 0) {
            return a.ssize == 1 && a.data[0] == b;
        } else if (b < 0) {
            return a.ssize == -1 && a.data[0] == -b;
        } else {
            return !a;
        }
    }

    friend bool operator==(
        const BigIntImpl &a, std::floating_point auto b) noexcept;

    friend std::strong_ordering operator<=>(
        const BigIntImpl &a, const BigIntImpl &b) noexcept
    {
        if (!a.ssize || (a.ssize != b.ssize)) {
            return a.ssize <=> b.ssize;
        } else if (a.ssize > 0) {
            return mp_cmp_n(a.data, b.data, a.get_size());
        } else {
            return 0 <=> mp_cmp_n(a.data, b.data, a.get_size());
        }
    }

    friend std::strong_ordering operator<=>(
        const BigIntImpl &a, std::integral auto b) noexcept
    {
        if (b > 0) {
            if (a.ssize == 1) {
                return a.data[0] <=> b;
            } else {
                return a.ssize <=> 1;
            }
        } else if (b < 0) {
            if (a.ssize == -1) {
                return -b <=> a.data[0];
            } else {
                return a.ssize <=> -1;
            }
        } else {
            return a.ssize <=> 0;
        }
    }

    friend std::strong_ordering operator<=>(
        const BigIntImpl &a, std::floating_point auto b) noexcept;

    friend void add(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (mp_same_sign(a.ssize, b.ssize)) {
            abs_add(a, b, r);
        } else {
            abs_sub(a, b, r);
        }
    }

    friend void add(const BigIntImpl &a, std::integral auto b, BigIntImpl &r)
    {
        if (mp_same_sign(a.ssize, b)) {
            abs_add(a, mp_abs(b), r);
        } else {
            abs_sub(a, mp_abs(b), r);
        }
    }

    friend void sub(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (mp_same_sign(a.ssize, b.ssize)) {
            abs_sub(a, b, r);
        } else {
            abs_add(a, b, r);
        }
    }

    friend void sub(const BigIntImpl &a, std::integral auto b, BigIntImpl &r)
    {
        if (mp_same_sign(a.ssize, b, r)) {
            abs_sub(a, mp_abs(b), r);
        } else {
            abs_add(a, mp_abs(b), r);
        }
    }

    friend void sub(std::integral auto a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (mp_same_sign(a, b.ssize, r)) {
            abs_sub(mp_abs(a), b, r);
        } else {
            abs_add(b, mp_abs(a), r);
        }
    }

    friend void abs_add(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
            return;
        } else if (!b.ssize) {
            r.assign(a);
            return;
        }

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        mp_size rn;

        if (an >= bn) {
            rn = an + 1;
            r.reserve(rn);
            r.data[an] = mp_add(r.data, a.data, an, b.data, bn);
        } else {
            rn = bn + 1;
            r.reserve(rn);
            r.data[an] = mp_add(r.data, b.data, bn, a.data, an);
        }

        rn -= !r.data[an];
        r.ssize = a.ssize >= 0 ? rn : mp_negate(rn);
    }

    friend void abs_add(const BigIntImpl &a, mp_uint b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
            return;
        }

        mp_size an = a.get_size();
        mp_size rn = an + 1;

        r.reserve(rn);
        r.data[an] = mp_add_1(r.data, a.data, an, b);
        rn -= !r.data[an];
        r.ssize = a.ssize >= 0 ? rn : mp_negate(rn);
    }

    friend void abs_sub(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
            r.negate();
            return;
        } else if (!b.ssize) {
            r.assign(a);
            return;
        }

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        std::strong_ordering cmp = mp_cmp(a.data, an, b.data, bn);

        if (cmp > 0) {
            r.reserve(an);
            mp_sub(r.data, a.data, an, b.data, bn);
            mp_size rn = r.normal_size(an);
            r.ssize = a.ssize >= 0 ? rn : mp_negate(rn);
        } else if (cmp < 0) {
            r.reserve(bn);
            mp_sub(r.data, b.data, bn, a.data, an);
            mp_size rn = r.normal_size(bn);
            r.ssize = a.ssize >= 0 ? mp_negate(rn) : rn;
        } else {
            r.ssize = 0;
        }
    }

    friend void abs_sub(const BigIntImpl &a, mp_uint b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.reserve(1);
            r.data[0] = b;
            r.ssize = a.ssize >= 0 ? -1 : 1;
            return;
        }

        mp_size an = a.get_size();
        std::strong_ordering cmp = mp_cmp_1(a.data, an, b);

        if (cmp > 0) {
            r.reserve(an);
            mp_sub_1(r.data, a.data, an, b);
            mp_size rn = r.normal_size(an);
            r.ssize = a.ssize >= 0 ? rn : mp_negate(rn);
        } else if (cmp < 0) {
            r.reserve(1);
            r.data[0] = b - a.data[0];
            r.ssize = a.ssize >= 0 ? -1 : 1;
        } else {
            r.ssize = 0;
        }
    }

    friend void abs_sub(mp_uint a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!b.ssize) {
            r.assign(a);
            return;
        }

        mp_size bn = b.get_size();
        std::strong_ordering cmp = mp_cmp_1(b.data, bn, a);

        if (cmp > 0) {
            r.reserve(1);
            r.data[0] = b.data[0] - a;
            r.ssize = b.ssize >= 0 ? -1 : 1;
        } else if (cmp < 0) {
            r.reserve(1);
            r.data[0] = a - b.data[0];
            r.ssize = b.ssize >= 0 ? 1 : -1;
        } else {
            r.ssize = 0;
        }
    }

    friend void mul(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!a.ssize || !b.ssize) {
            r.ssize = 0;
            return;
        }

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        mp_size tn = an + bn;
        BigIntImpl t(r.select_allocator_on_copy());

        t.reserve(tn);

        if (an >= bn) {
            mp_mul(t.data, a.data, an, b.data, bn);
        } else {
            mp_mul(t.data, b.data, bn, a.data, an);
        }

        tn = t.normal_size(tn);
        t.ssize = mp_same_sign(a.ssize, b.ssize) ? tn : mp_negate(tn);
        r = std::move(t);
    }

    friend void mul(const BigIntImpl &a, std::integral auto b, BigIntImpl &r)
    {
        if (!a.ssize || !b) {
            r.ssize = 0;
            return;
        }

        mp_size an = a.get_size();
        mp_size tn = an + 1;
        BigIntImpl t(r.select_allocator_on_copy());

        t.reserve(tn);
        mp_mul_1(t.data, a.data, an, mp_abs(b));
        tn = t.normal_size(tn);
        t.ssize = mp_same_sign(a.ssize, b) ? tn : mp_negate(tn);
        r = std::move(t);
    }

    friend void div(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r);

    friend void div(const BigIntImpl &a, std::integral auto b, BigIntImpl &r);

    friend void div(std::integral auto a, const BigIntImpl &b, BigIntImpl &r);

    friend void div(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &q,
                    BigIntImpl &r);

    friend void div(const BigIntImpl &a, std::integral auto b, BigIntImpl &q,
                    BigIntImpl &r);

    friend void div(std::integral auto a, const BigIntImpl &b, BigIntImpl &q,
                    BigIntImpl &r);

    friend void rem(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r);

    friend void rem(const BigIntImpl &a, std::integral auto b, BigIntImpl &r);

    friend void rem(std::integral auto a, const BigIntImpl &b, BigIntImpl &r);

    // a & -b = a & ~(b - 1)

    friend void bit_and_pos_neg(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize > 0);
        assert(b.ssize < 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        mp_size rn = std::min(an, bn);
        mp_uint borrow = 1;

        r.reserve(rn);

        for (mp_size i = 0; i < rn; i++) {
            mp_uint av = a.data[i];
            mp_uint bv = b.data[i];
            mp_uint rv = a & ~(b - borrow);

            borrow = b < borrow;
            r.data[i] = rv;
        }

        r.ssize = r.normal_size(rn);
    }

    // -a & -b = ~(a - 1) & ~(b - 1)
    //         = ~((a - 1) | (b - 1))
    //         = -(((a - 1) | (b - 1)) + 1)

    friend void bit_and_neg_neg(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize <= b.ssize);
        assert(b.ssize < 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        mp_size rn = an + 1;
        mp_uint carry = 1;
        mp_uint borrow1 = 1;
        mp_uint borrow2 = 1;

        r.reserve(rn);

        for (mp_size i = 0; i < bn; i++) {
            mp_uint av = a.data[i];
            mp_uint bv = b.data[i];
            mp_uint rv = ((av - borrow1) | (bv - borrow2)) + carry;

            borrow1 = a < borrow1;
            borrow2 = b < borrow2;
            carry = rv < carry;
            r.data[i] = rv;
        }

        for (mp_size i = bn; i < an; i++) {
            mp_uint av = a.data[i];
            mp_uint rv = (av - borrow1) + carry;

            borrow1 = av < borrow1;
            carry = rv < carry;
            r.data[i] = rv;
        }

        r.data[an] = carry;
        r.ssize = mp_negate(r.normal_size(rn));
    }

    friend void bit_and(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!a.ssize || !b.ssize) {
            r.ssize = 0;
        } else if (a.ssize > 0) {
            if (b.ssize > 0) {
                mp_size rn = std::min(a.get_size(), b.get_size());
                r.reserve(rn);
                mp_bit_and_n(r.data, a.data, b.data, rn);
                r.ssize = r.normal_size(rn);
            } else {
                bit_and_pos_neg(a, b, r);
            }
        } else if (b.ssize > 0) {
            bit_and_pos_neg(b, a, r);
        } else if (a.ssize <= b.ssize) {
            bit_and_neg_neg(a, b, r);
        } else {
            bit_and_neg_neg(b, a, r);
        }
    }

    // -a & b = ~(a - 1) & b
    //        = ~((a - 1) | ~b)
    //        = -(((a - 1) | ~b) + 1)

    friend void bit_and_neg_neg(
        const BigIntImpl &a, std::integral auto b, BigIntImpl &r)
    {
        assert(a.ssize < 0);
        assert(b < 0);

        mp_uint an = a.get_size();
        mp_uint rn = an + 1;
        mp_uint av = a.data[0];
        mp_uint rv = ((av - 1) | ~b) + 1;
        mp_uint borrow = av < 1;
        mp_uint carry = rv < 1;

        r.reserve(rn);
        r.data[0] = rv;

        for (mp_size i = 1; i < an; i++) {
            av = a.data[i];
            rv = av - borrow + carry;
            borrow = av < borrow;
            carry = rv < carry;
            r.data[i] = rv;
        }

        r.data[an] = carry;
        r.ssize = mp_negate(rn);
    }

    friend void bit_and(const BigIntImpl &a, std::integral auto b,
                        BigIntImpl &r)
    {
        if (!a.ssize || !b) {
            r.ssize = 0;
        } else if (a.ssize > 0) {
            if (b > 0) {
                r.assign_uint_pos(a.data[0] & b);
            } else {
                mp_size an = a.get_size();

                if (std::addressof(a) != std::addressof(r)) {
                    r.reserve(an);
                    std::copy_n(a.data, an, r.data);
                }

                r.data[0] &= b;
                r.ssize = r.normal_size(an);
            }
        } else if (b > 0) {
            // -a & b = ~(a - 1) & b
            mp_uint rv = (a.data[0] - 1) & b;

            if (rv) {
                r.assign_uint_pos(rv);
            } else {
                r.ssize = 0;
            }
        } else {
            bit_and_neg_neg(a, b, r);
        }
    }

    friend void bit_or_pos_pos(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize >= b.ssize);
        assert(b.ssize > 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();

        if (std::addressof(a) != std::addressof(r)) {
            r.reserve(an);
            r.ssize = an;
            std::copy_n(a.data + bn, an - bn, r.data + bn);
        }

        mp_bit_or_n(r.data, a.data, b.data, bn);
    }

    // a | -b = a | ~(b - 1)
    //        = ~(~a & (b - 1))
    //        = -((~a & (b - 1)) + 1)

    friend void bit_or_pos_neg(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize > 0);
        assert(b.ssize < 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        mp_size rn = bn + 1;
        mp_size min = std::min(an, bn);
        mp_uint borrow = 1;
        mp_uint carry = 1;

        r.reserve(rn);

        for (mp_size i = 0; i < min; i++) {
            mp_uint av = a.data[i];
            mp_uint bv = b.data[i];
            mp_uint rv = (~av & (bv - borrow)) + carry;

            borrow = bv < borrow;
            carry = rv < carry;
            r.data[i] = rv;
        }

        for (mp_size i = min; i < bn; i++) {
            mp_uint bv = b.data[i];
            mp_uint rv = bv - borrow + carry;

            borrow = bv < borrow;
            carry = rv < carry;
            r.data[i] = rv;
        }

        r.data[bn] = carry;
        r.ssize = mp_negate(r.normal_size(rn));
    }

    // -a | -b = ~(a - 1) | ~(b - 1)
    //         = ~((a - 1) & (b - 1))
    //         = -(((a - 1) & (b - 1)) + 1)

    friend void bit_or_neg_neg(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize <= b.ssize);
        assert(b.ssize < 0);

        mp_size bn = b.get_size();
        mp_size rn = bn + 1;
        mp_uint borrow1 = 1;
        mp_uint borrow2 = 1;
        mp_uint carry = 1;

        r.reserve(rn);

        for (mp_size i = 0; i < bn; i++) {
            mp_uint av = a.data[i];
            mp_uint bv = b.data[i];
            mp_uint rv = ((av - borrow1) | (bv - borrow2)) + carry;

            borrow1 = av < borrow1;
            borrow2 = bv < borrow2;
            carry = rv < carry;
            r.data[i] = rv;
        }

        r.data[bn] = carry;
        r.ssize = mp_negate(r.normal_size(rn));
    }

    friend void bit_or(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
        } else if (!b.ssize) {
            r.assign(a);
        } else if (a.ssize > 0) {
            if (b.ssize > 0) {
                if (a.ssize >= b.ssize) {
                    bit_or_pos_pos(a, b, r);
                } else {
                    bit_or_pos_pos(b, a, r);
                }
            } else {
                bit_or_pos_neg(a, b, r);
            }
        } else if (b.ssize > 0) {
            bit_or_pos_neg(a, b, r);
        } else if (a.ssize <= b.ssize) {
            bit_or_neg_neg(a, b, r);
        } else {
            bit_or_neg_neg(b, a, r);
        }
    }

    friend void bit_or(const BigIntImpl &a, std::integral auto b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
        } else if (!b) {
            r.assign(a);
        } else if (a.ssize > 0) {
            if (b > 0) {
                mp_size an = a.get_size();

                if (std::addressof(a) != std::addressof(b)) {
                    r.reserve(an);
                    std::copy_n(a.data, an, r.data);
                    r.ssize = an;
                }

                r.data[0] |= b;
            } else {
                // a | b = ~(~a & ~b)
                //       = -((~a & ~b) + 1)

                // TODO: prove if carry can occur

                mp_uint rv = (~a.data[0] & ~b) + 1;

                if (rv) {
                    r.assign_uint_neg(rv);
                } else {
                    r.reserve(2);
                    r.data[0] = 0;
                    r.data[1] = 1;
                    r.ssize = -2;
                }
            }
        } else if (b > 0) {
            // -a | b = ~(a - 1) | b
            //        = ~((a - 1) & ~b)
            //        = -(((a - 1) & ~b) + 1)

            // TODO: prove if carry can occur

            mp_size an = a.get_size();
            mp_size rn = an + 1;
            mp_uint av = a.data[0];
            mp_uint rv = ((av - 1) & ~b) + 1;
            mp_uint borrow = av < 1;
            mp_uint carry = rv < 1;

            r.reserve(rn);
            r.data[0] = rv;

            for (mp_size i = 0; i < an; i++) {
                av = a.data[i];
                rv = av - borrow + carry;
                borrow = av < borrow;
                carry = rv < carry;
                r.data[i] = rv;
            }

            r.data[an] = carry;
            r.ssize = mp_negate(r.normal_size(rn));
        } else {
            mp_uint rv = ((a.data[0] - 1) & ~b) + 1;

            if (rv) {
                r.assign_uint_neg(rv);
            } else {
                r.reserve(2);
                r.data[0] = 0;
                r.data[1] = 1;
            }
        }
    }

    friend void bit_xor_pos_pos(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize >= b.ssize);
        assert(b.ssize > 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();

        if (std::addressof(a) != std::addressof(r)) {
            r.reserve(an);
            std::copy_n(a.data + bn, an - bn, r.data + bn);
        }

        mp_bit_xor_n(r.data, a.data, b.data, bn);
        r.ssize = r.normal_size(an);
    }

    // a ^ -b = a ^ ~(b - 1)
    //        = ~(a ^ (b - 1))
    //        = -((a ^ (b - 1)) + 1)

    // TODO: prove if carry can occur

    friend void bit_xor_pos_neg(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize > 0);
        assert(b.ssize < 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        auto [min, max] = std::minmax(an, bn);
        mp_size rn = max + 1;
        mp_uint carry = 1;
        mp_uint borrow = 1;

        r.reserve(rn);

        for (mp_size i = 0; i < min; i++) {
            mp_uint av = a.data[i];
            mp_uint bv = b.data[i];
            mp_uint rv = (av ^ (bv - borrow)) + carry;

            borrow = bv < borrow;
            carry = rv < carry;
            r.data[i] = rv;
        }

        for (mp_size i = min; i < an; i++) {
            mp_uint av = a.data[i];
            mp_uint rv = av + carry;

            carry = rv < carry;
            r.data[i] = rv;
        }

        for (mp_size i = min; i < bn; i++) {
            mp_uint bv = a.data[i];
            mp_uint rv = bv - borrow + carry;

            borrow = bv < borrow;
            carry = rv < carry;
            r.data[i] = rv;
        }

        r.data[max] = carry;
        r.ssize = mp_negate(r.normal_size(rn));
    }

    // -a ^ -b = ~(a - 1) ^ ~(b - 1)
    //         = (a - 1) ^ (b - 1)

    friend void bit_xor_neg_neg(
        const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        assert(a.ssize <= b.ssize);
        assert(b.ssize < 0);

        mp_size an = a.get_size();
        mp_size bn = b.get_size();
        mp_uint borrow1 = 1;
        mp_uint borrow2 = 1;

        r.reserve(an);

        for (mp_size i = 0; i < bn; i++) {
            mp_uint av = a.data[i];
            mp_uint bv = b.data[i];
            mp_uint rv = (av - borrow1) ^ (bv - borrow2);

            borrow1 = av < borrow1;
            borrow2 = bv < borrow2;
            r.data[i] = rv;
        }

        if (borrow1) {
            mp_sub_1(r.data + bn, a.data + bn, an - bn, 1);
        }

        r.ssize = r.normal_size(an);
    }

    friend void bit_xor(const BigIntImpl &a, const BigIntImpl &b, BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
        } else if (!b.ssize) {
            r.assign(a);
        } else if (a.ssize > 0) {
            if (b.ssize > 0) {
                if (a.ssize >= b.ssize) {
                    bit_xor_pos_pos(a, b, r);
                } else {
                    bit_xor_pos_pos(b, a, r);
                }
            } else {
                bit_xor_pos_neg(a, b, r);
            }
        } else if (b.ssize > 0) {
            bit_xor_pos_neg(b, a, r);
        } else if (a.ssize <= b.ssize) {
            bit_xor_neg_neg(a, b, r);
        } else {
            bit_xor_neg_neg(b, a, r);
        }
    }

    friend void bit_xor(const BigIntImpl &a, std::integral auto b,
                        BigIntImpl &r)
    {
        if (!a.ssize) {
            r.assign(b);
        } else if (!b) {
            r.assign(a);
        } else if (a.ssize > 0) {
            if (b > 0) {
                mp_size an = a.get_size();

                if (std::addressof(a) != std::addressof(r)) {
                    r.reserve(an);
                    std::copy_n(a.data, an, r.data);
                }

                r.data[0] ^= b;
                r.ssize = an > 1 ? an : r.data[0] > 0;
            } else {
                // a ^ b = ~(a ^ ~b)
                //       = -((a ^ ~b) + 1)

                // TODO: prove if carry can occur

                mp_size an = a.get_size();
                mp_size rn = an + 1;
                mp_uint av = a.data[0];
                mp_uint rv = (av ^ ~b) + 1;
                mp_uint carry = rv < 1;

                r.reserve(rn);
                r.data[0] = rv;

                if (an > 1) {
                    carry = mp_add_1(r.data + 1, a.data + 1, an - 1, carry);
                }

                r.data[an] = carry;
                r.ssize = mp_negate(r.normal_size(rn));
            }
        } else if (b > 0) {
            // -a ^ b = ~(a - 1) ^ b
            //        = ~((a - 1) ^ b)
            //        = -(((a - 1) ^ b) + 1)

            // TODO: prove if carry can occur

            mp_size an = a.get_size();
            mp_size rn = an + 1;
            mp_uint av = a.data[0];
            mp_uint rv = ((av - 1) ^ b) + 1;
            mp_uint borrow = av < 1;
            mp_uint carry = rv < 1;

            r.reserve(rn);
            r.data[0] = rv;

            for (mp_size i = 1; i < an; i++) {
                av = a.data[i];
                rv = av - borrow + carry;

                borrow = av < borrow;
                carry = rv < carry;
                r.data[i] = rv;
            }

            r.data[an] = carry;
            r.ssize = mp_negate(r.normal_size(rn));
        } else {
            // -a ^ b = ~(a - 1) ^ b
            //        = ~((a - 1) ^ b)
            //        = (a - 1) ^ ~b

            mp_size an = a.get_size();

            r.reserve(an);
            mp_sub_1(r.data, a.data, an, 1);
            r.data[0] ^= b;
            r.ssize = r.normal_size(an);
        }
    }

    // ~a = -(a + 1)

    friend void bit_not(const BigIntImpl &a, BigIntImpl &r)
    {
        abs_add(a, 1, r);
        r.negate();
    }

    friend void left_shift(const BigIntImpl &a, std::size_t n, BigIntImpl &r)
    {
        mp_size words = n / mp_uint_bits;
        mp_size bits = n % mp_uint_bits;
        mp_size an = a.get_size();
        mp_size rn = an + words + 1;

        r.reserve(rn);

        if (bits) {
            mp_size rbits = mp_uint_bits - bits;

            for (mp_size i = an - 1; i > 0;) {
                r.data[i + words] =
                    (a.data[i] << bits) | (a.data[--i] >> rbits);
            }

            r.data[words] = a.data[0] << bits;
        } else if (std::addressof(a) == std::addressof(r)) {
            std::copy_backward(a.data, a.data + an, r.data + words);
        }

        std::fill_n(r.data, words, 0);
        rn = r.normal_size(rn);
        r.ssize = a.ssize >= 0 ? rn : mp_negate(rn);
    }

    friend void right_shift(const BigIntImpl &a, std::size_t n, BigIntImpl &r)
    {
        mp_size words = n / mp_uint_bits;
        mp_size bits = n % mp_uint_bits;
        mp_size an = a.get_size();

        if (words >= an) {
            if (a.ssize >= 0) {
                r.ssize = 0;
            } else {
                assign_uint_neg(1);
            }
            return;
        }

        mp_size rn = an - words;
        r.reserve(rn);

        if (bits) {
            mp_size lbits = mp_uint_bits - bits;

            --an;
            for (mp_size i = words, lim = an; i < an;) {
                r.data[i - words] =
                    (a.data[i] >> bits) | (a.data[++i] << lbits);
            }

            r.data[rn - 1] = (a.data[an] >> bits);
        } else {
            std::copy_n(a.data + words, rn, r.data);
        }

        rn = r.normal_size(rn);
        r.ssize = a.ssize >= 0 ? rn : mp_negate(rn);
    }
};

} // namespace htl::mp::detail

#endif
