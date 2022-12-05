#ifndef HTL_DETAIL_MD5_STATE_H_
#define HTL_DETAIL_MD5_STATE_H_

#include <bit>
#include <cstdint>
#include <htl/unaligned.h>

namespace htl::detail {

struct MD5State {
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
        step<F>(a, b, c, d, x[0], 7, 0xD76AA478);
        step<F>(d, a, b, c, x[1], 12, 0xE8C7B756);
        step<F>(c, d, a, b, x[2], 17, 0x242070DB);
        step<F>(b, c, d, a, x[3], 22, 0xC1BDCEEE);
        step<F>(a, b, c, d, x[4], 7, 0xF57C0FAF);
        step<F>(d, a, b, c, x[5], 12, 0x4787C62A);
        step<F>(c, d, a, b, x[6], 17, 0xA8304613);
        step<F>(b, c, d, a, x[7], 22, 0xFD469501);
        step<F>(a, b, c, d, x[8], 7, 0x698098D8);
        step<F>(d, a, b, c, x[9], 12, 0x8B44F7AF);
        step<F>(c, d, a, b, x[10], 17, 0xFFFF5BB1);
        step<F>(b, c, d, a, x[11], 22, 0x895CD7BE);
        step<F>(a, b, c, d, x[12], 7, 0x6B901122);
        step<F>(d, a, b, c, x[13], 12, 0xFD987193);
        step<F>(c, d, a, b, x[14], 17, 0xA679438E);
        step<F>(b, c, d, a, x[15], 22, 0x49B40821);

        // Round 2
        step<G>(a, b, c, d, x[1], 5, 0xF61E2562);
        step<G>(d, a, b, c, x[6], 9, 0xC040B340);
        step<G>(c, d, a, b, x[11], 14, 0x265E5A51);
        step<G>(b, c, d, a, x[0], 20, 0xE9B6C7AA);
        step<G>(a, b, c, d, x[5], 5, 0xD62F105D);
        step<G>(d, a, b, c, x[10], 9, 0x02441453);
        step<G>(c, d, a, b, x[15], 14, 0xD8A1E681);
        step<G>(b, c, d, a, x[4], 20, 0xE7D3FBC8);
        step<G>(a, b, c, d, x[9], 5, 0x21E1CDE6);
        step<G>(d, a, b, c, x[14], 9, 0xC33707D6);
        step<G>(c, d, a, b, x[3], 14, 0xF4D50D87);
        step<G>(b, c, d, a, x[8], 20, 0x455A14ED);
        step<G>(a, b, c, d, x[13], 5, 0xA9E3E905);
        step<G>(d, a, b, c, x[2], 9, 0xFCEFA3F8);
        step<G>(c, d, a, b, x[7], 14, 0x676F02D9);
        step<G>(b, c, d, a, x[12], 20, 0x8D2A4C8A);

        // Round 3
        step<H>(a, b, c, d, x[5], 4, 0xFFFA3942);
        step<H>(d, a, b, c, x[8], 11, 0x8771F681);
        step<H>(c, d, a, b, x[11], 16, 0x6D9D6122);
        step<H>(b, c, d, a, x[14], 23, 0xFDE5380C);
        step<H>(a, b, c, d, x[1], 4, 0xA4BEEA44);
        step<H>(d, a, b, c, x[4], 11, 0x4BDECFA9);
        step<H>(c, d, a, b, x[7], 16, 0xF6BB4B60);
        step<H>(b, c, d, a, x[10], 23, 0xBEBFBC70);
        step<H>(a, b, c, d, x[13], 4, 0x289B7EC6);
        step<H>(d, a, b, c, x[0], 11, 0xEAA127FA);
        step<H>(c, d, a, b, x[3], 16, 0xD4EF3085);
        step<H>(b, c, d, a, x[6], 23, 0x04881D05);
        step<H>(a, b, c, d, x[9], 4, 0xD9D4D039);
        step<H>(d, a, b, c, x[12], 11, 0xE6DB99E5);
        step<H>(c, d, a, b, x[15], 16, 0x1FA27CF8);
        step<H>(b, c, d, a, x[2], 23, 0xC4AC5665);

        // Round 4
        step<I>(a, b, c, d, x[0], 6, 0xF4292244);
        step<I>(d, a, b, c, x[7], 10, 0x432AFF97);
        step<I>(c, d, a, b, x[14], 15, 0xAB9423A7);
        step<I>(b, c, d, a, x[5], 21, 0xFC93A039);
        step<I>(a, b, c, d, x[12], 6, 0x655B59C3);
        step<I>(d, a, b, c, x[3], 10, 0x8F0CCC92);
        step<I>(c, d, a, b, x[10], 15, 0xFFEFF47D);
        step<I>(b, c, d, a, x[1], 21, 0x85845DD1);
        step<I>(a, b, c, d, x[8], 6, 0x6FA87E4F);
        step<I>(d, a, b, c, x[15], 10, 0xFE2CE6E0);
        step<I>(c, d, a, b, x[6], 15, 0xA3014314);
        step<I>(b, c, d, a, x[13], 21, 0x4E0811A1);
        step<I>(a, b, c, d, x[4], 6, 0xF7537E82);
        step<I>(d, a, b, c, x[11], 10, 0xBD3AF235);
        step<I>(c, d, a, b, x[2], 15, 0x2AD7D2BB);
        step<I>(b, c, d, a, x[9], 21, 0xEB86D391);

        a += aa;
        b += bb;
        c += cc;
        d += dd;
    }

    void digest(std::uint8_t *dest) noexcept
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
        return (x & z) | (y & ~z);
    }

    static std::uint32_t H(
        std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
    {
        return x ^ y ^ z;
    }

    static std::uint32_t I(
        std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
    {
        return y ^ (x | ~z);
    }

    template <auto Fn>
    static void
    step(std::uint32_t &a, std::uint32_t b, std::uint32_t c, std::uint32_t d,
         std::uint32_t x, std::size_t s, std::uint32_t t) noexcept
    {
        a = b + std::rotl(a + Fn(b, c, d) + x + t, s);
    }
};

} // namespace htl::detail

#endif
