#ifndef HLIB_IP_H_
#define HLIB_IP_H_

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <htl/ascii.h>
#include <htl/config.h>
#include <htl/detail/default_hash.h>
#include <htl/detail/encoding.h>
#include <htl/detail/ip.h>
#include <htl/detail/iterator.h>
#include <htl/scope_guard.h>
#include <htl/unaligned.h>
#include <htl/utility.h>

namespace htl {

using scope_id_type = std::uint64_t;

class IPv4Address;

class IPv6Address;

class IPAddress;

class IPv4Network;

class IPv6Network;

template <class Address>
class BasicIPAddressIterator;

template <class Address>
class BasicIPAddressRange;

using IPv4AddressIterator = BasicIPAddressIterator<IPv4Address>;

using IPv6AddressIterator = BasicIPAddressIterator<IPv6Address>;

using IPv4AddressRange = BasicIPAddressRange<IPv4Address>;

using IPv6AddressRange = BasicIPAddressRange<IPv6Address>;

class IPv4Address {
public:
    using uint_type = std::uint32_t;

    struct bytes_type : std::array<std::byte, 4> {
        bytes_type() noexcept = default;

        template <class... Args>
        bytes_type(Args &&...args)
            : std::array<std::byte, 4>{ static_cast<std::byte>(
                  std::forward<Args>(args))... }
        {}
    };

    IPv4Address() noexcept = default;

    IPv4Address(const bytes_type &bytes) noexcept : _bytes(bytes) {}

    explicit IPv4Address(uint_type value) noexcept
    {
        store_unaligned_be32(_bytes.data(), value);
    }

    IPv4Address(const IPv4Address &other) noexcept = default;

    IPv4Address &operator=(const IPv4Address &other) noexcept = default;

    bool is_unspecified() const noexcept
    {
        // 0.0.0.0/32
        return !_get(0) && !_get(1) && !_get(2) && !_get(3);
    }

    bool is_loopback() const noexcept
    {
        // 127.0.0.0/8
        return _get(0) == 0x7F;
    }

    bool is_class_a() const noexcept
    {
        // 0.0.0.0/8
        return !_get(0);
    }

    bool is_class_b() const noexcept
    {
        // 128.0.0.0/16
        return _get(0) == 0x80 && !_get(1);
    }

    bool is_class_c() const noexcept
    {
        // 192.0.0.0/24
        return _get(0) == 0xC0 && !_get(1) && !_get(2);
    }

    bool is_multicast() const noexcept
    {
        // 224.0.0.0/4
        return (_get(0) >> 4) == 0xE;
    }

    bool is_link_local() const noexcept
    {
        // 169.254.0.0/16
        return _get(0) == 0xA9 && _get(1) == 0xFE;
    }

    bool is_private() const noexcept
    {
        // 10.0.0.0/8
        // 172.16.0.0/12
        // 192.168.0.0/16
        return (
            (_get(0) == 0x0A) || (_get(0) == 0xAC && (_get(1) >> 4) == 0x01) ||
            (_get(0) == 0xC0 && _get(1) == 0xA8));
    }

    bytes_type to_bytes() const noexcept
    {
        return _bytes;
    }

    uint_type to_uint() const noexcept
    {
        return load_unaligned_be32(_bytes.data());
    }

    static IPv4Address any() noexcept
    {
        return {};
    }

    static IPv4Address loopback() noexcept
    {
        return bytes_type{ 0x7F, 0x00, 0x00, 0x01 };
    }

    static IPv4Address broadcast() noexcept
    {
        return bytes_type{ 0xFF, 0xFF, 0xFF, 0xFF };
    }

    static IPv4Address broadcast(
        const IPv4Address &addr, const IPv4Address &mask) noexcept
    {
        return IPv4Address{ addr.to_uint() | ~mask.to_uint() };
    }

    friend bool operator==(const IPv4Address &, const IPv4Address &) noexcept =
        default;

    friend std::strong_ordering operator<=>(
        const IPv4Address &, const IPv4Address &) noexcept = default;

private:
    bytes_type _bytes;

    std::uint8_t _get(std::size_t n) const noexcept
    {
        return static_cast<std::uint8_t>(_bytes[n]);
    }

