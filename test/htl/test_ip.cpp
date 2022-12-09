#include <cstddef>
#include <set>
#include <sstream>
#include <type_traits>
#include <vector>
#include <gtest/gtest.h>
#include <htl/ip.h>
#include "./util/allocator.h"

namespace htl::test {

static_assert(std::is_trivially_constructible_v<IPv4Address::bytes_type>);
static_assert(std::is_trivially_constructible_v<IPv6Address::bytes_type>);

TEST(IPTest, IPv4AddressDefaultConstruct)
{
    alignas(IPv4Address) std::array<std::byte, sizeof(IPv4Address)> full;
    alignas(IPv4Address) std::array<std::byte, sizeof(IPv4Address)> value;

    full.fill(~std::byte());
    value.fill(~std::byte());

    new (value.data()) IPv4Address;

    ASSERT_EQ(full, value);
}

TEST(IPTest, IPv4AddressValueConstruct)
{
    alignas(IPv4Address) std::array<std::byte, sizeof(IPv4Address)> zero;
    alignas(IPv4Address) std::array<std::byte, sizeof(IPv4Address)> value;

    zero.fill(std::byte());
    value.fill(~std::byte());

    new (value.data()) IPv4Address();

    ASSERT_EQ(zero, value);
}

TEST(IPTest, IPv4AddressIntConstruct)
{
    ASSERT_EQ(IPv4Address(0x12345678).to_bytes(),
              (IPv4Address::bytes_type{ 0x12, 0x34, 0x56, 0x78 }));
}

TEST(IPTest, IPv4AddressBytesConstruct)
{
    ASSERT_EQ((IPv4Address({ 0x12, 0x34, 0x56, 0x78 }).to_bytes()),
              (IPv4Address::bytes_type{ 0x12, 0x34, 0x56, 0x78 }));
}

TEST(IPTest, IPv4AddressToUInt)
{
    ASSERT_EQ(IPv4Address({ 0x12, 0x34, 0x56, 0x78 }).to_uint(), 0x12345678);
}

TEST(IPTest, IPv4AddressIsUnspecified)
{
    ASSERT_TRUE(IPv4Address::any().is_unspecified());
    ASSERT_FALSE(IPv4Address::loopback().is_unspecified());
    ASSERT_FALSE(IPv4Address::broadcast().is_unspecified());
}

TEST(IPTest, IPv4AddressIsLoopback)
{
    ASSERT_FALSE(IPv4Address::any().is_loopback());
    ASSERT_TRUE(IPv4Address::loopback().is_loopback());
    ASSERT_FALSE(IPv4Address::broadcast().is_loopback());
}

TEST(IPTest, IPv4AddressIsClassA)
{
    ASSERT_TRUE((IPv4Address({ 0x00, 0x01, 0x02, 0x03 }).is_class_a()));
    ASSERT_FALSE((IPv4Address({ 0x01, 0x01, 0x02, 0x03 }).is_class_a()));
}

TEST(IPTest, IPv4AddressIsClassB)
{
    ASSERT_TRUE((IPv4Address({ 0x80, 0x00, 0x01, 0x02 }).is_class_b()));
    ASSERT_FALSE((IPv4Address({ 0x80, 0x01, 0x01, 0x02 }).is_class_b()));
    ASSERT_FALSE((IPv4Address({ 0xFF, 0x00, 0x01, 0x02 }).is_class_b()));
}

TEST(IPTest, IPv4AddressIsClassC)
{
    ASSERT_TRUE((IPv4Address({ 0xC0, 0x00, 0x00, 0x01 }).is_class_c()));
    ASSERT_FALSE((IPv4Address({ 0xFF, 0x00, 0x00, 0x01 }).is_class_c()));
    ASSERT_FALSE((IPv4Address({ 0xC0, 0x01, 0x00, 0x01 }).is_class_c()));
    ASSERT_FALSE((IPv4Address({ 0xC0, 0x00, 0x01, 0x01 }).is_class_c()));
}

} // namespace htl::test
