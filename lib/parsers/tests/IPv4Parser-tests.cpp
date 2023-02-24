#include <string>
#include <gtest/gtest.h>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"

namespace pimc::testing {

class IPv4ParsingTests : public ::testing::Test {
protected:
};

TEST_F(IPv4ParsingTests, IPv4Address_BasicParse) {
  auto pa = parseIPv4Address("000.000.000.000");
  ASSERT_TRUE(pa);
  EXPECT_EQ(*pa, net::IPv4Address{});

  net::IPv4Address bc{255,255,255,255};
  auto bcP = parseIPv4Address("0255.00255.000255.0000255");
  ASSERT_TRUE(bcP);
  EXPECT_EQ(*bcP, bc);
}

TEST_F(IPv4ParsingTests, IPv4Address_InvalidInput) {
    auto r1 = parseIPv4Address("1.1.1.0 ");
    EXPECT_FALSE(r1);

    auto r2 = parseIPv4Address("1.256.1.0");
    EXPECT_FALSE(r2);

    auto r3 = parseIPv4Address("1.255.1");
    EXPECT_FALSE(r3);
}

TEST_F(IPv4ParsingTests, IPv4Prefix_BasicParse) {
  auto ep = net::IPv4Prefix::make(net::IPv4Address{235,254,0,0}, 17);
  auto pp = parseIPv4Prefix("235.254.43.13/17");
  ASSERT_TRUE(pp);
  EXPECT_EQ(*pp, ep);
}

TEST_F(IPv4ParsingTests, IPv4Prefix_InvalidInput) {
    auto r1 = parseIPv4Prefix("1.2.3.4 /24");
    EXPECT_FALSE(r1);

    auto r2 = parseIPv4Prefix("1.2.5.4/33");
    EXPECT_FALSE(r2);
}

} // namespace pimc::testing

