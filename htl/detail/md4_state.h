#ifndef HTL_DETAIL_MD4_STATE_H_
#define HTL_DETAIL_MD4_STATE_H_

#include <bit>
#include <cstdint>
#include <htl/unaligned.h>

namespace htl::detail {

struct MD4State {
    static constexpr std::size_t block_size = 64;
    static constexpr std::endian byte_order = std::endian::little;

    std::uint32_t a, b, c, d;

    void reset() noexcept
    {
        a = 0x67452301;
        b = 0xEFCDAB89;
        c = 0x98BADCFE;
        d = 0x10325476;
    }

    void transform(const std::uint8_t *block) noexcept
    {
        std::uint32_t aa = a, bb = b, cc = c, dd = d, x[16];

        for (int i = 0; i < 16; i++) {
            x[i] = load_unaligned_le32(block + 4 * i);
        }

        // Round 1
        FF(a, b, c, d, x[0], 3);
        FF(d, a, b, c, x[1], 7);
        FF(c, d, a, b, x[2], 11);
        FF(b, c, d, a, x[3], 19);
        FF(a, b, c, d, x[4], 3);
        FF(d, a, b, c, x[5], 7);
        FF(c, d, a, b, x[6], 11);
        FF(b, c, d, a, x[7], 19);
        FF(a, b, c, d, x[8], 3);
        FF(d, a, b, c, x[9], 7);
        FF(c, d, a, b, x[10], 11);
        FF(b, c, d, a, x[11], 19);
        FF(a, b, c, d, x[12], 3);
        FF(d, a, b, c, x[13], 7);
        FF(c, d, a, b, x[14], 11);
        FF(b, c, d, a, x[15], 19);

        // Round 2
        GG(a, b, c, d, x[0], 3);
        GG(d, a, b, c, x[4], 5);
        GG(c, d, a, b, x[8], 9);
        GG(b, c, d, a, x[12], 13);
        GG(a, b, c, d, x[1], 3);
        GG(d, a, b, c, x[5], 5);
        GG(c, d, a, b, x[9], 9);
        GG(b, c, d, a, x[13], 13);
        GG(a, b, c, d, x[2], 3);
        GG(d, a, b, c, x[6], 5);
        GG(c, d, a, b, x[10], 9);
        GG(b, c, d, a, x[14], 13);
        GG(a, b, c, d, x[3], 3);
        GG(d, a, b, c, x[7], 5);
        GG(c, d, a, b, x[11], 9);
        GG(b, c, d, a, x[15], 13);

        // Round 3
        HH(a, b, c, d, x[0], 3);
        HH(d, a, b, c, x[8], 9);
        HH(c, d, a, b, x[4], 11);
        HH(b, c, d, a, x[12], 15);
        HH(a, b, c, d, x[2], 3);
        HH(d, a, b, c, x[10], 9);
        HH(c, d, a, b, x[6], 11);
        HH(b, c, d, a, x[14], 15);
        HH(a, b, c, d, x[1], 3);
        HH(d, a, b, c, x[9], 9);
        HH(c, d, a, b, x[5], 11);
        HH(b, c, d, a, x[13], 15);
        HH(a, b, c, d, x[3], 3);
        HH(d, a, b, c, x[11], 9);
        HH(c, d, a, b, x[7], 11);
        HH(b, c, d, a, x[15], 15);

        a += aa;
        b += bb;
        c += cc;
        d += dd;
    }

    void digest(std::uint8_t *dest)
    {
        store_unaligned_le32(dest, a);
        store_unaligned_le32(dest + 4, b);
        store_unaligned_le32(dest + 8, c);
        store_unaligned_le32(dest + 12, d);
    }

    static std::uint32_t F(
        std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
    {
        return (x & y) | (~x & z);
    }

    static std::uint32_t G(
        std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
    {
        return (x & y) | (x & z) | (y & z);
    }

    static std::uint32_t H(
        std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
    {
        return x ^ y ^ z;
    }

    static void FF(std::uint32_t &a, std::uint32_t b, std::uint32_t c,
                   std::uint32_t d, std::uint32_t x, std::size_t s) noexcept
    {
        a = std::rotl(a + F(b, c, d) + x, s);
    }

    static void GG(std::uint32_t &a, std::uint32_t b, std::uint32_t c,
                   std::uint32_t d, std::uint32_t x, std::size_t s) noexcept
    {
        a = std::rotl(a + G(b, c, d) + x + 0x5A827999, s);
    }

    static void HH(std::uint32_t &a, std::uint32_t b, std::uint32_t c,
                   std::uint32_t d, std::uint32_t x, std::size_t s) noexcept
    {
        a = std::rotl(a + H(b, c, d) + x + 0x6ED9EBA1, s);
    }
};

} // namespace htl::detail

#endif