    uint_type _get_uint(std::size_t n) const noexcept
    {
        return static_cast<uint_type>(_bytes[n]);
    }
};

inline IPv4Address make_ipv4_address(std::string_view str)
{
    IPv4Address::bytes_type bytes;
    auto first = str.begin();
    auto last = str.end();
    auto code = detail::try_parse_ipv4_address(first, last, bytes);

    if (code == std::errc() && first != last) {
        code = std::errc::invalid_argument;
    }

    switch (code) {
    case std::errc::invalid_argument:
        throw std::invalid_argument(
            "htl::make_ipv4_address(): Invalid IPv4 address");
    }
    return bytes;
}

inline IPv4Address make_ipv4_address(
    std::string_view str, std::error_code &ec) noexcept
{
    IPv4Address::bytes_type bytes;
    auto first = str.begin();
    auto last = str.end();
    auto code = detail::try_parse_ipv4_address(first, last, bytes);

    if (code == std::errc() && first != last) {
        code = std::errc::invalid_argument;
    }

    ec = std::make_error_code(code);
    return bytes;
}

template <std::output_iterator<char> O>
inline O to_chars(const IPv4Address &value, O out)
{
    detail::ipv4_address_to_chars(value.to_bytes(), out);
    return out;
}

template <class Alloc = std::allocator<char>>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const IPv4Address &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(res, std::back_inserter(res));
    return res;
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &stream, const IPv4Address &value)
{
    to_chars(value, std::back_inserter(stream));
    return stream;
}

