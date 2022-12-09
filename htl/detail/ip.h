#ifndef HTL_DETAIL_IP_H_
#define HTL_DETAIL_IP_H_

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <system_error>
#include <net/if.h>
#include <netinet/in.h>
#include <htl/ascii.h>
#include <htl/detail/iterator.h>
#include <htl/scope_guard.h>
#include <htl/utility.h>

namespace htl::detail {

inline std::errc try_parse_ipv4_address(auto &&first, auto &&last, auto &bytes)
{
    for (int i = 0; i < 4; i++) {
        if (first == last || !ascii_isdigit(*first)) {
            return std::errc::invalid_argument;
        }

        int value = *first - '0';
        if (++first == last && value && ascii_isdigit(*first)) {
            value = 10 * value + *first - '0';
            if (++first == last && ascii_isdigit(*first)) {
                value = 10 * value + *first - '0';
                if (value > 0xFF) {
                    value /= 10;
                } else {
                    ++first;
                }
            }
        }

        if (i < 3 && (first == last || *first != '.')) {
            return std::errc::invalid_argument;
        }

        bytes[i] = std::byte(value);
    }

    return {};
}

inline std::errc try_parse_ipv6_address(
    auto &&first, auto &&last, auto &bytes, auto &scope_id);

inline std::errc try_parse_ipv4_network(
    auto &&first, auto &&last, auto &bytes, auto &prefix_length)
{
    auto code = try_parse_ipv4_address(first, last, bytes);
    if (code != std::errc()) {
        return code;
    } else if (first == last || *first != '/') {
        prefix_length = 32;
    } else if (++first == last || !ascii_isdigit(*first)) {
        return std::errc::invalid_argument;
    } else if ((prefix_length = *first - '0') && ++first != last &&
               ascii_isdigit(*first) &&
               (prefix_length * 10 + *first - '0') <= 32) {
        prefix_length = prefix_length * 10 + *first - '0';
        ++first;
    }

    return {};
}

inline std::errc try_parse_ipv6_network(
    auto &&first, auto &&last, auto &bytes, auto &prefix_length)
{
    auto code = try_parse_ipv4_address(first, last, bytes);
    if (code != std::errc()) {
        return code;
    } else if (first == last || *first != '/') {
        prefix_length = 128;
    } else if (++first == last || !ascii_isdigit(*first)) {
        return std::errc::invalid_argument;
    } else if ((prefix_length = *first - '0') && ++first != last &&
               ascii_isdigit(*first)) {
        prefix_length = prefix_length * 10 + *first - '0';
        if (++first != last && ascii_isdigit(*first) &&
            (prefix_length * 10 + *first - '0') <= 128) {
            prefix_length = prefix_length * 10 + *first - '0';
            ++first;
        }
    }

    return {};
}

inline void ipv4_address_to_chars(auto &bytes, auto &out)
{
    iter_write_int(to_underlying(bytes[0]), out);
    iter_write('.', out);
    iter_write_int(to_underlying(bytes[1]), out);
    iter_write('.', out);
    iter_write_int(to_underlying(bytes[2]), out);
    iter_write('.', out);
    iter_write_int(to_underlying(bytes[3]), out);
}

inline auto ipv6_address_find_zeros(auto &bytes)
{
    int max_start = 0;
    int max_size = 0;
    for (int i = 0, j = 0; i <= 16; i += 2) {
        if (bytes[i] != std::byte() || i == 16) {
            int sz = i - j;
            if (sz > max_size) {
                max_start = j;
                max_size = sz;
            }
            j = i;
        }
    }
    return std::pair{ max_start, max_start + max_size };
}

inline void ipv6_address_to_chars_segment(auto &bytes, int pos, auto &out)
{
    auto a = to_underlying(bytes[pos]);
    auto b = to_underlying(bytes[pos + 1]);
    char buf[4] = {
        detail::hex_charset_lower[a >> 4], detail::hex_charset_lower[a & 15],
        detail::hex_charset_lower[b >> 4], detail::hex_charset_lower[b & 15]
    };

    auto ptr = buf;
    for (; ptr < buf + 3 && *ptr == '0';) {
        ++ptr;
    }

    for (; ptr < std::end(buf); ++ptr) {
        iter_write(*ptr, out);
    }
}

inline void ipv6_address_to_chars_segments(auto &bytes, auto &out)
{
    auto [start, stop] = ipv6_address_find_zeros(bytes);

    if (!stop) {
        for (int i = 0; i < 14; i += 2) {
            ipv6_address_to_chars_segment(bytes, i, out);
            iter_write(':', out);
        }

        ipv6_address_to_chars_segment(bytes, 14, out);
    } else if (stop == 16) {
        iter_write(':', out);
        iter_write(':', out);
    } else {
        for (int i = 0; i < start; i += 2) {
            hlib_ipv6_address_to_string_segment(bytes, i, out);
            iter_write(':', out);
        }

        if (start == 0 || stop == 16) {
            iter_write(':', out);
        }

        for (int i = stop; i < 16; i += 2) {
            iter_write(':', out);
            hlib_ipv6_address_to_string_segment(bytes, i, out);
        }
    }
}

inline std::errc ipv6_address_to_chars_interface(auto scope_id, auto &out)
{
    ErrnoScopeGuard guard;
    char name[IF_NAMESIZE];

    if (!if_indextoname(scope_id, name)) {
        return std::errc(errno);
    }

    iter_write_string(name, out);
    return {};
}

inline std::errc ipv6_address_to_chars(auto &bytes, auto scope_id, auto &out)
{
    ipv6_address_to_chars_segments(bytes, out);

    if (scope_id) {
        iter_write('/', out);
        return ipv6_address_to_chars_interface(scope_id, out);
    }

    return {};
}

inline void ipv4_network_to_chars(auto &bytes, auto prefix_length, auto &out)
{
    ipv4_address_to_chars(bytes, out);
    iter_write('/', out);
    iter_write_int(prefix_length, out);
}

inline void ipv6_network_to_chars(auto &bytes, auto prefix_length, auto &out)
{
    ipv6_address_to_chars_segments(bytes, out);
    iter_write('/', out);
    iter_write_int(prefix_length, out);
}

} // namespace htl::detail

#endif
