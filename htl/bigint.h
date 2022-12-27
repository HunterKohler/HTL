#ifndef HTL_BIGINT_H_
#define HTL_BIGINT_H_

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <string>
#include <htl/detail/bigint.h>
#include <htl/math.h>

namespace htl {

template <class Alloc>
class BasicBigInt {
public:
    using allocator_type = Alloc;

    BasicBigInt() noexcept(noexcept(Alloc())) : _alloc() {}

    explicit BasicBigInt(const Alloc &alloc) noexcept : _alloc(alloc) {}

    explicit BasicBigInt(
        std::integral auto value, const Alloc &alloc = Alloc());

    BasicBigInt(const BasicBigInt &other)
        : BasicBigInt(
              other,
              std::allocator_traits<Alloc>::
                  select_on_container_copy_construction(other.get_allocator()))
    {}

    BasicBigInt(const BasicBigInt &other, const Alloc &alloc) : _alloc(alloc)
    {
        _copy_construct(other);
    }

    BasicBigInt(BasicBigInt &&other) noexcept : _alloc(std::move(other._alloc))
    {
        _move_data(std::move(other));
    }

    BasicBigInt(BasicBigInt &&other, const Alloc &alloc) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value)
        : _alloc(alloc)
    {
        if (std::allocator_traits<Alloc>::is_always_equal::value ||
            _alloc == other._alloc) {
            _move_data(std::move(other));
        } else {
            _copy_construct(other);
        }
    }

    ~BasicBigInt() noexcept
    {
        _destroy();
    }

    BasicBigInt &operator=(const BasicBigInt &other)
    {
        if (this != std::addressof(other)) {
            _copy_assign(other);
        }

        return *this;
    }