template <class CharT, class Traits>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, IPv4Address &value)
{
    if (!stream) {
        return stream;
    }

    IPv4Address::bytes_type bytes;
    std::errc code = detail::try_parse_ipv4_address(
        std::istream_iterator<char>(stream), std::default_sentinel, bytes);
    if (code == std::errc()) {
        value = bytes;
    } else {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}

class IPv6Address {
public:
    struct bytes_type : std::array<std::byte, 16> {
        bytes_type() noexcept = default;

        template <class... Args>
        bytes_type(Args &&...args)
            : std::array<std::byte, 16>{ static_cast<std::byte>(
                  std::forward<Args>(args))... }
        {}
    };

    IPv6Address() noexcept = default;

    IPv6Address(const bytes_type &bytes, scope_id_type scope_id = 0)
        : _bytes(bytes), _scope_id(scope_id)
    {}

    IPv6Address(const IPv6Address &other) noexcept = default;

    IPv6Address &operator=(const IPv6Address &other) noexcept = default;

    void scope_id(scope_id_type new_scope_id) noexcept
    {
        _scope_id = new_scope_id;
    }

    scope_id_type scope_id() const noexcept
    {
        return _scope_id;
    }

    bool is_unspecified() const noexcept
    {
        // ::/128
        return !_get(0) && !_get(1) && !_get(2) && !_get(3) && !_get(4) &&
               !_get(5) && !_get(6) && !_get(7) && !_get(8) && !_get(9) &&
               !_get(10) && !_get(11) && !_get(12) && !_get(13) && !_get(14) &&
               !_get(15);
    }

    bool is_loopback() const noexcept
    {
        // ::1/128
        return !_get(0) && !_get(1) && !_get(2) && !_get(3) && !_get(4) &&
               !_get(5) && !_get(6) && !_get(7) && !_get(8) && !_get(9) &&
               !_get(10) && !_get(11) && !_get(12) && !_get(13) && !_get(14) &&
               _get(15) == 1;
    }

    bool is_multicast() const noexcept
    {
        // ff00::/8
        return _get(0) == 0xFF;
    }

    bool is_link_local() const noexcept
    {
        // fe80::/10
        return _get(0) == 0xFE && (_get(1) & 0xC0) == 0x80;
    }

    bool is_site_local() const noexcept
    {
        // fec0::/10
        return _get(0) == 0xFE && (_get(1) & 0xC0) == 0xC0;
    }

    bool is_v4_mapped() const noexcept
    {
        // ::ffff:0.0.0.0/96
        //     const uint8_t *p = address->data;
        return !_get(0) && !_get(1) && !_get(2) && !_get(3) && !_get(4) &&
               !_get(5) && !_get(6) && !_get(7) && !_get(8) && !_get(9) &&
               _get(10) == 0xFF && _get(11) == 0xFF;
    }

    bool is_v4_compatible() const noexcept
    {
        // ::0.0.0.0/96
        return !_get(0) && !_get(1) && !_get(2) && !_get(3) && !_get(4) &&
               !_get(5) && !_get(6) && !_get(7) && !_get(8) && !_get(9) &&
               !_get(10) && !_get(11);
    }

    bool is_multicast_node_local() const noexcept
    {
        // ff01::/16
        return _get(0) == 0xFF && _get(1) == 0x01;
    }

    bool is_multicast_link_local() const noexcept
    {
        // ff02::/16
        return _get(0) == 0xFF && _get(1) == 0x02;
    }

    bool is_multicast_site_local() const noexcept
    {
        // ff05::/16
        return _get(0) == 0xFF && _get(1) == 0x05;
    }

    bool is_multicast_org_local() const noexcept
    {
        // ff08::/16
        return _get(0) == 0xFF && _get(1) == 0x08;
    }

    bool is_multicast_global() const noexcept
    {
        // ff0e::/16
        return _get(0) == 0xFF && _get(1) == 0x0E;
    }

    bool is_private() const noexcept
    {
        if (is_v4_mapped()) {
            return IPv4Address({ _get(12), _get(13), _get(14), _get(15) })
                .is_private();
        } else {
            // fc00::/7
            return _get(0) == 0xFC && !(_get(1) & 0xFE);
        }
    }

    bytes_type to_bytes() const noexcept
    {
        return _bytes;
    }

    static IPv6Address any() noexcept
    {
        return {};
    }

    static IPv6Address loopback() noexcept
    {
        return bytes_type{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    }

    friend bool operator==(const IPv6Address &, const IPv6Address &) noexcept =
        default;

    friend std::strong_ordering operator<=>(
        const IPv6Address &, const IPv6Address &) noexcept = default;

private:
    bytes_type _bytes;
    scope_id_type _scope_id;

    std::uint8_t _get(std::size_t n) const noexcept
    {
        return static_cast<std::uint8_t>(_bytes[n]);
    }

    friend IPv6AddressIterator;
};

inline IPv6Address make_ipv6_address(std::string_view str);

inline IPv6Address make_ipv6_address(
    std::string_view str, std::error_code &ec) noexcept;

template <std::output_iterator<char> O>
inline O to_chars(const IPv6Address &value, O out)
{
    detail::ipv6_address_to_chars(value.to_bytes(), out);
    return out;
}

template <class Alloc = std::allocator<char>>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const IPv6Address &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(res, std::back_inserter(res));
    return res;
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &stream, const IPv6Address &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, IPv6Address &value)
{
    if (!stream) {
        return stream;
    }

    IPv6Address::bytes_type bytes;
    scope_id_type scope_id;
    std::errc code = detail::try_parse_ipv6_address(
        std::istream_iterator<char>(stream), std::default_sentinel, bytes,
        scope_id);
    if (code == std::errc()) {
        value = { bytes, scope_id };
    } else {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}

class IPAddress {
public:
    IPAddress() noexcept : _is_v4(), _v4() {}

    explicit IPAddress(const IPv4Address &value) noexcept
        : _is_v4(true), _v4(value)
    {}

    explicit IPAddress(const IPv6Address &value) noexcept
        : _is_v4(false), _v6(value)
    {}

    IPAddress(const IPAddress &other) noexcept = default;

    IPAddress &operator=(const IPAddress &other) noexcept = default;

    bool is_v4() const noexcept
    {
        return _is_v4;
    }

    bool is_v6() const noexcept
    {
        return !_is_v4;
    }

    bool is_unspecified() const noexcept
    {
        return _is_v4 ? _v4.is_unspecified() : _v6.is_unspecified();
    }

    bool is_loopback() const noexcept
    {
        return _is_v4 ? _v4.is_loopback() : _v6.is_loopback();
    }

    bool is_multicast() const noexcept
    {
        return _is_v4 ? _v4.is_multicast() : _v6.is_multicast();
    }

    IPv4Address to_v4() const noexcept
    {
        return _v4;
    }

    IPv6Address to_v6() const noexcept
    {
        return _v6;
    }

private:
    bool _is_v4;
    union {
        IPv4Address _v4;
        IPv6Address _v6;
    };
};

inline bool operator==(const IPAddress &a, const IPAddress &b)
{
    if (a.is_v4() != b.is_v4()) {
        return false;
    } else if (a.is_v4()) {
        return a.to_v4() == b.to_v4();
    } else {
        return a.to_v6() == b.to_v6();
    }
}

inline std::strong_ordering operator<=>(const IPAddress &a, const IPAddress &b)
{
    if (a.is_v4() != b.is_v4()) {
        if (a.is_v4()) {
            return std::strong_ordering::less;
        } else {
            return std::strong_ordering::greater;
        }
    } else if (a.is_v4()) {
        return a.to_v4() <=> b.to_v4();
    } else {
        return a.to_v6() <=> b.to_v6();
    }
}

inline IPAddress make_ip_address(std::string_view str);

inline IPAddress make_ip_address(
    std::string_view str, std::error_code &ec) noexcept;

template <std::output_iterator<char> O>
inline O to_chars(const IPAddress &value, O out);

template <class Alloc = std::allocator<char>>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const IPAddress &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res));
    return res;
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &stream, const IPAddress &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, IPAddress &value);

