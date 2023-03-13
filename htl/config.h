#ifndef HTL_CONFIG_H_
#define HTL_CONFIG_H_

#if defined(__GNUC__) || defined(__GNUG__)
#define HTL_GCC 1
#endif

#if defined(__clang__)
#define HTL_CLANG 1
#endif

#if defined(_MSC_VER)
#define HTL_MSVC 1
#endif

#if defined(__has_builtin)
#define HTL_HAS_BUILTIN(x) __has_builtin(x)
#else
#define HTL_HAS_BUILTIN(x) 0
#endif

#if defined(__has_cpp_attribute)
#define HTL_HAS_ATTR(x) __has_cpp_attribute(x)
#else
#define HTL_HAS_ATTR(x) 0
#endif

#if HTL_HAS_ATTR(gnu::warning)
#define HTL_ATTR_WARNING(x) [[gnu::warning(x)]]
#elif HTL_HAS_ATTR(clang::warning)
#define HTL_ATTR_WARNING(x) [[clang::warning(x)]]
#else
#define HTL_ATTR_WARNING(x)
#endif

#if HTL_HAS_ATTR(gnu::packed)
#define HTL_ATTR_PACKED [[gnu::packed]]
#elif HTL_HAS_ATTR(clang::packed)
#define HTL_ATTR_PACKED [[clang::packed]]
#else
#define HTL_ATTR_PACKED \
    HTL_ATTR_WARNING("attribute 'packed' not supported by compiler")
#endif

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define HTL_WINDOWS 1
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__bsdi__) || defined(__DragonFly__)
#define HTL_BSD 1
#endif

#if defined(__linux__) || defined(__linux)
#define HTL_LINUX 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define HTL_MACH 1
#endif

#if defined(__ANDROID__)
#define HTL_ANDROID
#endif

#if defined(__SIZEOF_INT128__) && \
    ((HTL_CLANG && !HTL_WINDOWS) || (HTL_GCC && !HTL_CLANG))
#define HTL_HAS_INTRINSIC_INT128 1
#endif

#if !defined(HTL_INLINE_ASM)
#define HTL_INLINE_ASM 1
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || \
    defined(__amd64) || defined(_M_AMD64)
#define HTL_ARCH_X86_64 1
#elif defined(__i386__) || defined(__i386) || defined(_M_I86) || \
    defined(_M_IX86)
#define HTL_ARCH_x86 1
#elif defined(__mips__) || defined(__mips)
#define HTL_ARCH_MIPS 1
#elif defined(__powerpc) || defined(__powerpc__) || defined(_M_PPC)
#define HTL_ARCH_POWERPC 1
#elif defined(__aarch64__)
#define HTL_ARCH_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
#define HTL_ARCH_ARM 1
#elif defined(__riscv)
#define HTL_ARCH_RISCV 1
#elif defined(__ia64__) || defined(__ia64) || defined(_IA64) || \
    defined(__IA64__) || defined(__itanium__) || defined(_M_IA64)
#define HTL_ARCH_IA64 1
#elif defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#define HTL_ARCH_ALPHA 1
#endif

#if !defined(HTL_DEBUG) && defined(NDEBUG)
#define HTL_DEBUG 1
#endif

/**
 * HTL namespace
 */
namespace htl {}

#endif