    BasicBigInt &operator=(BasicBigInt &&other) //
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value ||
                 std::allocator_traits<
                     Alloc>::propagate_on_container_move_assignment::value)
    {
        if (this != std::addressof(other)) {
            _move_assign(std::move(other));
        }

        return *this;
    }

    void assign(std::integral auto value);

    Alloc get_allocator() const noexcept
    {
        return Alloc(_data);
    }

    int sign() const noexcept
    {
        return htl::sign(_size);
    }

    void abs() noexcept
    {
        _size = std::abs(_size);
    }

    void add(const BasicBigInt &other)
    {
        if (_size ^ other._size >= 0) {
            _abs_add(other);
        } else {
            _abs_sub(other);
        }
    }

    void add(std::integral auto value);

    void sub(const BasicBigInt &other)
    {
        if (_size ^ other._size >= 0) {
            _abs_sub(other);
        } else {
            _abs_add(other);
        }
    }

    void sub(std::integral auto value);

    void mul(const BasicBigInt &other);
    void mul(std::integral auto value);

    void div(const BasicBigInt &other);
    void div(std::integral auto value);

    void rem(const BasicBigInt &other);
    void rem(std::integral auto value);

    void bit_and(const BasicBigInt &other);
    void bit_and(std::integral auto value);

    void bit_xor(const BasicBigInt &other);
    void bit_xor(std::integral auto value);

    void bit_or(const BasicBigInt &other);
    void bit_or(std::integral auto value);

    void swap(BasicBigInt &other) //
        noexcept(
            std::allocator_traits<Alloc>::is_always_equal::value ||
            std::allocator_traits<Alloc>::propagate_on_container_swap::value)
    {
        using std::swap;

        constexpr bool propagate =
            std::allocator_traits<Alloc>::propagate_on_container_swap::value;

        if constexpr (propagate) {
            swap(_alloc, other._alloc);
        }

        swap(_size, other._size);
        swap(_capacity, other._capacity);
        swap(_data, other._data);
    }

    BasicBigInt &operator+=(const BasicBigInt &other)
    {
        add(other);
        return *this;
    }

    BasicBigInt &operator+=(std::integral auto value)
    {
        add(value);
        return *this;
    }

    BasicBigInt &operator-=(const BasicBigInt &other)
    {
        sub(other);
        return *this;
    }

    BasicBigInt &operator-=(std::integral auto value)
    {
        sub(value);
        return *this;
    }

    BasicBigInt &operator*=(const BasicBigInt &other)
    {
        mul(other);
        return *this
    }

    BasicBigInt &operator*=(std::integral auto value)
    {
        mul(value);
        return *this
    }

    BasicBigInt &operator/=(const BasicBigInt &other)
    {
        div(other);
        return *this;
    }

    BasicBigInt &operator/=(std::integral auto value)
    {
        div(value);
        return *this;
    }

    BasicBigInt &operator%=(const BasicBigInt &other)
    {
        rem(other);
        return *this;
    }

    BasicBigInt &operator%=(std::integral auto value)
    {
        rem(value);
        return *this;
    }

    BasicBigInt &operator&=(const BasicBigInt &other)
    {
        bit_and(other);
        return *this;
    }

    BasicBigInt &operator&=(std::integral auto value)
    {
        bit_and(value);
        return *this;
    }

    BasicBigInt &operator^=(const BasicBigInt &other)
    {
        bit_xor(other);
        return *this;
    }

    BasicBigInt &operator^=(std::integral auto value)
    {
        bit_xor(value);
        return *this;
    }

    BasicBigInt &operator|=(const BasicBigInt &other)
    {
        bit_or(other);
        return *this;
    }

    BasicBigInt &operator|=(std::integral auto value)
    {
        bit_or(value);
        return *this;
    }

    BasicBigInt &operator<<=(std::size_t n);

    BasicBigInt &operator>>=(std::size_t n);

    BasicBigInt operator+() const
    {
        return *this;
    }

    BasicBigInt operator+() &&
    {
        return std::move(*this);
    }

    BasicBigInt operator-() const
    {
        auto copy(*this);
        copy._size = -_size;
        return copy;
    }

    BasicBigInt operator-() &&
    {
        _size = -_size;
        return std::move(*this);
    }

    BasicBigInt &operator++()
    {
        return *this += 1;
    }

    BasicBigInt &operator--()
    {
        return *this -= 1;
    }

    BasicBigInt operator++(int)
    {
        auto copy(*this);
        ++*this;
        return copy;
    }

    BasicBigInt operator--(int)
    {
        auto copy(*this);
        --*this;
        return copy;
    }

    explicit operator bool() noexcept
    {
        return _size;
    }

    friend void swap(BasicBigInt &a, BasicBigInt &b) //
        noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }

    friend bool operator==(const BasicBigInt &a, const BasicBigInt &b) noexcept
    {
        return a._size == b._size &&
               std::equal(a._data, a._data + std::abs(_size), b._data);
    }

    friend bool operator==(const BasicBigInt &a, std::integral auto b) noexcept;

    friend std::strong_ordering operator<=>(
        const BasicBigInt &a, const BasicBigInt &b) noexcept
    {
        if (a._size != b._size) {
            return a._size <=> b._size;
        }

        mp_size_t n = std::abs(a._size);
        return std::lexicographical_compare_three_way(
            a._data, a._data + n, b._data, b._data + n);
    }

    friend std::strong_ordering operator<=>(
        const BasicBigInt &a, std::integral auto b) noexcept;

    friend BasicBigInt operator+(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator+(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator+(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator+(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator+(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator+(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator+(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator+(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator-(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator-(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator-(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator-(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator-(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator-(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator-(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator-(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator*(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator*(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator*(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator*(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator*(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator*(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator*(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator*(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator/(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator/(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator/(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator/(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator/(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator/(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator/(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator/(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator%(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator%(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator%(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator%(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator%(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator%(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator%(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator%(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator&(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator&(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator&(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator&(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator&(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator&(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator&(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator&(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator^(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator^(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator^(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator^(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator^(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator^(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator^(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator^(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator|(const BasicBigInt &a, const BasicBigInt &b);
    friend BasicBigInt operator|(const BasicBigInt &a, std::integral auto b);
    friend BasicBigInt operator|(const BasicBigInt &a, BasicBigInt &&b);
    friend BasicBigInt operator|(BasicBigInt &&a, const BasicBigInt &b);
    friend BasicBigInt operator|(BasicBigInt &&a, std::integral auto b);
    friend BasicBigInt operator|(BasicBigInt &&a, BasicBigInt &&b);
    friend BasicBigInt operator|(std::integral auto a, const BasicBigInt &b);
    friend BasicBigInt operator|(std::integral auto a, BasicBigInt &&b);

    friend BasicBigInt operator<<(const BasicBigInt &a, std::size_t n);
    friend BasicBigInt operator<<(BasicBigInt &&a, std::size_t n);

    friend BasicBigInt operator>>(const BasicBigInt &a, std::size_t n);
    friend BasicBigInt operator>>(BasicBigInt &&a, std::size_t n);

    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(
        std::basic_ostream<CharT, Traits> &stream, const BasicBigInt &value);

    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT, Traits> &stream, BasicBigInt &value);

    template <std::output_iterator O>
    friend O to_chars(const BasicBigInt &value, O out, int base = 10);

    template <class Alloc = std::allocator<char>>
    friend std::basic_string<char, std::char_traits<char>, Alloc>
    to_string(const BasicBigInt &value, const Alloc &alloc);

    template <class Alloc = std::allocator<char>>
    friend std::basic_string<char, std::char_traits<char>, Alloc> to_string(
        const BasicBigInt &value, int base = 10, const Alloc &alloc = Alloc());

private:
    using mp_uint_t = detail::mp_uint_t;
    using mp_size_t = detail::mp_size_t;
    using mp_ssize_t = detail::mp_ssize_t;
    using mp_uint_alloc =
        typename std::allocator_traits<Alloc>::rebind_alloc<mp_uint_t>;

    [[no_unique_address]] mp_uint_alloc _alloc;
    std::make_signed_t<mp_size_t> _size;
    mp_size_t _capacity;
    std::uintptr_t *_data;

    static mp_uint_t *_allocate(mp_uint_alloc &alloc, mp_size_t n)
    {
        if (!n) {
            return nullptr;
        }

        return std::to_address(
            std::allocator_traits<mp_uint_alloc>::allocate(alloc, n));
    }

    static void _deallocate(
        mp_uint_alloc &alloc, mp_uint_t *p, mp_size_t n) noexcept
    {
        using pointer = typename std::allocator_traits<mp_uint_alloc>::pointer;

        if (n) {
            pointer ptr = std::pointer_traits<pointer>::pointer_to(*p);
            std::allocator_traits<mp_uint_alloc>::deallocate(alloc, ptr, n);
        }
    }

    void _destroy()
    {
        _deallocate(_alloc, _data, _capacity);
    }

    void _resize(mp_size_t n)
    {
        _reserve(n);
        _set_size(n);
    }

    void _reserve(mp_size_t n)
    {
        if (_capacity < n) {
            mp_uint_t *new_data = _allocate(_alloc, n);
            std::copy_n(_data, std::abs(size), n);
            _destroy();
            _data = new_data;
            _capacity = n;
        }
    }

    void _reserve_for_overwrite(mp_size_t n)
    {
        if (new_capacity > _capacity) {
            mp_uint_t *new_data = _allocate(_alloc, n);
            _destroy();
            _data = new_data;
            _capacity = n;
        }
    }

    void _set_size(mp_size_t n)
    {
        _size = _size > 0 ? n : -static_cast<mp_ssize_t>(n);
    }

    void _copy_construct(const BasicBigInt &other)
    {
        _data = _allocate(_alloc, other._capacity);
        _size = other._size;
        _capacity = other._capacity;

        std::copy_n(other._data, std::abs(_size), _data);
    }

    void _move_data(BasicBigInt &&other) noexcept
    {
        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;
        other._clear_data();
    }

    void _clear_data() noexcept
    {
        _data = nullptr;
        _size = 0;
        _capacity = 0;
    }

    void _copy_assign(const BasicBigInt &other)
    {
        constexpr bool propagate = std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value;
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;

        mp_size_t n = std::abs(other._size);

        if (!propagate || is_always_equal || _alloc == other._alloc) {
            _reserve_for_overwrite(n);

            if constexpr (propagate) {
                _alloc = other._alloc;
            }
        } else {
            mp_uint_t *new_data = _allocate(other._alloc, n);
            _destroy();
            _data = new_data;
            _capacity = n;
        }

        _size = other._size;
        std::copy_n(other._data, n, _data);
    }

    void _move_assign(BasicBigInt &&other)
    {
        constexpr bool propagate = std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value;
        constexpr bool is_always_equal =
            std::allocator_traits<Alloc>::is_always_equal::value;

        if (propagate || is_always_equal || _alloc == other._alloc) {
            _move_data(std::move(other));

            if constexpr (propagate) {
                _alloc = other._alloc;
            }
        } else {
            mp_size_t n = std::abs(other._size);

            _reserve_for_overwrite(n);
            _size = other._size;
            std::copy_n(other._data, n, _data);
        }
    }

    void _abs_add(const BasicBigInt &other)
    {
        mp_size_t an = std::abs(_size);
        mp_size_t bn = std::abs(other._size);
        const mp_uint_t *ap = _data;
        const mp_uint_t *bp = other._data;

        if (an < bn) {
            std::swap(ap, bp);
            std::swap(an, bn);
        }

        mp_size_t new_size = an + 1;

        _reserve(new_size);
        _set_size(new_size);
        _data[an] = detail::mp_add(_data, ap, an, bp, bn);
        _normalize();
    }

    void _abs_sub(const BasicBigInt &other);

    void _normalize()
    {
        mp_size_t n = std::abs(_size);

        for (; n && !dest[--n];) {
        }

        _set_size(n);
    }
};

using BigInt = BasicBigInt<std::allocator<std::byte>>;

namespace pmr {

using BigInt = BasicBigInt<std::pmr::polymorphic_allocator<std::byte>>;

} // namespace pmr

} // namespace htl

#endif