template <class Alloc>
class BasicIPAddressIterator;

using IPv4AddressIterator = BasicIPAddressIterator<IPv4Address>;
using IPv6AddressIterator = BasicIPAddressIterator<IPv6Address>;

template <>
class BasicIPAddressIterator<IPv4Address> {
public:
    using value_type = IPv4Address;
    using difference_type = std::int64_t;
    using pointer = const IPv4Address *;
    using reference = const IPv4Address &;
    using iterator_category = std::input_iterator_tag;

    BasicIPAddressIterator() noexcept = default;

    BasicIPAddressIterator(const IPv4Address &address) noexcept
        : _address(address)
    {}

    reference operator*() const noexcept
    {
        return _address;
    }

    pointer operator->() const noexcept
    {
        return std::addressof(_address);
    }

    BasicIPAddressIterator &operator++() noexcept
    {
        return *this += 1;
    }

    BasicIPAddressIterator operator++(int) noexcept
    {
        auto copy(*this);
        ++*this;
        return copy;
    }

    BasicIPAddressIterator &operator--() noexcept
    {
        return *this -= 1;
    }

    BasicIPAddressIterator operator--(int) noexcept
    {
        auto copy(*this);
        --*this;
        return copy;
    }

    BasicIPAddressIterator &operator+=(difference_type n) noexcept
    {
        _address = IPv4Address(_address.to_uint() + n);
        return *this;
    }

    BasicIPAddressIterator &operator-=(difference_type n) noexcept
    {
        return *this += -n;
    }

    friend BasicIPAddressIterator operator+(
        const BasicIPAddressIterator &a, difference_type b) noexcept
    {
        return BasicIPAddressIterator(a) += b;
    }

    friend BasicIPAddressIterator operator+(
        difference_type a, const BasicIPAddressIterator &b) noexcept
    {
        return b + a;
    }

    friend BasicIPAddressIterator operator-(
        const BasicIPAddressIterator &a, difference_type b) noexcept
    {
        return BasicIPAddressIterator(a) -= b;
    }

    friend difference_type operator-(const BasicIPAddressIterator &a,
                                     const BasicIPAddressIterator &b) noexcept
    {
        return a._address.to_uint() - b._address.to_uint();
    }

private:
    IPv4Address _address;
};

template <>
class BasicIPAddressIterator<IPv6Address> {
public:
    using value_type = IPv6Address;
    using difference_type = std::int64_t;
    using pointer = const IPv6Address *;
    using reference = const IPv6Address &;
    using iterator_category = std::input_iterator_tag;

    BasicIPAddressIterator() noexcept = default;

    BasicIPAddressIterator(const IPv6Address &address) noexcept
        : _address(address)
    {}

    reference operator*() const noexcept
    {
        return _address;
    }

    pointer operator->() const noexcept
    {
        return std::addressof(_address);
    }

    BasicIPAddressIterator &operator++() noexcept
    {
        return *this += 1;
    }

    BasicIPAddressIterator operator++(int) noexcept
    {
        auto copy(*this);
        ++*this;
        return copy;
    }

    BasicIPAddressIterator &operator--() noexcept
    {
        return *this -= 1;
    }

    BasicIPAddressIterator operator--(int) noexcept
    {
        auto copy(*this);
        --*this;
        return copy;
    }

    BasicIPAddressIterator &operator+=(difference_type n) noexcept
    {
        n > 0 ? _add(n) : _sub(n);
        return *this;
    }

