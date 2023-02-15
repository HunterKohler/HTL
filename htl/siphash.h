#ifndef HTL_SIPHASH_H_
#define HTL_SIPHASH_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <htl/unaligned.h>

namespace htl {

template <std::size_t C, std::size_t D>
class BasicSipHash {
public:
    static constexpr std::size_t key_size = 16;
    static constexpr std::size_t block_size = 8;
    static constexpr std::size_t digest_size = 8;

    static void hash(const std::uint8_t *data, std::size_t data_size,
                     const std::uint8_t *key, std::uint8_t *dest) noexcept
    {
        BasicSipHash context{ key };

        context.update(data, data_size);
        context.finalize(dest);
    }

    explicit BasicSipHash(const std::uint8_t *key) noexcept
    {
        reset(key);
    }

    void reset(const std::uint8_t *key) noexcept
    {
        std::uint64_t k0 = load_unaligned_le64(key);
        std::uint64_t k1 = load_unaligned_le64(key + 8);

        _v0 = k0 ^ 0x736F6D6570736575ULL;
        _v1 = k1 ^ 0x646F72616E646F6DULL;
        _v2 = k0 ^ 0x6C7967656E657261ULL;
        _v3 = k1 ^ 0x7465646279746573ULL;
        _size = 0;
    }

    void update(const std::uint8_t *data, std::size_t data_size) noexcept
    {
        if (!data_size) {
            return;
        }

        std::size_t buffer_size = _size & (block_size - 1);
        _size += data_size;

        if (buffer_size) {
            std::uint8_t *buffer_end = _buffer + buffer_size;
            std::size_t buffer_rem = block_size - buffer_size;

            if (buffer_rem > data_size) {
                std::ranges::copy_n(data, data_size, buffer_end);
                return;
            }

            std::ranges::copy_n(data, buffer_rem, buffer_end);
            data += buffer_rem;
            data_size -= buffer_rem;
            _update_state(_buffer);
        }

        const std::uint8_t *data_end = data + data_size;
        const std::uint8_t *data_lim = data_end - block_size;

        for (; data <= data_lim; data += block_size) {
            _update_state(data);
        }

        std::ranges::copy(data, data_end, _buffer);
    }

    void finalize(std::uint8_t *dest)
    {
        std::size_t buffer_size = _size & (block_size - 1);
        std::uint8_t *buffer_end = _buffer + buffer_size;

        std::ranges::fill(buffer_end, std::ranges::end(_buffer) - 1, 0);

        _buffer[block_size - 1] = _size & 0xFF;

        _update_state(_buffer);

        _v2 ^= 0xFF;

        for (int i = 0; i < D; i++) {
            _sipround();
        }

        std::uint64_t digest = _v0 ^ _v1 ^ _v2 ^ _v3;
        store_unaligned_le64(dest, digest);
    }

private:
    std::size_t _size;
    std::uint8_t _buffer[block_size];
    std::uint64_t _v0, _v1, _v2, _v3;

    void _update_state(const std::uint8_t *data) noexcept
    {
        std::uint64_t m = load_unaligned_le64(data);
        _v3 ^= m;

        for (int i = 0; i < C; i++) {
            _sipround();
        }

        _v0 ^= m;
    }

    void _sipround() noexcept
    {
        _v0 += _v1;
        _v1 = std::rotl(_v1, 13);
        _v1 ^= _v0;
        _v0 = std::rotl(_v0, 32);

        _v2 += _v3;
        _v3 = std::rotl(_v3, 16);
        _v3 ^= _v2;

        _v2 += _v1;
        _v1 = std::rotl(_v1, 17);
        _v1 ^= _v2;
        _v2 = std::rotl(_v2, 32);

        _v0 += _v3;
        _v3 = std::rotl(_v3, 21);
        _v3 ^= _v0;
    }
};

using SipHash = BasicSipHash<2, 4>;

} // namespace htl

#endif
