#ifndef HTL_CONFIG_H_
#define HTL_CONFIG_H_

#if defined(__GNUC__) || defined(__GNUG__)
#define HTL_GCC 1
#else
#define HTL_GCC 0
#endif

#if defined(__clang__)
#define HTL_CLANG 1
#else
#define HTL_CLANG 0
#endif

#if defined(_MSC_VER)
#define HTL_MSVC 1
#else
#define HTL_MSVC 0
#endif

#if HTL_GCC || HTL_CLANG
#define HTL_HAS_BUILTIN(x) __has_builtin(x)
#else
#define HTL_HAS_BUILTIN(x) 0
#endif

#if HTL_GCC
#define HTL_ATTR_PACKED [[gnu::packed]]
#elif HTL_CLANG
#define HTL_ATTR_PACKED [[clang::packed]]
#elif HTL_MSVC
#define HTL_ATTR_PACKED
#else
#define HTL_ATTR_PACKED
#endif

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define HTL_WINDOWS 1
#else
#define HTL_WINDOWS 0
#endif

#if defined(__SIZEOF_INT128__) && \
    ((HTL_CLANG && !HTL_WINDOWS) || (HTL_GCC && !HTL_CLANG))
#define HTL_HAS_INTRINSIC_INT128 1
#elif
#define HTL_HAS_INTRINSIC_INT128 0
#endif

/**
 * HTL namespace
 */
namespace htl {}

#endif