    BasicIPAddressIterator &operator-=(difference_type n) noexcept
    {
        n < 0 ? _add(n) : _sub(n);
        return *this;
    }

    friend BasicIPAddressIterator operator+(
        const BasicIPAddressIterator &a, difference_type b) noexcept
    {
        return BasicIPAddressIterator(a) += b;
    }

    friend BasicIPAddressIterator operator+(
        difference_type a, const BasicIPAddressIterator &b) noexcept
    {
        return b + a;
    }

    friend BasicIPAddressIterator operator-(
        const BasicIPAddressIterator &a, difference_type b) noexcept
    {
        return BasicIPAddressIterator(a) -= b;
    }

    friend difference_type operator-(const BasicIPAddressIterator &a,
                                     const BasicIPAddressIterator &b) noexcept
    {
        auto [hi_a, lo_a] = a._get_words();
        auto [hi_b, lo_b] = b._get_words();

        std::uint64_t carry = lo_b > lo_a;

        lo_a -= lo_b;
        hi_b += carry;
        carry = (hi_b < carry);
        carry += hi_a < hi_b;
        hi_a -= hi_b;
        lo_a -= carry;

        return {};
    }

    friend bool operator==(const BasicIPAddressIterator &,
                           const BasicIPAddressIterator &) noexcept = default;

    friend std::strong_ordering operator<=>(
        const BasicIPAddressIterator &,
        const BasicIPAddressIterator &) noexcept = default;

private:
    IPv6Address _address;

    BasicIPAddressIterator(std::uint64_t lo, std::uint64_t hi) noexcept
    {
        _set_words(lo, hi);
    }

    void _set_words(std::uint64_t lo, std::uint64_t hi) noexcept
    {
        store_unaligned_be64(_address._bytes.data() + 8, lo);
        store_unaligned_be64(_address._bytes.data(), hi);
    }

    std::pair<std::uint64_t, std::uint64_t> _get_words() const noexcept
    {
        return { load_unaligned_be64(_address._bytes.data() + 8),
                 load_unaligned_be64(_address._bytes.data()) };
    }

    void _add(std::uint64_t n) noexcept
    {
        auto [lo, hi] = _get_words();
        lo += n;
        n = lo < n;
        hi += n;
        lo += hi < n;
        _set_words(lo, hi);
    }

    void _sub(std::uint64_t n) noexcept
    {
        auto [lo, hi] = _get_words();
        std::uint64_t carry = lo < n;
        lo -= n;
        hi -= carry;
        lo -= carry < hi;
        _set_words(lo, hi);
    }
};

template <class Address>
class BasicIPAddressRange {
public:
    using iterator = BasicIPAddressIterator<Address>;

    BasicIPAddressRange() noexcept = default;

    BasicIPAddressRange(const Address &first, const Address &last) noexcept
        : _first(first), _last(last)
    {}

    iterator begin() const noexcept
    {
        return _first;
    }

    iterator end() const noexcept
    {
        return _last;
    }

    bool empty() const noexcept
    {
        return _first == _last;
    }

    size_t size() const noexcept
    {
        return _last - _first;
    }

    iterator find(const Address &address) const noexcept
    {
        return address >= *_first && address < *_last
                   ? iterator(address)
                   : _last;
    }

    explicit operator bool() noexcept
    {
        return _first != _last;
    }

    friend bool operator==(const BasicIPAddressRange &,
                           const BasicIPAddressRange &) noexcept = default;

private:
    iterator _first, _last;
};

using IPv4AddressRange = BasicIPAddressRange<IPv4Address>;
using IPv6AddressRange = BasicIPAddressRange<IPv6Address>;

class IPv4Network {
public:
    IPv4Network() noexcept = default;

    IPv4Network(const IPv4Address &address, unsigned int prefix_length)
        : _address(address), _prefix_length(prefix_length)
    {
        if (prefix_length > 32) {
            throw std::out_of_range(
                "htl::IPv4Network::IPv4Network(): Prefix length out of range");
        }
    }

