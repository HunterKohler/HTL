#ifndef HTL_SHARED_LIBRARY_H_
#define HTL_SHARED_LIBRARY_H_

#include <concepts>
#include <cstdint>
#include <ranges>
#include <stdexcept>
#include <string>
#include <dlfcn.h>
#include <htl/detail/bitmask.h>
#include <htl/utility.h>

namespace htl {

class SharedLibraryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    using std::runtime_error::operator=;
};

enum class SharedLibaryOptions {
    Lazy = RTLD_LAZY,
    Now = RTLD_NOW,
    Global = RTLD_GLOBAL,
    Local = RTLD_LOCAL,
    NoDelete = RTLD_NODELETE,
    NoLoad = RTLD_NOLOAD,
#ifdef RTLD_DEEPBIND
    DeepBind = RTLD_DEEPBIND,
#endif
#ifdef RTLD_FIRST
    First = RTLD_FIRST,
#endif
};

constexpr SharedLibaryOptions operator&(
    SharedLibaryOptions x, SharedLibaryOptions y) noexcept
{
    return detail::bitmask_and(x, y);
}

constexpr SharedLibaryOptions operator|(
    SharedLibaryOptions x, SharedLibaryOptions y) noexcept
{
    return detail::bitmask_or(x, y);
}

constexpr SharedLibaryOptions operator^(
    SharedLibaryOptions x, SharedLibaryOptions y) noexcept
{
    return detail::bitmask_xor(x, y);
}

constexpr SharedLibaryOptions operator~(SharedLibaryOptions x) noexcept
{
    return detail::bitmask_not(x);
}

constexpr SharedLibaryOptions &operator&=(
    SharedLibaryOptions &x, SharedLibaryOptions y) noexcept
{
    return x = x & y;
}

constexpr SharedLibaryOptions &operator|=(
    SharedLibaryOptions &x, SharedLibaryOptions y) noexcept
{
    return x = x | y;
}

constexpr SharedLibaryOptions &operator^=(
    SharedLibaryOptions &x, SharedLibaryOptions y) noexcept
{
    return x = x ^ y;
}

class SharedLibrary {
public:
    using native_handle_type = void *;

    SharedLibrary() noexcept : _handle() {}

    explicit SharedLibrary(const char *path, SharedLibraryOptions opts = {})
        : _handle(_open(path, opts))
    {}

    explicit SharedLibrary(
        const std::string &path, SharedLibaryOptions opts = {})
        : SharedLibrary(path.c_str(), opts)
    {}

    SharedLibrary(const SharedLibrary &) = delete;

    SharedLibrary(SharedLibrary &&other) noexcept : _handle(other._handle)
    {
        other._handle = nullptr;
    }

    SharedLibrary &operator=(const SharedLibrary &) = delete;

    SharedLibrary &operator=(SharedLibrary &&other)
    {
        _destroy();
        _handle = other._handle;
        other._handle = nullptr;
        return *this;
    }

    ~SharedLibrary() noexcept
    {
        _destroy();
    }

    void reset()
    {
        if (_handle) {
            _close(_handle);
            _handle = nullptr;
        }
    }

    void reset(const char *path, SharedLibaryOptions opts = {})
    {
        auto prev = _handle;
        try {
            _handle = _open(path, opts);
        } catch (...) {
            _handle = prev;
            throw;
        }
    }

    void reset(const std::string &path, SharedLibaryOptions opts = {})
    {
        reset(path.c_str(), opts);
    }

    bool has_symbol(std::nullptr_t) const = delete;

    bool has_symbol(const char *name) const
    {
        ::dlerror();
        ::dlsym(_handle, name);
        return !::dlerror();
    }

    bool has_symbol(const std::string &name) const
    {
        return has_symbol(name.c_str());
    }

    template <class T = void>
    T *get_symbol(std::nullptr_t) const = delete;

    template <class T = void>
    T *get_symbol(const char *name) const
    {
        ::dlerror();
        void *sym = ::dlsym(_handle, name);
        const char *msg = ::dlerror();
        if (msg) {
            throw SharedLibraryError(msg);
        }
        return sym;
    }

    template <class T = void>
    T *get_symbol(const std::string &name) const
    {
        return get_symbol(name.c_str());
    }

    const char *pathname() const
    {
        ::Dl_info info;
        if (::dladdr(_handle, std::addressof(info))) {
            throw SharedLibraryError(::dlerror());
        }
        return info.dli_fname;
    }

    native_handle_type native_handle() const noexcept
    {
        return _handle;
    }

    bool loaded() const noexcept
    {
        return _handle;
    }

    explicit operator bool() const noexcept
    {
        return _handle;
    }

private:
    native_handle_type _handle;

    void _destroy() noexcept
    {
        if (_handle) {
            _close(_handle);
        }
    }

    static void *_open(const char *path, SharedLibaryOptions opts)
    {
        if (to_underlying(~opts & SharedLibaryOptions::Now)) {
            opts |= SharedLibaryOptions::Lazy;
        }

        if (to_underlying(~opts & SharedLibaryOptions::Global)) {
            opts |= SharedLibaryOptions::Local;
        }

        void *handle = ::dlopen(path, to_underlying(opts));

        if (!handle) {
            throw SharedLibraryError(::dlerror());
        }

        return handle;
    }

    static void _close(void *handle)
    {
        if (::dlclose(handle)) {
            throw SharedLibraryError(::dlerror());
        }
    }
};

inline bool operator==(const SharedLibrary &a, const SharedLibrary &b) noexcept
{
    return a.native_handle() == b.native_handle();
}

} // namespace htl

#endif
