#include <gtest/gtest.h>

#include "pimc/text/NanosText.hpp"

namespace pimc::testing {

class NanosTextTests: public ::testing::Test {
protected:
};

TEST_F(NanosTextTests, Basic) {
    NanosText nt;

    char const* t;
    uint64_t c;

    std::tie(t, c) = nt.prc(123450000, 6);
    // zeroes at the end are truncated
    EXPECT_STREQ(t, "12345");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123450000, 5);
    EXPECT_STREQ(t, "12345");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(123450000, 4);
    EXPECT_STREQ(t, "1235");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(999500000, 5);
    EXPECT_STREQ(t, "9995");
    EXPECT_EQ(c, 0);

    std::tie(t, c) = nt.prc(999500000, 3);
    EXPECT_STREQ(t, "");
    EXPECT_EQ(c, 1);

}

} // namespace pimc::testing