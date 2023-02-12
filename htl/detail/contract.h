#ifndef HTL_DETAIL_CONTRACT_H_
#define HTL_DETAIL_CONTRACT_H_

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <source_location>

namespace htl::detail {

inline constexpr char handle_contract_fmt[] =
    "%s:%" PRIuLEAST32 ":%" PRIuLEAST32 ": %s: contract violation: (%s)\n";

constexpr void handle_contract(
    bool cond, const char *expr, const std::source_location &src) noexcept
{
    if (!cond) [[unlikely]] {
        std::fprintf(stderr, handle_contract_fmt, src.file_name(), src.line(),
                     src.column(), src.function_name(), expr);
        std::fflush(stderr);
        std::abort();
    }
}

} // namespace htl::detail

#endif
