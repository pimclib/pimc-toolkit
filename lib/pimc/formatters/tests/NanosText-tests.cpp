#include <gtest/gtest.h>

#include "pimc/formatters/NanosText.hpp"

namespace pimc::testing {

class NanosTextTests: public ::testing::Test {
protected:
};

TEST_F(NanosTextTests, Basic) {
    NanosText nt;

    char const* t;
    uint64_t c;

    std::tie(t, c) = nt.prc(123456789ul, 9);
    EXPECT_STREQ(t, "123456789");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 8);
    EXPECT_STREQ(t, "12345679");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 7);
    EXPECT_STREQ(t, "1234568");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 6);
    EXPECT_STREQ(t, "123457");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 5);
    EXPECT_STREQ(t, "12346");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 4);
    EXPECT_STREQ(t, "1235");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 3);
    EXPECT_STREQ(t, "123");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 2);
    EXPECT_STREQ(t, "12");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123456789ul, 1);
    EXPECT_STREQ(t, "1");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123450000ul, 6);
    // zeroes at the end are truncated
    EXPECT_STREQ(t, "12345");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123450000ul, 5);
    EXPECT_STREQ(t, "12345");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123450000ul, 4);
    EXPECT_STREQ(t, "1235");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(999500000ul, 5);
    EXPECT_STREQ(t, "9995");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(999500000ul, 3);
    EXPECT_STREQ(t, "");
    EXPECT_EQ(c, 1);

}

} // namespace pimc::testing