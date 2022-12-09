#ifndef HTL_DETAIL_DEFAULT_HASH_H_
#define HTL_DETAIL_DEFAULT_HASH_H_

#include <concepts>
#include <cstddef>
#include <htl/siphash.h>
#include <htl/unaligned.h>

namespace htl::detail {

template <class T>
inline constexpr bool can_default_hash =
    std::is_trivially_copyable_v<std::remove_reference_t<T>> &&
    std::has_unique_object_representations_v<std::remove_reference_t<T>>;

class DefaultHasher {
private:
    static constexpr std::uint8_t siphash_key[SipHash::key_size]{
        0xF1, 0x64, 0x5D, 0x48, 0x73, 0xC2, 0x45, 0x6B,
        0xE2, 0x97, 0x70, 0x6E, 0xC6, 0xE4, 0xA9, 0xF5,
    };

    SipHash _base;

    template <class T>
    void _update_one(T &&value) noexcept
    {
        _base.update(
            reinterpret_cast<const std::uint8_t *>(std::addressof(value)),
            sizeof(std::remove_reference_t<T>));
    }

public:
    DefaultHasher() noexcept : _base(siphash_key) {}

    void reset() noexcept
    {
        _base.reset(siphash_key);
    }

    template <class... Args>
        requires(can_default_hash<Args> && ...)
    DefaultHasher &update(Args &&...args) noexcept
    {
        (_update_one(std::forward<Args>(args)), ...);
        return *this;
    }

    std::uint64_t digest() noexcept
    {
        std::uint8_t dest[SipHash::digest_size];
        _base.finalize(dest);
        return load_unaligned_le64(dest);
    }
};

template <class... Args>
    requires(can_default_hash<Args> && ...)
inline std::size_t default_hash(Args &&...args) noexcept
{
    return DefaultHasher().update(std::forward<Args>(args)...).digest();
}

} // namespace htl::detail

#endif
