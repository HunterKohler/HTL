#include <ranges>
#include <sstream>
#include <gtest/gtest.h>
#include <htl/rational.h>
#include "./util/allocator.h"

namespace htl::test {

TEST(RationalTest, DefaultConstructor)
{
    alignas(Rational<int>) char buf[sizeof(Rational<int>)];

    std::ranges::fill(buf, ~0);

    auto a = new (buf) Rational<int>();

    ASSERT_EQ(a->numer(), 0);
    ASSERT_EQ(a->denom(), 1);
}

TEST(RationalTest, NumeratorConstructor)
{
    Rational<int> a(10);

    ASSERT_EQ(a.numer(), 10);
    ASSERT_EQ(a.denom(), 1);
}

TEST(RationalTest, NumerDenomConstructor)
{
    Rational<int> a(10, 20);

    ASSERT_EQ(a.numer(), 10);
    ASSERT_EQ(a.denom(), 20);
}

TEST(RationalTest, ConvertingConstructor)
{
    Rational<long long> a(10, 20);
    Rational<short> b(a);

    ASSERT_EQ(b, Rational(10, 20));
}

TEST(RationalTest, IntAssignment)
{
    Rational<int> a(10, 20);

    a = 25;

    ASSERT_EQ(a, Rational(25, 1));
}

TEST(RationalTest, ConvertingAssignment)
{
    Rational<long long> a(10, 20);
    Rational<short> b;

    b = a;

    ASSERT_EQ(b, Rational(10, 20));
}

TEST(RationalTest, Assign)
{
    Rational<int> a(10, 20);

    a.assign(5, 10);

    ASSERT_EQ(a, Rational(5, 10));
}

TEST(RationalTest, Invert)
{
    Rational<int> a(10, 20);

    ASSERT_EQ(a.invert(), Rational(20, 10));
}

TEST(RationalTest, Normalize)
{
    Rational<int> a(-10, -20);

    ASSERT_EQ(a.normalize(), Rational(1, 2));
}

TEST(RationalTest, Abs)
{
    Rational<int> a(-1251, 12);

    ASSERT_EQ(a.abs(), Rational(1251, 12));
}

TEST(RationalTest, SetNumer)
{
    Rational<int> a(10, 20);

    a.numer(50);

    ASSERT_EQ(a, Rational(50, 20));
}

TEST(RationalTest, SetDenom)
{
    Rational<int> a(10, 20);

    a.denom(50);

    ASSERT_EQ(a, Rational(10, 50));
}

TEST(RationalTest, BoolConversion)
{
    Rational<int> a(1, 2);
    Rational<int> b(0, 2);

    ASSERT_TRUE(a);
    ASSERT_FALSE(b);
}

TEST(RationalTest, PreIncrement)
{
    Rational<int> a(1, 2);

    ASSERT_EQ(++a, Rational(3, 2));
    ASSERT_EQ(a, Rational(3, 2));
}

TEST(RationalTest, PostIncrement)
{
    Rational<int> a(1, 2);

    ASSERT_EQ(a++, Rational(1, 2));
    ASSERT_EQ(a, Rational(3, 2));
}

TEST(RationalTest, PreDecrement)
{
    Rational<int> a(1, 2);

    ASSERT_EQ(--a, Rational(-1, 2));
    ASSERT_EQ(a, Rational(-1, 2));
}

TEST(RationalTest, PostDecrement)
{
    Rational<int> a(1, 2);

    ASSERT_EQ(a--, Rational(1, 2));
    ASSERT_EQ(a, Rational(-1, 2));
}

TEST(RationalTest, AddAssignRational)
{
    Rational<int> a(1, 2);

    a += Rational(5, 6);

    ASSERT_EQ(a, Rational(16, 12));
}

TEST(RationalTest, SubAssignRational)
{
    Rational<int> a(1, 2);

    a -= Rational(5, 6);

    ASSERT_EQ(a, Rational(-4, 12));
}

TEST(RationalTest, MultAssignRational)
{
    Rational<int> a(1, 2);

    a *= Rational(5, 6);

    ASSERT_EQ(a, Rational(5, 12));
}

TEST(RationalTest, DivAssignRational)
{
    Rational<int> a(1, 2);

    a /= Rational(5, 6);

    ASSERT_EQ(a, Rational(6, 10));
}

TEST(RationalTest, AddAssignInt)
{
    Rational<int> a(1, 2);

    a += 10;

    ASSERT_EQ(a, Rational(21, 2));
}

TEST(RationalTest, SubAssignInt)
{
    Rational<int> a(1, 2);

    a -= 10;

    ASSERT_EQ(a, Rational(-19, 2));
}

TEST(RationalTest, MultAssignInt)
{
    Rational<int> a(1, 2);

    a *= 10;

    ASSERT_EQ(a, Rational(10, 2));
}

TEST(RationalTest, DivAssignInt)
{
    Rational<int> a(1, 2);

    a /= 10;

    ASSERT_EQ(a, Rational(1, 20));
}

TEST(RationalTest, UnaryNegate)
{
    Rational<int> a(1, 2);

    ASSERT_EQ(-a, Rational(-1, 2));
}

TEST(RationalTest, RationalAdd)
{
    ASSERT_EQ(Rational(1, 2) + Rational(65, 12), Rational(71, 12));
}

TEST(RationalTest, RationalSub)
{
    ASSERT_EQ(Rational(1, 2) - Rational(65, 12), Rational(-59, 12));
}

TEST(RationalTest, RationalMult)
{
    ASSERT_EQ(Rational(1, 2) * Rational(65, 12), Rational(65, 24));
}

TEST(RationalTest, RationalDiv)
{
    ASSERT_EQ(Rational(1, 2) / Rational(65, 12), Rational(6, 65));
}

TEST(RationalTest, IntAdd)
{
    ASSERT_EQ(Rational(1, 2) + 5, Rational(11, 2));
    ASSERT_EQ(5 + Rational(1, 2), Rational(11, 2));
}

TEST(RationalTest, IntSub)
{
    ASSERT_EQ(Rational(1, 2) - 5, Rational(-9, 2));
    ASSERT_EQ(5 - Rational(1, 2), Rational(9, 2));
}

TEST(RationalTest, IntMult)
{
    ASSERT_EQ(Rational(1, 2) * 5, Rational(5, 2));
    ASSERT_EQ(5 * Rational(1, 2), Rational(5, 2));
}

TEST(RationalTest, IntDiv)
{
    ASSERT_EQ(Rational(1, 2) / 5, Rational(1, 10));
    ASSERT_EQ(5 / Rational(1, 2), Rational(10, 1));
}

TEST(RationalTest, IntEqual)
{
    ASSERT_NE(Rational(1, 2), 1);
    ASSERT_EQ(Rational(1, 1), 1);
    ASSERT_EQ(Rational(5, 1), 5);
    ASSERT_EQ(Rational(64, 8), 8);
}

TEST(RationalTest, RationalEqual)
{
    ASSERT_NE(Rational(1, 2), Rational(1, 3));
    ASSERT_EQ(Rational(2, 4), Rational(1, 2));
    ASSERT_EQ(Rational(-12, -24), Rational(1, 2));
}

TEST(RationalTest, IntThreeWayCompare)
{
    ASSERT_LT(Rational(1, 2), 1);
    ASSERT_LT(Rational(-1, -2), 1);
    ASSERT_GT(Rational(3, 2), 1);
    ASSERT_GT(Rational(-3, -2), 1);
}

TEST(RationalTest, RationalThreeWayCompare)
{
    ASSERT_GT(Rational(1, 2), Rational(1, 3));
    ASSERT_LT(Rational(-1, 2), Rational(1, 2));
    ASSERT_LT(Rational(1, -2), Rational(1, 2));
}

TEST(RationalTest, UnaryPlus)
{
    Rational<int> a(1, 2);

    ASSERT_EQ(+a, a);
}

TEST(RationalTest, ToChars)
{
    std::string dest;
    to_chars(Rational(-12, 13), std::back_inserter(dest));
    ASSERT_EQ(dest, "-12/13");
}

TEST(RationalTest, ToString)
{
    IdentityAllocator<char> alloc(52);
    Rational<int> value(-12, 13);
    auto res = to_string(value, alloc);

    ASSERT_EQ(res.get_allocator(), alloc);
    ASSERT_EQ(res, "-12/13");
}

TEST(RationalTest, IStreamOperator)
{
    Rational<int> a, b, c;
    std::stringstream stream_a("123/-456x");
    std::stringstream stream_b("123x");
    std::stringstream stream_c("123/x");

    stream_a >> a;
    stream_b >> b;
    stream_c >> c;

    ASSERT_TRUE(stream_a);
    ASSERT_TRUE(stream_b);
    ASSERT_FALSE(stream_c);

    ASSERT_EQ(a, Rational(123, -456));
    ASSERT_EQ(b, 123);
    ASSERT_EQ(c, Rational(0, 1));
}

TEST(RationalTest, OStreamOperator)
{
    Rational<int> a(123, 456);
    std::stringstream stream;

    stream << a;

    ASSERT_EQ(stream.view(), "123/456");
}

template <class T>
std::size_t hash_rational(const Rational<T> &value)
{
    return std::hash<Rational<T>>()(value);
}

TEST(RationalTest, Hash)
{
    ASSERT_EQ(hash_rational(Rational(1, 2)), hash_rational(Rational(1, 2)));
    ASSERT_EQ(hash_rational(Rational(1, 2)), hash_rational(Rational(2, 4)));
    ASSERT_EQ(hash_rational(Rational(1, 2)), hash_rational(Rational(-1, -2)));
    ASSERT_NE(hash_rational(Rational(1, 2)), hash_rational(Rational(-1, 2)));
    ASSERT_NE(hash_rational(Rational(1, 2)), hash_rational(Rational(1, 3)));
}

} // namespace htl::test
