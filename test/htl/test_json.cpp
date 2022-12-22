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
    char cstring[] = "123";
    TestAlloc alloc(1);
    TestDocument value(cstring, alloc);
    ASSERT_TRUE(value.is_string());
    ASSERT_STREQ(value.get_string().data(), cstring);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentStringViewConstructor)
{
    std::string_view string_view = "123";
    TestAlloc alloc(1);
    TestDocument value(string_view, alloc);
    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), string_view);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentStringCopyConstructor)
{
    TestAlloc alloc(1);
    TestString string("123", alloc);
    TestString string_copy(string);
    TestDocument value(string);

    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), string_copy);
    ASSERT_EQ(value.get_allocator(), alloc);
    ASSERT_EQ(string, string_copy);
}

TEST(JSONTest, DocumentArrayCopyConstructor)
{
    TestAlloc alloc(1);
    TestArray array({ nullptr, true, 123, 123.123, "123" }, alloc);
    TestArray array_copy(array);
    TestDocument value(array);

    ASSERT_TRUE(value.is_array());
    ASSERT_EQ(value.get_array(), array_copy);
    ASSERT_EQ(value.get_allocator(), alloc);
    ASSERT_EQ(array, array_copy);
}

TEST(JSONTest, DocumentObjectCopyConstructor)
{
    TestAlloc alloc(1);
    TestObject object(
        { { "key1", nullptr },
          { "key2", true },
          { "key3", 123 },
          { "key4", 123.123 },
          { "key5", "123" } },
        alloc);
    TestObject object_copy(object);
    TestDocument value(object);

    ASSERT_TRUE(value.is_object());
    ASSERT_EQ(value.get_object(), object_copy);
    ASSERT_EQ(value.get_allocator(), alloc);
    ASSERT_EQ(object, object_copy);
}

TEST(JSONTest, DocumentStringAllocatorExtendedCopyConstructor)
{
    TestAlloc alloc1(1), alloc2(2);
    TestString string("123", alloc1);
    TestString string_copy(string);
    TestDocument value(string, alloc2);

    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), string_copy);
    ASSERT_EQ(value.get_allocator(), alloc2);
    ASSERT_EQ(string, string_copy);
}

TEST(JSONTest, DocumentArrayAllocatorExtendedCopyConstructor)
{
    TestAlloc alloc1(1), alloc2(2);
    TestArray array({ nullptr, true, 123, 123.123, "123" }, alloc1);
    TestArray array_copy(array);
    TestDocument value(array, alloc2);

    ASSERT_TRUE(value.is_array());
    ASSERT_EQ(value.get_array(), array_copy);
    ASSERT_EQ(value.get_allocator(), alloc2);
    ASSERT_EQ(array, array_copy);
}

TEST(JSONTest, DocumentObjectAllocatorExtendedCopyConstructor)
{
    TestAlloc alloc1(1), alloc2(2);
    TestObject object(
        { { "key1", nullptr },
          { "key2", true },
          { "key3", 123 },
          { "key4", 123.123 },
          { "key5", "123" } },
        alloc1);
    TestObject object_copy(object);
    TestDocument value(object, alloc2);

    ASSERT_TRUE(value.is_object());
    ASSERT_EQ(value.get_object(), object_copy);
    ASSERT_EQ(value.get_allocator(), alloc2);
    ASSERT_EQ(object, object_copy);
}

TEST(JSONTest, DocumentStringMoveConstructor)
{
    TestAlloc alloc(1);
    TestString string("123", alloc);
    TestString string_copy("123", alloc);
    TestDocument value(std::move(string));

    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), string_copy);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentArrayMoveConstructor)
{
    TestAlloc alloc(1);
    TestArray array({ nullptr, true, 123, 123.123, "123" }, alloc);
    TestArray array_copy(array);
    TestDocument value(std::move(array));

    ASSERT_TRUE(value.is_array());
    ASSERT_EQ(value.get_array(), array_copy);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentObjectMoveConstructor)
{
    TestAlloc alloc(1);
    TestObject object(
        { { "key1", nullptr },
          { "key2", true },
          { "key3", 123 },
          { "key4", 123.123 },
          { "key5", "123" } },
        alloc);
    TestObject object_copy(object);
    TestDocument value(std::move(object));

    ASSERT_TRUE(value.is_object());
    ASSERT_EQ(value.get_object(), object_copy);
    ASSERT_EQ(value.get_allocator(), alloc);
}

TEST(JSONTest, DocumentStringAllocatorExtendedMoveConstructor)
{
    TestAlloc alloc1(1), alloc2(2);
    TestString string("123", alloc1);
    TestString string_copy(string);
    TestDocument value(std::move(string), alloc2);

    ASSERT_TRUE(value.is_string());
    ASSERT_EQ(value.get_string(), string_copy);
    ASSERT_EQ(value.get_allocator(), alloc2);
}

TEST(JSONTest, DocumentArrayAllocatorExtendedMoveConstructor)
{
    TestAlloc alloc1(1), alloc2(2);
    TestArray array({ nullptr, true, 123, 123.123, "123" }, alloc1);
    TestArray array_copy(array);
    TestDocument value(std::move(array), alloc2);

    ASSERT_TRUE(value.is_array());
    ASSERT_EQ(value.get_array(), array_copy);
    ASSERT_EQ(value.get_allocator(), alloc2);
}

TEST(JSONTest, DocumentObjectAllocatorExtendedMoveConstructor)
{
    TestAlloc alloc1(1), alloc2(2);
    TestObject object(
        { { "key1", nullptr },
          { "key2", true },
          { "key3", 123 },
          { "key4", 123.123 },
          { "key5", "123" } },
        alloc1);
    TestObject object_copy(object);
    TestDocument value(std::move(object), alloc2);

    ASSERT_TRUE(value.is_object());
    ASSERT_EQ(value.get_object(), object_copy);
    ASSERT_EQ(value.get_allocator(), alloc2);
}

// TEST(JSONTest, DocumentCopyConstructor)
// {
// }

// TEST(JSONTest, DocumentMoveConstructor)
// {
// }

// TEST(JSONTest, DocumentAllocatorExtendedCopyConstructor)
// {
// }

// TEST(JSONTest, DocumentAllocatorExtendedMoveConstructor)
// {
// }

} // namespace htl::test
