#include <ranges>
#include <gtest/gtest.h>
#include <htl/ascii.h>

namespace htl::test {

constexpr std::ranges::iota_view ascii_view{ 0, 128 };

auto assert_same_under_ascii(auto f1, auto f2)
{
    for (char n: ascii_view) {
        ASSERT_EQ(f1(n), f2(n));
    }
}

TEST(AsciiTest, AsciiIsAscii)
{
    for (char n: ascii_view) {
        ASSERT_TRUE(ascii_isascii(n));
        ASSERT_FALSE(ascii_isascii(n + 128));
    }
}

TEST(AsciiTest, AsciiIsDigit)
{
    assert_same_under_ascii(ascii_isdigit, isdigit);
}

TEST(AsciiTest, AsciiIsLower)
{
    assert_same_under_ascii(ascii_islower, islower);
}

TEST(AsciiTest, AsciiIsUpper)
{
    assert_same_under_ascii(ascii_isupper, isupper);
}

TEST(AsciiTest, AsciiIsGraph)
{
    assert_same_under_ascii(ascii_isgraph, isgraph);
}

TEST(AsciiTest, AsciiIsPrint)
{
    assert_same_under_ascii(ascii_isprint, isprint);
}

TEST(AsciiTest, AsciiIsAlnum)
{
    assert_same_under_ascii(ascii_isalnum, isalnum);
}

TEST(AsciiTest, AsciiIsAlpha)
{
    assert_same_under_ascii(ascii_isalpha, isalpha);
}

TEST(AsciiTest, AsciiIsBlank)
{
    assert_same_under_ascii(ascii_isblank, isblank);
}

TEST(AsciiTest, AsciiIsCntrl)
{
    assert_same_under_ascii(ascii_iscntrl, iscntrl);
}

TEST(AsciiTest, AsciiIsPunct)
{
    assert_same_under_ascii(ascii_ispunct, ispunct);
}

TEST(AsciiTest, AsciiIsSpace)
{
    assert_same_under_ascii(ascii_isspace, isspace);
}

TEST(AsciiTest, AsciiIsXdigit)
{
    assert_same_under_ascii(ascii_isxdigit, isxdigit);
}

TEST(AsciiTest, AsciiToLower)
{
    assert_same_under_ascii(ascii_tolower, tolower);
}

TEST(AsciiTest, AsciiToUpper)
{
    assert_same_under_ascii(ascii_toupper, toupper);
}

} // namespace htl::test
