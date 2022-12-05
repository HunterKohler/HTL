#include <array>
#include <gtest/gtest.h>
#include <htl/bit.h>

namespace htl::test {

template <class T>
void check_byteswap(T input, T expected)
{
    ASSERT_EQ(byteswap(input), expected);
}

TEST(BitTest, ByteSwap)
{
    check_byteswap<bool>(true, true);
    check_byteswap<bool>(false, false);
    check_byteswap<std::int8_t>(0x01, 0x01);
    check_byteswap<std::uint8_t>(0x01, 0x01);
    check_byteswap<std::int16_t>(0x0102, 0x0201);
    check_byteswap<std::uint16_t>(0x0102, 0x0201);
    check_byteswap<std::int32_t>(0x01020304, 0x04030201);
    check_byteswap<std::uint32_t>(0x01020304, 0x04030201);
    check_byteswap<std::int64_t>(0x0102030405060708, 0x0807060504030201);
    check_byteswap<std::uint64_t>(0x0102030405060708, 0x0807060504030201);
}

} // namespace htl::test
