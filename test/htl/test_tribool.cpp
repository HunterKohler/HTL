#include <algorithm>
#include <gtest/gtest.h>
#include <htl/tribool.h>
#include "./util/allocator.h"

namespace htl::test {

TEST(TriBoolTest, DefaultConstructor)
{
    ASSERT_EQ(TriBool(), nullptr);
    ASSERT_NE(TriBool(), false);
    ASSERT_NE(TriBool(), true);
}

TEST(TriBoolTest, NullConstructor)
{
    ASSERT_EQ(TriBool(nullptr), nullptr);
    ASSERT_NE(TriBool(nullptr), false);
    ASSERT_NE(TriBool(nullptr), true);
}

TEST(TriBoolTest, BoolConstructor)
{
    ASSERT_NE(TriBool(false), nullptr);
    ASSERT_EQ(TriBool(false), false);
    ASSERT_NE(TriBool(false), true);

    ASSERT_NE(TriBool(true), nullptr);
    ASSERT_NE(TriBool(true), false);
    ASSERT_EQ(TriBool(true), true);
}

TEST(TriBoolTest, NegateOperator)
{
    ASSERT_FALSE(!TriBool(nullptr));
    ASSERT_TRUE(!TriBool(false));
    ASSERT_FALSE(!TriBool(true));
}

TEST(TriBoolTest, OutputStream)
{
    ASSERT_EQ((std::stringstream() << TriBool(nullptr)).str(), "null");
    ASSERT_EQ((std::stringstream() << TriBool(false)).str(), "false");
    ASSERT_EQ((std::stringstream() << TriBool(true)).str(), "true");
}

TEST(TriBoolTest, Ordering)
{
    std::array<TriBool, 3> unordered{ true, false, nullptr },
        ordered{ nullptr, false, true };

    std::ranges::sort(unordered);
    ASSERT_EQ(unordered, ordered);
}

} // namespace htl::test