    IPv4Network(const IPv4Address &address, const IPv4Address &mask)
        : _prefix_length()
    {
        IPv4Address::bytes_type address_bytes = address.to_bytes();
        IPv4Address::bytes_type mask_bytes = mask.to_bytes();
        for (int i = 0; i < 4; i++) {
            auto mask_value = mask_bytes[i];
            if (mask_value != std::byte(0xFF)) {
                auto bit_count = std::popcount(to_underlying(mask_value));
                auto bit_mask = std::byte(0xFF) >> bit_count;
                auto address_value = address_bytes[i];
                if ((address_value & bit_mask) != std::byte()) {
                    throw std::invalid_argument(
                        "htl::IPv4Network::IPv4Network(): Invalid mask");
                }
                address_bytes[i] = address_value & ~bit_mask;
                std::ranges::fill(
                    address_bytes.begin() + i + 1, address_bytes.end(),
                    std::byte());
            }
        }
        _address = mask_bytes;
    }

    IPv4Address address() const noexcept
    {
        return _address;
    }

    unsigned int prefix_length() const noexcept
    {
        return _prefix_length;
    }

    IPv4Address netmask() const noexcept
    {
        if (is_host()) {
            return IPv4Address::bytes_type{ 0xFF, 0xFF, 0xFF, 0xFF };
        }

        IPv4Address::bytes_type bytes;
        auto byte_count = _prefix_length >> 3;
        auto bit_count = _prefix_length & 7;
        auto bit_mask = ~(std::byte(0xFF) >> bit_count);

        std::ranges::fill_n(bytes.begin(), byte_count, std::byte(0xFF));
        bytes[byte_count] = ~bit_mask;
        std::ranges::fill(
            bytes.begin() + byte_count + 1, bytes.end(), std::byte());

        return bytes;
    }

    IPv4Address network() const noexcept
    {
        if (is_host()) {
            return _address;
        }

        IPv4Address::bytes_type bytes, address_bytes = _address.to_bytes();
        auto byte_count = _prefix_length >> 3;
        auto bit_count = _prefix_length & 7;
        auto bit_mask = std::byte(0xFF) >> bit_count;

        std::ranges::copy_n(address_bytes.begin(), byte_count, bytes.begin());
        bytes[byte_count] = address_bytes[byte_count] & ~bit_mask;
        std::ranges::fill(
            bytes.begin() + byte_count + 1, bytes.end(), std::byte());

        return bytes;
    }

    IPv4Address broadcast() const noexcept
    {
        if (is_host()) {
            return _address;
        }

        IPv4Address::bytes_type bytes, address_bytes = _address.to_bytes();
        auto byte_count = _prefix_length >> 3;
        auto bit_count = _prefix_length & 7;
        auto bit_mask = ~(std::byte(0xFF) >> bit_count);

        std::ranges::copy_n(address_bytes.begin(), byte_count, bytes.begin());
        bytes[byte_count] = (address_bytes[byte_count] & ~bit_mask) | bit_mask;
        std::ranges::fill(
            bytes.begin() + byte_count + 1, bytes.end(), std::byte(0xFF));

        return bytes;
    }

    IPv4Network canonical() const noexcept
    {
        return { network(), prefix_length() };
    }

    IPv4AddressRange hosts() const noexcept
    {
        if (is_host()) {
            return { _address, *++IPv4AddressIterator(_address) };
        }

        return { *++IPv4AddressIterator(network()), broadcast() };
    }

    bool is_host() const noexcept
    {
        return _prefix_length == 32;
    }

    bool is_subnet_of(const IPv4Network &other) const noexcept
    {
        return other.prefix_length() < prefix_length() &&
               IPv4Network(address(), other.prefix_length()).canonical() ==
                   other.canonical();
    }

    friend bool operator==(const IPv4Network &, const IPv4Network &) noexcept =
        default;

private:
    IPv4Address _address;
    std::uint8_t _prefix_length;
};

inline IPv4Network make_ipv4_network(std::string_view str)
{
    IPv4Address::bytes_type bytes;
    unsigned int len;
    auto first = str.begin();
    auto last = str.end();
    auto code = detail::try_parse_ipv4_network(first, last, bytes, len);

    if (code == std::errc() && first != last) {
        code = std::errc::invalid_argument;
    }

    switch (code) {
    case std::errc::invalid_argument:
        throw std::out_of_range(
            "htl::make_ipv4_network(): Invalid IPv4 network");
    case std::errc::result_out_of_range:
        throw std::out_of_range(
            "htl::make_ipv4_network(): Prefix length out of range");
    }

    return { bytes, len };
}

