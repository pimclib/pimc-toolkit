#include <string>
#include <gtest/gtest.h>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"

namespace pimc::testing {

class IPv4AddressTests : public ::testing::Test {
protected:
};

class IPv4PrefixTests : public ::testing::Test {
protected:
};


TEST_F(IPv4AddressTests, MulticastAddresses) {
    IPv4Address lastUcast{223,255,255,255};
    EXPECT_FALSE(lastUcast.isMcast());
    IPv4Address firstMcast{224,0,0,0};
    EXPECT_TRUE(firstMcast.isMcast());
    IPv4Address lastMcast{239,255,255,255};
    EXPECT_TRUE(lastMcast.isMcast());
    IPv4Address firstMarsian{240,0,0,0};
    EXPECT_FALSE(firstMarsian.isMcast());
}

TEST_F(IPv4PrefixTests, PrefixComparisons) {
    auto p1 = IPv4Prefix::make(IPv4Address{10, 1, 3, 3}, 24);
    auto p2 = IPv4Prefix::make(IPv4Address{10, 1, 3, 0}, 24);
    auto p3 = IPv4Prefix::make(IPv4Address{10, 1, 3, 128}, 26);
    auto p4 = IPv4Prefix::make(IPv4Address{10, 1, 2, 0}, 23);

    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);

    EXPECT_TRUE(p1 >= p2);
    EXPECT_FALSE(p1 > p2);
    EXPECT_TRUE(p1 <= p2);
    EXPECT_FALSE(p1 < p2);

    EXPECT_FALSE(p1 > p3);
    EXPECT_FALSE(p1 >= p3);
    EXPECT_TRUE(p1 < p3);
    EXPECT_TRUE(p2 <= p3);

    EXPECT_TRUE(p1 > p4);
    EXPECT_TRUE(p2 >= p4);
    EXPECT_FALSE(p1 < p4);
    EXPECT_FALSE(p1 <= p4);

    EXPECT_FALSE(p1.contains(p2));
    EXPECT_TRUE(p1.contains(p3));
    EXPECT_FALSE(p1.contains(p4));
    EXPECT_TRUE(p4.contains(p1));
}

} // namespace pimc::testing


