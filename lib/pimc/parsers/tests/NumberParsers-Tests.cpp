#include <vector>
#include <string>

#include <gtest/gtest.h>
#include "pimc/formatters/Fmt.hpp"

#include "pimc/parsers/NumberParsers.hpp"

using namespace std::string_literals;

namespace pimc::testing {

class NumberParsersTests: public ::testing::Test {
protected:
};

TEST_F(NumberParsersTests, UInt64_BasicParse) {
     char const* vs{"00000000000123"};

    auto r1 = parseDecimalUInt64(vs);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 123u);

    auto r2 = parseDecimalUInt64("000"s);
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0u);
}

TEST_F(NumberParsersTests, UInt64_MaxValue) {
    char const* vs{"0018446744073709551615"};

    auto r = parseDecimalUInt64(vs);
    ASSERT_TRUE(r.hasValue());
    EXPECT_EQ(*r, 0xFFFFffffFFFFfffful);
}

TEST_F(NumberParsersTests, UInt64_Overflows) {
    for (unsigned i{6}; i <= 9; ++i) {
        std::string vs{fmt::format("00001844674407370955161{}", i)};
        auto r = parseDecimalUInt64(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 1 vs 0 -------------------+
    //                           v
    std::string vo1{"18446744073719551615"};
    auto r1 = parseDecimalUInt64(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    std::string vo2{"99999999999999999999999999999999"};
    auto r2 = parseDecimalUInt64(vo2);
    ASSERT_TRUE(r2.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, UInt64_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 18446744073709551615"s,
            "18446744073709551615 "s,
            "1844674407 709551615 "s,
            "99999999999999999999999999999999 "s,
            "-0"s,
            ""s,
            "s"s}) {
        auto r = parseDecimalUInt64(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, UInt32_BasicParse) {
    auto r1 = parseDecimalUInt32("00000433");
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 433u);

    auto r2 = parseDecimalUInt32("000"s);
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0u);
}

TEST_F(NumberParsersTests, UInt32_MaxValue) {
    std::string const vs{"000004294967295"};

    auto r = parseDecimalUInt32(vs);
    ASSERT_TRUE(r.hasValue());
    EXPECT_EQ(*r, 0xFFFFFFFFu);
}

TEST_F(NumberParsersTests, UInt32_Overflows) {
    for (unsigned i{6}; i <= 9; ++i) {
        std::string vs{fmt::format("0000429496729{}", i)};
        auto r = parseDecimalUInt32(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 7 vs 8--------------+
    //                     v
    std::string vo1{"4294968295"};
    auto r1 = parseDecimalUInt32(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    char const* vo2{"99999999999999999999999999999999"};
    auto r2 = parseDecimalUInt32(vo2);
    ASSERT_TRUE(r2.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, UInt32_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 184467"s,
            "18446744 "s,
            "1844674407 709551615 "s,
            "99999999999999999999999999999999 "s,
            "-1"s,
            ""s,
            "s"s,}) {
        auto r = parseDecimalUInt32(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, UInt16_BasicParse) {
    auto r1 = parseDecimalUInt16("000000000433"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 433u);

    auto r2 = parseDecimalUInt16("000");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0u);
}

TEST_F(NumberParsersTests, UInt16_MaxValue) {
    std::string const vs{"0000065535"};

    auto r = parseDecimalUInt16(vs);
    ASSERT_TRUE(r.hasValue());
    EXPECT_EQ(*r, 0xFFFFu);
}

TEST_F(NumberParsersTests, UInt16_Overflows) {
    for (unsigned i{6}; i <= 9; ++i) {
        std::string vs{fmt::format("0000000006553{}", i)};
        auto r = parseDecimalUInt16(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 3 vs 4 ----------+
    //                  v
    std::string vo1{"65545"};
    auto r1 = parseDecimalUInt16(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    std::string vo2{"99999999999999999999999999999999"};
    auto r2 = parseDecimalUInt16(vo2);
    ASSERT_TRUE(r2.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, UInt16_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 18467"s,
            "184744 "s,
            "1844674407 709551615 "s,
            "99999999999999999999999999999999 "s,
            "-1"s,
            ""s,
            "s"s}) {
        auto r = parseDecimalUInt16(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, UInt8_BasicParse) {
    auto r1 = parseDecimalUInt8("00000000043");
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 43u);

    auto r2 = parseDecimalUInt8("000"s);
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0u);
}

TEST_F(NumberParsersTests, UInt8_MaxValue) {
    std::string const vs{"00000255"};

    auto r = parseDecimalUInt8(vs);
    ASSERT_TRUE(r.hasValue());
    EXPECT_EQ(*r, 0xFFu);
}

TEST_F(NumberParsersTests, UInt8_Overflows) {
    for (unsigned i{6}; i <= 9; ++i) {
        std::string vs{fmt::format("00000000025{}", i)};
        auto r = parseDecimalUInt8(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 5 vs 6 --------+
    //                v
    std::string vo1{"265"};
    auto r1 = parseDecimalUInt8(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    char const* vo2{"99999999999999999999999999999999"};
    auto r2 = parseDecimalUInt8(vo2);
    ASSERT_TRUE(r2.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, UInt8_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 187"s,
            "18 "s,
            "18 7 "s,
            "99999999999999999999999999999999 "s,
            "-1"s,
            ""s,
            "s"s}) {
        auto r = parseDecimalUInt8(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, Int64_BasicParse) {
    auto r1 = parseDecimalInt64("00000000000123");
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 123l);

    auto r2 = parseDecimalInt64("+00000015353258"s);
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 15353258l);

    auto r3 = parseDecimalInt64("000"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, 0l);

    auto r4 = parseDecimalInt64("-00");
    ASSERT_TRUE(r4.hasValue());
    EXPECT_EQ(*r4, 0l);

    auto r5 = parseDecimalInt64("+000"s);
    ASSERT_TRUE(r5.hasValue());
    EXPECT_EQ(r5, 0l);

    auto r6 = parseDecimalInt64("-00008320289582005765276"s);
    ASSERT_TRUE(r6.hasValue());
    ASSERT_EQ(*r6, -8320289582005765276l);
}

TEST_F(NumberParsersTests, Int64_MaxMinValues) {
    auto r1 = parseDecimalInt64("9223372036854775807"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 0x7fffffffffffffffl);

    auto r2 = parseDecimalInt64("+0009223372036854775807");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0x7fffffffffffffffl);

    auto r3 = parseDecimalInt64("-009223372036854775808"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, -0x8000000000000000l);
}

TEST_F(NumberParsersTests, Int64_Overflows) {
    // Positive overflow
    for (unsigned i{8}; i <= 9; ++i) {
        std::string vs{fmt::format("0000922337203685477580{}", i)};
        auto r = parseDecimalInt64(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // Negative overflow
    for (unsigned i{9}; i <= 9; ++i) {
        std::string vs{fmt::format("-0000922337203685477580{}", i)};
        auto r = parseDecimalInt64(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 1 vs 0 --------------+
    //                      v
    std::string vo1{"9223372136854775807"};
    auto r1 = parseDecimalInt64(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    // 1 vs 0 --- -----------+
    //                       v
    std::string vo2{"-9223372136854775808"};
    auto r2 = parseDecimalInt64(vo2);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);

    std::string vo3{"9999999999999999999999999999999999"};
    auto r3 = parseDecimalInt64(vo3);
    ASSERT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), NumberParseError::Overflow);

    std::string vo4{"-9999999999999999999999999999999999"};
    auto r4 = parseDecimalInt64(vo3);
    ASSERT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, Int64_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 9223372036854775807"s,
            "- 9223372036854775807"s,
            "18446744073709551615 "s,
            "-18446744073709551615 "s,
            "1844674407 709551615 "s,
            "99999999999999999999999999999999 "s,
            "--0"s,
            "-+1"s,
            "++1"s,
            "+-1"}) {
        auto r = parseDecimalInt64(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, Int32_BasicParse) {
    auto r1 = parseDecimalInt32("00000003000123"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 3000123);

    auto r2 = parseDecimalInt32("+00000003000123");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 3000123);

    auto r3 = parseDecimalInt32("000"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, 0);

    auto r4 = parseDecimalInt32("-00"s);
    ASSERT_TRUE(r4.hasValue());
    EXPECT_EQ(*r4, 0);

    auto r5 = parseDecimalInt32("+000"s);
    ASSERT_TRUE(r5.hasValue());
    EXPECT_EQ(*r5, 0);

    auto r6 = parseDecimalInt32("-0000832028958"s);
    ASSERT_TRUE(r6.hasValue());
    ASSERT_EQ(*r6, -832028958);
}

TEST_F(NumberParsersTests, Int32_MaxMinValues) {
    auto r1 = parseDecimalInt32("2147483647"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 0x7fffffff);

    auto r2 = parseDecimalInt32("+000002147483647"s);
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0x7fffffff);

    auto r3 = parseDecimalInt32("-0002147483648");
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, -0x80000000);
}

TEST_F(NumberParsersTests, Int32_Overflows) {
    // Positive overflow
    for (unsigned i{8}; i <= 9; ++i) {
        std::string vs{fmt::format("0000214748364{}", i)};
        auto r = parseDecimalInt32(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // Negative overflow
    for (unsigned i{9}; i <= 9; ++i) {
        std::string vs{fmt::format("-0000214748364{}", i)};
        auto r = parseDecimalInt32(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 3 vs 4 -------------+
    //                     v
    std::string vo1{"2147484647"};
    auto r1 = parseDecimalInt32(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    // 6 vs 7 ---------------+
    //                       v
    std::string vo2{"-2147483748"};
    auto r2 = parseDecimalInt32(vo2);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);

    std::string vo3{"9999999999999999999999999999999999"};
    auto r3 = parseDecimalInt32(vo3);
    ASSERT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), NumberParseError::Overflow);

    std::string vo4{"-9999999999999999999999999999999999"};
    auto r4 = parseDecimalInt32(vo3);
    ASSERT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, Int32_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 54775807"s,
            "- 5807"s,
            "184467445 "s,
            "-18445 "s,
            "1847 7015 "s,
            "99999999999999999999999999999999999 "s,
            "--0"s,
            "-+1"s,
            "++1"s,
            "+-1"}) {
        auto r = parseDecimalInt32(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, Int16_BasicParse) {
    auto r1 = parseDecimalInt16("00000003123"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 3123);

    auto r2 = parseDecimalInt16("+00000006123");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 6123);

    auto r3 = parseDecimalInt16("000"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, 0);

    auto r4 = parseDecimalInt16("-00"s);
    ASSERT_TRUE(r4.hasValue());
    EXPECT_EQ(*r4, 0);

    auto r5 = parseDecimalInt16("+000"s);
    ASSERT_TRUE(r5.hasValue());
    EXPECT_EQ(*r5, 0);

    auto r6 = parseDecimalInt16("-00008372");
    ASSERT_TRUE(r6.hasValue());
    ASSERT_EQ(*r6, -8372);
}

TEST_F(NumberParsersTests, Int16_MaxMinValues) {
    auto r1 = parseDecimalInt16("032767"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 0x7fff);

    auto r2 = parseDecimalInt16("+0000032767");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0x7fff);

    auto r3 = parseDecimalInt16("-00032768"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, -0x8000);
}

TEST_F(NumberParsersTests, Int16_Overflows) {
    // Positive overflow
    for (unsigned i{8}; i <= 9; ++i) {
        std::string vs{fmt::format("00003276{}", i)};
        auto r = parseDecimalInt16(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // Negative overflow
    for (unsigned i{9}; i <= 9; ++i) {
        std::string vs{fmt::format("-00003276{}", i)};
        auto r = parseDecimalInt16(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 2 vs 3 --------+
    //                v
    std::string vo1{"33767"};
    auto r1 = parseDecimalInt16(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    // 6 vs 7 -----------+
    //                   v
    std::string vo2{"-32778"};
    auto r2 = parseDecimalInt16(vo2);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);

    char const* vo3{"9999999999999999999999999999999999"};
    auto r3 = parseDecimalInt16(vo3);
    ASSERT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), NumberParseError::Overflow);

    std::string vo4{"-9999999999999999999999999999999999"};
    auto r4 = parseDecimalInt16(vo3);
    ASSERT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, Int16_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 54707"s,
            "- 587"s,
            "184467445 "s,
            "-18445 "s,
            "1847 7015 "s,
            "99999999999999999999999999999999999 "s,
            "--0"s,
            "-+1"s,
            "++1"s,
            "+-1"s,
            "+"s,
            "-"s,}) {
        auto r = parseDecimalInt16(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

TEST_F(NumberParsersTests, Int8_BasicParse) {
    auto r1 = parseDecimalInt8("0000000123"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 123);

    auto r2 = parseDecimalInt8("+0000000123");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 123);

    auto r3 = parseDecimalInt8("000"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, 0);

    auto r4 = parseDecimalInt8("-00");
    ASSERT_TRUE(r4.hasValue());
    EXPECT_EQ(*r4, 0);

    auto r5 = parseDecimalInt8("+000"s);
    ASSERT_TRUE(r5.hasValue());
    EXPECT_EQ(*r5, 0);

    auto r6 = parseDecimalInt8("-000072"s);
    ASSERT_TRUE(r6.hasValue());
    ASSERT_EQ(*r6, -72);
}

TEST_F(NumberParsersTests, Int8_MaxMinValues) {
    auto r1 = parseDecimalInt8("0127"s);
    ASSERT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, 0x7f);

    auto r2 = parseDecimalInt8("+00000127");
    ASSERT_TRUE(r2.hasValue());
    EXPECT_EQ(*r2, 0x7f);

    auto r3 = parseDecimalInt8("-000128"s);
    ASSERT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, -0x80);
}

TEST_F(NumberParsersTests, Int8_Overflows) {
    // Positive overflow
    for (unsigned i{8}; i <= 9; ++i) {
        std::string vs{fmt::format("000012{}", i)};
        auto r = parseDecimalInt8(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // Negative overflow
    for (unsigned i{9}; i <= 9; ++i) {
        std::string vs{fmt::format("-000012{}", i)};
        auto r = parseDecimalInt8(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Overflow);
    }

    // 2 vs 3 --------+
    //                v
    std::string vo1{"137"};
    auto r1 = parseDecimalInt8(vo1);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), NumberParseError::Overflow);

    // 2 vs 3 ---------+
    //                 v
    std::string vo2{"-138"};
    auto r2 = parseDecimalInt8(vo2);
    ASSERT_TRUE(r1.hasError());
    EXPECT_EQ(r2.error(), NumberParseError::Overflow);

    std::string vo3{"9999999999999999999999999999999999"};
    auto r3 = parseDecimalInt8(vo3);
    ASSERT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), NumberParseError::Overflow);

    std::string vo4{"-9999999999999999999999999999999999"};
    auto r4 = parseDecimalInt8(vo3);
    ASSERT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), NumberParseError::Overflow);
}

TEST_F(NumberParsersTests, Int8_Invalids) {
    for (const auto& vs: std::vector<std::string>{
            " 54"s,
            "- 57"s,
            "114 "s,
            "-15 "s,
            "1 7 "s,
            "99999999999999999999999999999999999 "s,
            "--0"s,
            "-+1"s,
            "++1"s,
            "+-1"s,
            "+"s,
            "-"s,}) {
        auto r = parseDecimalInt8(vs);
        ASSERT_TRUE(r.hasError());
        EXPECT_EQ(r.error(), NumberParseError::Invalid);
    }
}

} // namespace pimc::testing