inline IPv4Network make_ipv4_network(
    std::string_view str, std::error_code &ec) noexcept
{
    IPv4Address::bytes_type bytes;
    unsigned int len;
    auto first = str.begin();
    auto last = str.end();
    auto code = detail::try_parse_ipv4_network(first, last, bytes, len);

    if (code == std::errc() && first != last) {
        code = std::errc::invalid_argument;
    }

    ec = std::make_error_code(code);
    return { bytes, len };
}

template <std::output_iterator<char> O>
inline O to_chars(const IPv4Network &value, O out)
{
    detail::ipv4_network_to_chars(
        value.address().to_bytes(), value.prefix_length(), out);
    return out;
}

template <class Alloc = std::allocator<char>>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const IPv4Network &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res));
    return res;
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &stream, const IPv4Network &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, IPv4Network &value)
{
    if (!stream) {
        return stream;
    }

    IPv4Address::bytes_type bytes;
    unsigned int prefix_length;
    std::errc code = detail::try_parse_ipv4_network(
        std::istream_iterator<char>(stream), std::default_sentinel, bytes,
        prefix_length);
    if (code == std::errc()) {
        value = { bytes, prefix_length };
    } else {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}

class IPv6Network {
public:
    IPv6Network() noexcept = default;

    IPv6Network(const IPv6Address &address, unsigned int prefix_length)
        : _address(address), _prefix_length(prefix_length)
    {
        if (prefix_length > 128) {
            throw std::out_of_range(
                "htl::IPv6Network::IPv6Network(): Prefix length out of range");
        }
    }

    IPv6Network(const IPv6Address &address, const IPv6Address &mask)
        : _prefix_length()
    {
        IPv6Address::bytes_type address_bytes = address.to_bytes();
        IPv6Address::bytes_type mask_bytes = mask.to_bytes();
        for (int i = 0; i < 16; i++) {
            auto mask_value = mask_bytes[i];
            if (mask_value != std::byte(0xFF)) {
                auto bit_count = std::popcount(to_underlying(mask_value));
                auto bit_mask = std::byte(0xFF) >> bit_count;
                auto address_value = address_bytes[i];
                if ((address_value & bit_mask) != std::byte()) {
                    throw std::invalid_argument(
                        "htl::IPv6Network::IPv6Network(): Invalid mask");
                }
                address_bytes[i] = address_value & ~bit_mask;
                std::ranges::fill(
                    address_bytes.begin() + i + 1, address_bytes.end(),
                    std::byte());
            }
        }
        _address = mask_bytes;
    }

    IPv6Address address() const noexcept
    {
        return _address;
    }

    unsigned int prefix_length() const noexcept
    {
        return _prefix_length;
    }

    IPv6Address netmask() const noexcept
    {
        if (is_host()) {
            return IPv6Address::bytes_type{
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };
        }

        IPv6Address::bytes_type bytes;
        auto byte_count = _prefix_length >> 3;
        auto bit_count = _prefix_length & 7;
        auto bit_mask = std::byte(0xFF) >> bit_count;

        std::ranges::fill_n(bytes.begin(), byte_count, std::byte(0xFF));
        bytes[byte_count] = ~bit_mask;
        std::ranges::fill(
            bytes.begin() + byte_count + 1, bytes.end(), std::byte());

        return bytes;
    }

    IPv6Address network() const noexcept
    {
        if (is_host()) {
            return _address;
        }

        IPv6Address::bytes_type bytes, address_bytes = _address.to_bytes();
        auto byte_count = _prefix_length >> 3;
        auto bit_count = _prefix_length & 7;
        auto bit_mask = std::byte(0xFF) >> bit_count;

        std::ranges::copy_n(address_bytes.begin(), byte_count, bytes.begin());
        bytes[byte_count] = address_bytes[byte_count] & ~bit_mask;
        std::ranges::fill(
            bytes.begin() + byte_count + 1, bytes.end(), std::byte());

        return bytes;
    }

    IPv6Address broadcast() const noexcept
    {
        if (is_host()) {
            return _address;
        }

        IPv6Address::bytes_type bytes, address_bytes = _address.to_bytes();
        auto byte_count = _prefix_length >> 3;
        auto bit_count = _prefix_length & 7;
        auto bit_mask = std::byte(0xFF) >> bit_count;

        std::ranges::copy_n(address_bytes.begin(), byte_count, bytes.begin());
        bytes[byte_count] = address_bytes[byte_count] & ~bit_mask;
        std::ranges::fill(
            bytes.begin() + byte_count + 1, bytes.end(), std::byte(0xFF));

        return bytes;
    }

    IPv6Network canonical() const noexcept
    {
        return { network(), prefix_length() };
    }

    IPv6AddressRange hosts() const noexcept
    {
        if (is_host()) {
            return { _address, *++IPv6AddressIterator(_address) };
        }

        return { *++IPv6AddressIterator(network()), broadcast() };
    }

    bool is_host() const noexcept
    {
        return _prefix_length == 128;
    }

    bool is_subnet_of(const IPv6Network &other) const noexcept
    {
        return other.prefix_length() < prefix_length() &&
               IPv6Network(address(), other.prefix_length()).canonical() ==
                   other.canonical();
    }

    friend bool operator==(const IPv6Network &, const IPv6Network &) noexcept =
        default;

private:
    IPv6Address _address;
    std::uint8_t _prefix_length;
};

inline IPv6Network make_ipv6_network(std::string_view str)
{
    IPv6Address::bytes_type bytes;
    unsigned int len;
    auto first = str.begin();
    auto last = str.begin();
    auto code = detail::try_parse_ipv6_network(first, last, bytes, len);

    if (code == std::errc()) {
        code = std::errc::invalid_argument;
    }

    switch (code) {
    case std::errc::invalid_argument:
        throw std::out_of_range(
            "htl::make_ipv6_network(): Invalid IPv6 network");
    case std::errc::result_out_of_range:
        throw std::out_of_range(
            "htl::make_ipv6_network(): Prefix length out of range");
    }

    return { bytes, len };
}

inline IPv6Network make_ipv6_network(
    std::string_view str, std::error_code &ec) noexcept
{
    IPv6Address::bytes_type bytes;
    unsigned int len;
    auto first = str.begin();
    auto last = str.begin();
    auto code = detail::try_parse_ipv6_network(first, last, bytes, len);

    if (code == std::errc()) {
        code = std::errc::invalid_argument;
    }

    ec = std::make_error_code(code);
    return { bytes, len };
}

template <std::output_iterator<char> O>
inline O to_chars(const IPv6Network &value, O out)
{
    detail::ipv6_network_to_chars(
        value.address().to_bytes(), value.prefix_length(), out);
    return out;
}

template <class Alloc = std::allocator<char>>
inline std::basic_string<char, std::char_traits<char>, Alloc>
to_string(const IPv6Network &value, const Alloc &alloc = Alloc())
{
    std::basic_string<char, std::char_traits<char>, Alloc> res(alloc);
    to_chars(value, std::back_inserter(res));
    return res;
}

template <class CharT, class Traits>
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &stream, const IPv6Network &value)
{
    to_chars(value, std::ostream_iterator<char>(stream));
    return stream;
}

