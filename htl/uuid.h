#ifndef HTL_UUID_H_
#define HTL_UUID_H_

#include <array>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <htl/detail/default_hash.h>
#include <htl/detail/encoding.h>
#include <htl/unaligned.h>
#include <htl/utility.h>

namespace htl {

enum class UUIDVersion : unsigned char {
    Unknown,
    v1,
    v2,
    v3,
    v4,
    v5,
};

enum class UUIDVariant : unsigned char {
    Unknown,
    NCS,
    RFC,
    Microsoft,
    Future,
};

namespace detail {

inline auto uuid_version_from_num(std::byte value) noexcept
{
    switch (to_underlying(value)) {
    case 1:
        return UUIDVersion::v1;
    case 2:
        return UUIDVersion::v2;
    case 3:
        return UUIDVersion::v3;
    case 4:
        return UUIDVersion::v4;
    case 5:
        return UUIDVersion::v5;
    default:
        return UUIDVersion::Unknown;
    }
}

inline auto uuid_version_to_num(UUIDVersion value) noexcept
{
    return std::byte(to_underlying(value));
}

inline auto uuid_variant_from_num(std::byte value) noexcept
{
    switch (to_underlying(value)) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return UUIDVariant::NCS;
    case 8:
    case 9:
    case 10:
    case 11:
        return UUIDVariant::RFC;
    case 12:
    case 13:
        return UUIDVariant::Microsoft;
    case 14:
        return UUIDVariant::Future;
    default:
        return UUIDVariant::Unknown;
    }
}

inline auto uuid_variant_to_num(UUIDVariant value) noexcept
{
    switch (value) {
    case UUIDVariant::NCS:
        return std::byte();
    case UUIDVariant::RFC:
        return std::byte(8);
    case UUIDVariant::Microsoft:
        return std::byte(12);
    case UUIDVariant::Future:
        return std::byte(14);
    default:
        return std::byte(15);
    }
}

} // namespace detail

class UUID {
public:
    struct bytes_type : std::array<std::byte, 16> {
        constexpr bytes_type() noexcept = default;

        template <class... Args>
        constexpr bytes_type(Args &&...args)
            : std::array<std::byte, 16>{ static_cast<std::byte>(
                  std::forward<Args>(args))... }
        {}
    };

    UUID() noexcept = default;

    UUID(std::nullptr_t) noexcept : _bytes() {}

    UUID(const bytes_type &bytes) noexcept : _bytes(bytes) {}

    bytes_type to_bytes() const noexcept
    {
        return _bytes;
    }

    UUIDVersion version() const noexcept
    {
        std::byte value = _bytes[6] >> 4;
        return detail::uuid_version_from_num(value);
    }

    UUIDVariant variant() const noexcept
    {
        std::byte value = _bytes[8] >> 4;
        return detail::uuid_variant_from_num(value);
    }

    bool is_nil() const noexcept
    {
        for (auto value: _bytes) {
            if (value != std::byte()) {
                return false;
            }
        }

        return true;
    }

    explicit operator bool() const noexcept
    {
        return !is_nil();
    }

    friend bool operator==(const UUID &, const UUID &) noexcept = default;

    friend std::strong_ordering operator<=>(
        const UUID &, const UUID &) noexcept = default;

private:
    bytes_type _bytes;
};

inline UUID make_uuid(std::string_view s, std::error_code &err) noexcept
{
    UUID::bytes_type bytes;

    if (s.size() == 38 && s.front() == '{' && s.back() == '}') {
        s.remove_prefix(1);
        s.remove_suffix(1);
    }

    if (s.size() != 36 || s[8] != '-' || s[13] != '-' || s[18] != '-' ||
        s[23] != '-') {
        err = std::make_error_code(std::errc::invalid_argument);
        return {};
    }

    char hex[]{
        s[0],  s[1],  s[2],  s[3],  s[4],  s[5],  s[6],  s[7],
        s[9],  s[10], s[11], s[12], s[14], s[15], s[16], s[17],
        s[19], s[20], s[21], s[22], s[24], s[25], s[26], s[27],
        s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35]
    };

    for (int i = 0; i < 16; i++) {
        auto a = detail::hex_values[static_cast<unsigned char>(hex[2 * i])];
        auto b = detail::hex_values[static_cast<unsigned char>(hex[2 * i + 1])];

        if (a < 0 || b < 0) {
            err = std::make_error_code(std::errc::invalid_argument);
            return {};
        }

        bytes[i] = std::byte(a) << 4 | std::byte(b);
    }

    err.clear();
    return { bytes };
}

