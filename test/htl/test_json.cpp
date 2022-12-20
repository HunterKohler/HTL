#include <gtest/gtest.h>
#include <htl/json.h>
#include <htl/utility.h>
#include "./util/allocator.h"

namespace htl::test {

using namespace htl::json;

using TestAlloc = IdentityAllocator<std::byte>;
using TestDocument = BasicDocument<IdentityAllocator<std::byte>>;
using TestString = BasicString<IdentityAllocator<std::byte>>;
using TestArray = BasicArray<IdentityAllocator<std::byte>>;
using TestObject = BasicObject<IdentityAllocator<std::byte>>;

TEST(JSONTest, DocumentDefaultContructor)
{
    TestDocument value;
    ASSERT_TRUE(value.is_null());
    ASSERT_EQ(value.get_allocator(), TestAlloc());
}

TEST(JSONTest, DocumentAllocatorConstructor)
{
    TestAlloc alloc(1);
    TestDocument value(alloc);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentNullConstructor)
{
    TestAlloc alloc(1);
    TestDocument value(nullptr, alloc);
    ASSERT_TRUE(value.is_null());
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentBoolConstructor)
{
    TestAlloc alloc(1);
    TestDocument value1(true, alloc), value2(false, alloc);
    ASSERT_TRUE(value1.is_bool());
    ASSERT_TRUE(value2.is_bool());
    ASSERT_EQ(value1.get_bool(), true);
    ASSERT_EQ(value2.get_bool(), false);
    ASSERT_EQ(value1.get_allocator(), alloc);
    ASSERT_EQ(value2.get_allocator(), alloc);
}

TEST(JSONTest, DocumentIntegralConstructor)
{
    TestAlloc alloc(1);
    TestDocument value(123, alloc);
    ASSERT_TRUE(value.is_int());
    ASSERT_EQ(value.get_int(), 123);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentFloatingPointConstructor)
{
    TestAlloc alloc(1);
    TestDocument value(123.123, alloc);
    ASSERT_TRUE(value.is_float());
    ASSERT_EQ(value.get_float(), 123.123);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentCStringConstructor)
{
    TestAlloc alloc(1);
    const char str[] = "123";
    TestDocument value(str, alloc);
    ASSERT_TRUE(value.is_string());
    ASSERT_STREQ(value.get_string().data(), str);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentStringViewConstructor)
{
    TestAlloc alloc(1);
    std::string_view str("123");
    TestDocument value(str, alloc);
    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), str);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentStringCopyConstructor)
{
    std::string_view sv("123");
    TestAlloc alloc(1);
    TestString str(sv, alloc);
    TestDocument value(str);

    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), str);
    ASSERT_EQ(str, sv);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentArrayCopyConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentObjectCopyConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentStringAllocatorExtendedCopyConstructor)
{
    std::string_view sv("123");
    TestAlloc alloc1(1), alloc2(2);
    TestString str(sv, alloc1);
    TestDocument value(str, alloc2);

    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), str);
    ASSERT_EQ(str, sv);
    ASSERT_EQ(value.get_allocator(), alloc2);
}

TEST(JSONTest, DocumentArrayAllocatorExtendedCopyConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentObjectAllocatorExtendedCopyConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentStringMoveConstructor)
{
    std::string_view sv("123");
    TestAlloc alloc(1);
    TestString str(sv, alloc);
    TestDocument value(std::move(str));

    ASSERT_TRUE(str.empty());
    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), sv);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentArrayMoveConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentObjectMoveConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentStringAllocatorExtendedMoveConstructor)
{
    std::string_view sv("123");
    TestAlloc alloc1(1), alloc2(2);
    TestString str(sv, alloc1);
    TestDocument value(std::move(str), alloc2);

    ASSERT_TRUE(str.empty());
    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), sv);
    ASSERT_EQ(value.get_allocator(), alloc2);
}

TEST(JSONTest, DocumentArrayAllocatorExtendedMoveConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentObjectAllocatorExtendedMoveConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentCopyConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentMoveConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentAllocatorExtendedCopyConstructor)
{
    GTEST_SKIP();
}

TEST(JSONTest, DocumentAllocatorExtendedMoveConstructor)
{
    GTEST_SKIP();
}

} // namespace htl::test
