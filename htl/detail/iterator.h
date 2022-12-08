#ifndef HTL_DETAIL_ITERATOR_H_
#define HTL_DETAIL_ITERATOR_H_

#include <charconv>
#include <concepts>
#include <limits>
#include <utility>
#include <htl/ascii.h>

namespace htl::detail {

void iter_write(auto &&value, auto &out)
{
    *out = std::forward<decltype(value)>(value);
    ++out;
}

void iter_write_int(std::integral auto value, auto &out)
{
    char buf[std::numeric_limits<decltype(value)>::digits10 + 2];
    auto res = std::to_chars(buf, std::end(buf), value);
    for (auto ptr = buf; ptr != res.ptr; ++ptr) {
        iter_write(*ptr, out);
    }
}

void iter_write_float(std::floating_point auto value, auto &out)
{
    char buf[std::numeric_limits<decltype(value)>::max_digits10 + 2];
    auto res = std::to_chars(buf, std::end(buf), value);
    for (auto ptr = buf; ptr != res.ptr; ++ptr) {
        iter_write(*ptr, out);
    }
}

} // namespace htl::detail

#endif
