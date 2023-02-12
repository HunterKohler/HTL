#ifndef HTL_CONTRACT_H_
#define HTL_CONTRACT_H_

#include <source_location>
#include <htl/config.h>
#include <htl/detail/contract.h>

#if !defined(NDEBUG) && (!defined(HTL_DEBUG) || HTL_DEBUG)
#define HTL_CONTRACT(x)                                \
    do {                                               \
        ::htl::detail::handle_contract(                \
            (x), #x, std::source_location::current()); \
    } while (0)
#else
#define HTL_CONTRACT(x) \
    do {                \
    } while (0)
#endif

#endif
