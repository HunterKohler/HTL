#ifndef HTL_CONTRACT_H_
#define HTL_CONTRACT_H_

#include <source_location>
#include <htl/config.h>
#include <htl/detail/contract.h>
#include <htl/utility.h>

#if HTL_DEBUG
#define HTL_EXPECTS(x)                                                 \
    do {                                                               \
        ::htl::detail::handle_contract(                                \
            (x), #x, "precondition", std::source_location::current()); \
    } while (0)

#define HTL_ENSURES(x)                                                  \
    do {                                                                \
        ::htl::detail::handle_contract(                                 \
            (x), #x, "postcondition", std::source_location::current()); \
    } while (0)
#elif HTL_HAS_BUILTIN(__builtin_assume)
#define HTL_EXPECTS(x)         \
    do {                       \
        __builtin_assume((x)); \
    } while (0)

#define HTL_ENSURES(x) HTL_EXPECTS((x))
#elif HTL_HAS_BUILTIN(__builtin_unreachable)
#define HTL_EXPECTS(x)               \
    do {                             \
        if (!(x)) {                  \
            __builtin_unreachable(); \
        }                            \
    } while (0)

#define HTL_ENSURES(x) HTL_EXPECTS((x))
#elif HTL_MSVC
#define HTL_EXPECTS(x) \
    do {               \
        __assume((x)); \
    } while (0)

#define HTL_ENSURES(x) HTL_EXPECTS((x))
#endif

#endif
