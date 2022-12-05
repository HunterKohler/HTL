#ifndef HTL_DETAIL_MDXHASH_H_
#define HTL_DETAIL_MDXHASH_H_

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <htl/unaligned.h>

namespace htl::detail {

template <class T>
class MDXHash {
public:
    static constexpr std::size_t block_size = T::block_size;
    static constexpr std::endian byte_order = T::byte_order;

    MDXHash() noexcept
    {
        reset();
    }

    void reset() noexcept
    {
        _state.reset();
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
            _state.transform(_buffer);
        }

        const std::uint8_t *data_end = data + data_size;
        const std::uint8_t *data_lim = data_end - block_size;

        for (; data <= data_lim; data += block_size) {
            _state.transform(data);
        }

        std::ranges::copy(data, data_end, _buffer);
    }

    void finalize(std::uint8_t *dest) noexcept
    {
        std::size_t prelen_size = block_size - 8;
        std::size_t buffer_size = _size & (block_size - 1);
        std::uint8_t *buffer_end = _buffer + buffer_size + 1;
        std::uint8_t *buffer_prelen_end = _buffer + prelen_size;
        std::size_t bit_size = _size << 3;

        _buffer[buffer_size] = 0x80;

        if (buffer_size < prelen_size) {
            std::ranges::fill(buffer_end, buffer_prelen_end, 0);
        } else {
            std::ranges::fill(buffer_end, std::ranges::end(_buffer), 0);
            _state.transform(_buffer);
            std::ranges::fill(_buffer, buffer_prelen_end, 0);
        }

        if constexpr (byte_order == std::endian::little) {
            store_unaligned_le64(buffer_prelen_end, bit_size);
        } else {
            store_unaligned_be64(buffer_prelen_end, bit_size);
        }

        _state.transform(_buffer);
        _state.digest(dest);
    }

private:
    std::size_t _size;
    std::uint8_t _buffer[block_size];
    T _state;
};

} // namespace htl::detail

#endif