template <class CharT, class Traits>
inline std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> &stream, IPv6Network &value)
{
    if (!stream) {
        return stream;
    }

    IPv6Address address;
    unsigned int prefix_length;
    std::errc code = detail::try_parse_ipv6_network(
        std::istream_iterator<char>(stream), std::default_sentinel, address,
        prefix_length);
    if (code == std::errc()) {
        value = { address, prefix_length };
    } else {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}

} // namespace htl

namespace std {

template <>
struct hash<htl::IPv4Address> {
    std::size_t operator()(const htl::IPv4Address &value) noexcept
    {
        return htl::detail::default_hash(value);
    }
};

template <>
struct hash<htl::IPv6Address> {
    std::size_t operator()(const htl::IPv6Address &value) noexcept
    {
        return htl::detail::default_hash(value);
    }
};

template <>
struct hash<htl::IPAddress> {
    std::size_t operator()(const htl::IPAddress &value) noexcept
    {
        if (value.is_v4()) {
            return htl::detail::default_hash(true, value.to_v4());
        } else {
            return htl::detail::default_hash(false, value.to_v6());
        }
    }
};

template <>
struct hash<htl::IPv4Network> {
    std::size_t operator()(const htl::IPv4Network &value) noexcept
    {
        return htl::detail::default_hash(
            value.address(), value.prefix_length());
    }
};

template <>
struct hash<htl::IPv6Network> {
    std::size_t operator()(const htl::IPv6Network &value) noexcept
    {
        return htl::detail::default_hash(
            value.address(), value.prefix_length());
    }
};

} // namespace std

#endif
