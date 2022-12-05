#ifndef HTL_MD4_H_
#define HTL_MD4_H_

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <htl/detail/md4_state.h>
#include <htl/detail/mdx_hash.h>

namespace htl {

class MD4 {
public:
    static constexpr std::size_t key_size = 0;
    static constexpr std::size_t block_size = 64;
    static constexpr std::size_t digest_size = 16;

    static void hash(const std::uint8_t *data, std::size_t data_size,
                     std::uint8_t *dest) noexcept
    {
        MD4 context;

        context.update(data, data_size);
        context.finalize(dest);
    }

    MD4() noexcept = default;

    void reset() noexcept
    {
        _hash.reset();
    }

    void update(const std::uint8_t *data, std::size_t data_size) noexcept
    {
        _hash.update(data, data_size);
    }

    void finalize(std::uint8_t *dest) noexcept
    {
        _hash.finalize(dest);
    }

private:
    detail::MDXHash<detail::MD4State> _hash;
};

} // namespace htl

#endif
