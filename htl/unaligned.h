#ifndef HTL_UNALIGNED_H_
#define HTL_UNALIGNED_H_

#include <concepts>
#include <htl/bit.h>
#include <htl/type_traits.h>

namespace htl {
namespace detail {

template <bool Cond, class T>
constexpr T byteswap_if(T value) noexcept
{
    if constexpr (Cond) {
        return byteswap(value);
    } else {
        return value;
    }
}

template <class T>
struct HTL_ATTR_PACKED PackedStorage {
    T value;
};

} // namespace detail

template <std::integral T, std::endian E = std::endian::native, class U>
    requires is_byte_v<U>
constexpr T load_unaligned(const U *p) noexcept
{
    if (std::is_constant_evaluated()) {
        T value{};

        for (int i = 0; i < sizeof(T); i++) {
            value |= T(p[i] << (8 * i));
        }

        return detail::byteswap_if<std::endian::little != E>(value);
    } else {
        return detail::byteswap_if<std::endian::little != E>(
            reinterpret_cast<const detail::PackedStorage<T> *>(p)->value);
    }
}

template <std::integral T, std::endian E = std::endian::native, class U>
    requires(!std::is_const_v<U> && is_byte_v<U>)
constexpr void store_unaligned(U *p, T value) noexcept
{
    if (std::is_constant_evaluated()) {
        value = detail::byteswap_if<std::endian::little != E>(value);

        for (int i = 0; i < sizeof(T); i++) {
            p[i] = (value >> (8 * i)) & 0xFF;
        }
    } else {
        reinterpret_cast<detail::PackedStorage<T> *>(p)->value =
            detail::byteswap_if<std::endian::native != E>(value);
    }
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint16_t load_unaligned_le16(const T *p) noexcept
{
    return load_unaligned<std::uint16_t, std::endian::little>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint32_t load_unaligned_le32(const T *p) noexcept
{
    return load_unaligned<std::uint32_t, std::endian::little>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint64_t load_unaligned_le64(const T *p) noexcept
{
    return load_unaligned<std::uint64_t, std::endian::little>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint16_t load_unaligned_be16(const T *p) noexcept
{
    return load_unaligned<std::uint16_t, std::endian::big>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint32_t load_unaligned_be32(const T *p) noexcept
{
    return load_unaligned<std::uint32_t, std::endian::big>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint64_t load_unaligned_be64(const T *p) noexcept
{
    return load_unaligned<std::uint64_t, std::endian::big>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint16_t load_unaligned_ne16(const T *p) noexcept
{
    return load_unaligned<std::uint16_t, std::endian::native>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint32_t load_unaligned_ne32(const T *p) noexcept
{
    return load_unaligned<std::uint32_t, std::endian::native>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr std::uint64_t load_unaligned_ne64(const T *p) noexcept
{
    return load_unaligned<std::uint64_t, std::endian::native>(p);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_le16(T *p, std::uint16_t value) noexcept
{
    store_unaligned<std::uint16_t, std::endian::little>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_le32(T *p, std::uint32_t value) noexcept
{
    store_unaligned<std::uint32_t, std::endian::little>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_le64(T *p, std::uint64_t value) noexcept
{
    store_unaligned<std::uint64_t, std::endian::little>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_be16(T *p, std::uint16_t value) noexcept
{
    store_unaligned<std::uint16_t, std::endian::big>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_be32(T *p, std::uint32_t value) noexcept
{
    store_unaligned<std::uint32_t, std::endian::big>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_be64(T *p, std::uint64_t value) noexcept
{
    store_unaligned<std::uint64_t, std::endian::big>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_ne16(T *p, std::uint16_t value) noexcept
{
    store_unaligned<std::uint16_t, std::endian::native>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_ne32(T *p, std::uint32_t value) noexcept
{
    store_unaligned<std::uint32_t, std::endian::native>(p, value);
}

template <class T>
    requires is_byte_v<T>
constexpr void store_unaligned_ne64(T *p, std::uint64_t value) noexcept
{
    store_unaligned<std::uint64_t, std::endian::native>(p, value);
}

} // namespace htl

#endif
