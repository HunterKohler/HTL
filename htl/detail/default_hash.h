#ifndef HTL_DETAIL_DEFAULT_HASH_H_
#define HTL_DETAIL_DEFAULT_HASH_H_

#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <htl/siphash.h>
#include <htl/unaligned.h>

namespace htl::detail {

class DefaultHasher {
private:
    static constexpr std::uint8_t siphash_key[SipHash::key_size]{
        0xF1, 0x64, 0x5D, 0x48, 0x73, 0xC2, 0x45, 0x6B,
        0xE2, 0x97, 0x70, 0x6E, 0xC6, 0xE4, 0xA9, 0xF5,
    };

    SipHash _base;

public:
    DefaultHasher() noexcept : _base(siphash_key) {}

    auto &reset() noexcept
    {
        _base.reset(siphash_key);
        return *this;
    }

    template <class T>
        requires std::has_unique_object_representations_v<T>
    auto &update(const T &value) noexcept
    {
        _base.update(reinterpret_cast<const std::byte *>(std::addressof(value)),
                     sizeof(value));
        return *this;
    }

    template <std::ranges::input_range R>
        requires std::has_unique_object_representations_v<
            std::ranges::range_value_t<R>>
    auto &update_range(R &&r)
    {
        return update_range(std::ranges::begin(r), std::ranges::end(r));
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
        requires std::has_unique_object_representations_v<std::iter_value_t<I>>
    auto &update_range(I first, S last)
    {
        if constexpr (std::contiguous_iterator<I> &&
                      std::sized_sentinel_for<S, I>) {
            _base.update(
                reinterpret_cast<const std::byte *>(std::addressof(*first)),
                sizeof(std::iter_value_t<I>) *
                    std::ranges::distance(first, last));
        } else {
            for (; first != last; ++first) {
                decltype(auto) value = *first;
                update(value);
            }
        }

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
    requires(std::has_unique_object_representations<Args> && ...)
inline std::size_t default_hash(const Args &...args) noexcept
{
    DefaultHasher hash();
    (hash.update(args), ...);
    return hash.digest();
}

} // namespace htl::detail

#endif