inline UUID make_uuid(std::string_view str)
{
    std::error_code err;
    auto value = make_uuid(str, err);
    if (err) {
        throw std::invalid_argument("htl::make_uuid: Invalid UUID string");
    }
    return value;
}

namespace detail {

inline void set_uuid_bytes_version(auto &bytes, UUIDVersion new_version)
{
    bytes[6] =
        (uuid_version_to_num(new_version) << 4) | (bytes[6] & std::byte(15));
}

inline void set_uuid_bytes_variant(auto &bytes, UUIDVariant new_variant)
{
    bytes[8] =
        (uuid_variant_to_num(new_variant) << 4) | (bytes[8] & std::byte(15));
}

} // namespace detail

template <class Gen>
    requires std::uniform_random_bit_generator<std::remove_reference_t<Gen>>
inline UUID make_uuid(Gen &&g, UUIDVariant variant = UUIDVariant::RFC)
{
    UUID::bytes_type bytes;
    std::uniform_int_distribution<std::uint64_t> dist;
    store_unaligned_ne64(bytes.data(), dist(g));
    store_unaligned_ne64(bytes.data() + 8, dist(g));
    detail::set_uuid_bytes_variant(bytes, variant);
    detail::set_uuid_bytes_version(bytes, UUIDVersion::v4);
    return { bytes };
}

namespace detail {

inline void uuid_to_chars_hex(auto byte, auto &out)
{
    *out = hex_charset_lower[to_underlying(byte) >> 4];
    ++out;
    *out = hex_charset_lower[to_underlying(byte) & 15];
    ++out;
}

inline void uuid_to_chars_dash(auto &out)
{
    *out = '-';
    ++out;
}

} // namespace detail

template <std::output_iterator<char> O>
inline O to_chars(const UUID &value, O out)
{
    auto bytes = value.to_bytes();
    detail::uuid_to_chars_hex(bytes[0], out);
    detail::uuid_to_chars_hex(bytes[1], out);
    detail::uuid_to_chars_hex(bytes[2], out);
    detail::uuid_to_chars_hex(bytes[3], out);
    detail::uuid_to_chars_dash(out);
    detail::uuid_to_chars_hex(bytes[4], out);
    detail::uuid_to_chars_hex(bytes[5], out);
    detail::uuid_to_chars_dash(out);
    detail::uuid_to_chars_hex(bytes[6], out);
    detail::uuid_to_chars_hex(bytes[7], out);
    detail::uuid_to_chars_dash(out);
    detail::uuid_to_chars_hex(bytes[8], out);
    detail::uuid_to_chars_hex(bytes[9], out);
    detail::uuid_to_chars_dash(out);
    detail::uuid_to_chars_hex(bytes[10], out);
    detail::uuid_to_chars_hex(bytes[11], out);
    detail::uuid_to_chars_hex(bytes[12], out);
    detail::uuid_to_chars_hex(bytes[13], out);
    detail::uuid_to_chars_hex(bytes[14], out);
    detail::uuid_to_chars_hex(bytes[15], out);
    return out;
}

template <class Alloc = std::allocator<char>>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const UUID &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    res.reserve(36);
    to_chars(value, std::back_inserter(res));
    return res;
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &stream, const UUID &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, UUID &value)
{
    char buf[37];

    if (stream && stream >> buf) {
        std::error_code err;
        auto new_value = make_uuid(std::string_view(buf, 36), err);
        if (err) {
            stream.setstate(std::ios::failbit);
        } else {
            value = new_value;
        }
    }

    return stream;
}

} // namespace htl

namespace std {

template <>
struct hash<htl::UUID> {
    std::size_t operator()(const htl::UUID &value) noexcept
    {
        return htl::detail::default_hash(value.to_bytes());
    }
};

} // namespace std

#endif
