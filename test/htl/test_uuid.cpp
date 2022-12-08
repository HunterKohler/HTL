#include <cstddef>
#include <set>
#include <sstream>
#include <vector>
#include <gtest/gtest.h>
#include <htl/uuid.h>
#include "./util/allocator.h"

namespace htl::test {

struct UUIDTestData {
    std::string_view string;
    UUID::bytes_type bytes;
    UUIDVersion version;
    UUIDVariant variant;
};

const std::vector<UUIDTestData> uuid_test_data{
    {
        "00000000-0000-0000-0000-000000000000",
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        UUIDVersion::Unknown,
        UUIDVariant::NCS,
    },
    {
        "123e4567-e89b-12d3-0456-426614174000",
        { 0x12, 0x3E, 0x45, 0x67, 0xE8, 0x9B, 0x12, 0xD3, 0x04, 0x56, 0x42,
          0x66, 0x14, 0x17, 0x40, 0x00 },
        UUIDVersion::v1,
        UUIDVariant::NCS,
    },
    {
        "123e4567-e89b-22d3-8456-426614174000",
        { 0x12, 0x3E, 0x45, 0x67, 0xE8, 0x9B, 0x22, 0xD3, 0x84, 0x56, 0x42,
          0x66, 0x14, 0x17, 0x40, 0x00 },
        UUIDVersion::v2,
        UUIDVariant::RFC,
    },
    {
        "123e4567-e89b-32d3-c456-426614174000",
        { 0x12, 0x3E, 0x45, 0x67, 0xE8, 0x9B, 0x32, 0xD3, 0xC4, 0x56, 0x42,
          0x66, 0x14, 0x17, 0x40, 0x00 },
        UUIDVersion::v3,
        UUIDVariant::Microsoft,
    },
    {
        "123e4567-e89b-42d3-e456-426614174000",
        { 0x12, 0x3E, 0x45, 0x67, 0xE8, 0x9B, 0x42, 0xD3, 0xE4, 0x56, 0x42,
          0x66, 0x14, 0x17, 0x40, 0x00 },
        UUIDVersion::v4,
        UUIDVariant::Future,
    },
    {
        "123e4567-e89b-52d3-f456-426614174000",
        { 0x12, 0x3E, 0x45, 0x67, 0xE8, 0x9B, 0x52, 0xD3, 0xF4, 0x56, 0x42,
          0x66, 0x14, 0x17, 0x40, 0x00 },
        UUIDVersion::v5,
        UUIDVariant::Unknown,
    },
    {
        "123e4567-e89b-02d3-f456-426614174000",
        { 0x12, 0x3E, 0x45, 0x67, 0xE8, 0x9B, 0x02, 0xD3, 0xF4, 0x56, 0x42,
          0x66, 0x14, 0x17, 0x40, 0x00 },
        UUIDVersion::Unknown,
        UUIDVariant::Unknown,
    }
};

const std::vector<std::string_view> uuid_invalid_strings{
    "",
    "x",
    "xxxxxxxx",
    "xxxxxxxx-xxxx",
    "xxxxxxxx-xxxx-xxxx",
    "xxxxxxxx-xxxx-xxxx-xxxx",
    "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
    "123e4567",
    "123e4567-e89b",
    "123e4567-e89b-52d3",
    "123e4567-e89b-52d3-f456",
    "123e4567xe89b-02d3-f456-426614174000",
    "123e4567-e89bx02d3-f456-426614174000",
    "123e4567-e89b-02d3xf456-426614174000",
    "123e4567-e89b-02d3-f456x426614174000",
    "{123e4567-e89b-02d3-f456-426614174000",
};

TEST(UUIDTest, DefaultConstruct)
{
    UUID::bytes_type full;
    UUID::bytes_type buf;

    full.fill(~std::byte());
    buf.fill(~std::byte());

    new (std::addressof(buf)) UUID;

    ASSERT_EQ(buf, full);
}

TEST(UUIDTest, ValueConstruct)
{
    UUID::bytes_type zero;
    UUID::bytes_type buf;

    zero.fill(std::byte());
    buf.fill(~std::byte());

    new (std::addressof(buf)) UUID();

    ASSERT_EQ(buf, zero);
}

TEST(UUIDTest, NullConstruct)
{
    ASSERT_EQ(UUID(), UUID(nullptr));
}

TEST(UUIDTest, IsNil)
{
    ASSERT_TRUE(UUID().is_nil());
    ASSERT_TRUE(UUID(nullptr).is_nil());
    ASSERT_FALSE(UUID(UUID::bytes_type(0xFF)).is_nil());
}

TEST(UUIDTest, BoolOperator)
{
    ASSERT_FALSE(UUID());
    ASSERT_FALSE(UUID(nullptr));
    ASSERT_TRUE(UUID(UUID::bytes_type(0xFF)));
}

TEST(UUIDTest, Version)
{
    for (auto &data: uuid_test_data) {
        ASSERT_EQ(UUID(data.bytes).version(), data.version);
    }
}

TEST(UUIDTest, Variant)
{
    for (auto &data: uuid_test_data) {
        ASSERT_EQ(UUID(data.bytes).variant(), data.variant);
    }
}

TEST(UUIDTest, MakeUUIDFromString)
{
    for (auto &data: uuid_test_data) {
        std::error_code err;
        auto value = make_uuid(data.string, err);

        ASSERT_FALSE(err);
        ASSERT_EQ(value.to_bytes(), data.bytes);
    }

    for (auto &data: uuid_test_data) {
        auto value = make_uuid(data.string);

        ASSERT_EQ(value.to_bytes(), data.bytes);
    }

    for (auto &data: uuid_test_data) {
        std::error_code err;
        auto string = "{" + std::string(data.string) + "}";
        auto value = make_uuid(string, err);

        ASSERT_FALSE(err);
        ASSERT_EQ(value.to_bytes(), data.bytes);
    }

    for (auto &data: uuid_test_data) {
        auto string = "{" + std::string(data.string) + "}";
        auto value = make_uuid(string);

        ASSERT_EQ(value.to_bytes(), data.bytes);
    }

    for (auto &string: uuid_invalid_strings) {
        std::error_code err;
        make_uuid(string, err);
        ASSERT_EQ(err, std::make_error_code(std::errc::invalid_argument));
    }

    for (auto &string: uuid_invalid_strings) {
        ASSERT_THROW(make_uuid(string), std::invalid_argument);
    }
}

TEST(UUIDTest, MakeUUIDWithGenerator)
{
    std::set<UUID> ids;
    std::mt19937 engine;
    int count = 100;

    for (int i = 0; i < count; i++) {
        ids.insert(make_uuid(engine));
    }

    ASSERT_EQ(ids.size(), count);
    ASSERT_EQ(make_uuid(engine).version(), UUIDVersion::v4);
    ASSERT_EQ(make_uuid(engine).variant(), UUIDVariant::RFC);
    ASSERT_EQ(make_uuid(engine, UUIDVariant::NCS).variant(), UUIDVariant::NCS);
    ASSERT_EQ(make_uuid(engine, UUIDVariant::RFC).variant(), UUIDVariant::RFC);
    ASSERT_EQ(make_uuid(engine, UUIDVariant::Microsoft).variant(),
              UUIDVariant::Microsoft);
    ASSERT_EQ(
        make_uuid(engine, UUIDVariant::Future).variant(), UUIDVariant::Future);
    ASSERT_EQ(make_uuid(engine, UUIDVariant::Unknown).variant(),
              UUIDVariant::Unknown);
}

TEST(UUIDTest, ToChars)
{
    for (auto &data: uuid_test_data) {
        UUID value(data.bytes);
        char buf[36];

        to_chars(value, buf);
        ASSERT_EQ(std::string_view(buf, 36), data.string);
    }
}

TEST(UUIDTest, ToString)
{
    IdentityAllocator<char> alloc(52);

    for (auto &data: uuid_test_data) {
        UUID value(data.bytes);
        auto string = to_string(value, alloc);

        ASSERT_EQ(string, data.string);
        ASSERT_EQ(string.get_allocator(), alloc);
    }
}

TEST(UUIDTest, OStreamOperator)
{
    for (auto &data: uuid_test_data) {
        UUID value(data.bytes);
        std::stringstream stream;

        stream << value;

        ASSERT_EQ(stream.view(), data.string);
    }
}

TEST(UUIDTest, IStreamOperator)
{
    for (auto &data: uuid_test_data) {
        UUID value;
        std::stringstream stream;

        stream << data.string << 'x';
        stream >> value;

        ASSERT_TRUE(stream);
        ASSERT_EQ(value.to_bytes(), data.bytes);
        ASSERT_EQ(static_cast<char>(stream.peek()), 'x');
    }

    for (auto &data: uuid_test_data) {
        UUID value;
        std::stringstream stream;

        stream << ("{" + std::string(data.string) + "}") << 'x';
        stream >> value;

        ASSERT_TRUE(stream);
        ASSERT_EQ(value.to_bytes(), data.bytes);
        ASSERT_EQ(static_cast<char>(stream.peek()), 'x');
    }

    for (auto &string: uuid_invalid_strings) {
        UUID value;
        std::stringstream stream;

        stream << string;
        stream >> value;

        ASSERT_FALSE(stream);
    }
}

TEST(UUIDTest, Hash)
{
    std::set<std::size_t> hashes;
    std::hash<UUID> hash;

    for (auto &data: uuid_test_data) {
        ASSERT_EQ(hash(data.bytes), hash(data.bytes));
        hashes.insert(hash(data.bytes));
    }

    ASSERT_EQ(hashes.size(), uuid_test_data.size());
}

} // namespace htl::test
