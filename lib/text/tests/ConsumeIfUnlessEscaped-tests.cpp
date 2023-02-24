#include <cctype>
#include <string_view>

#include <gtest/gtest.h>

#include "pimc/text/CString.hpp"
#include "pimc/text/MemoryBuffer.hpp"
#include "pimc/text/ConsumeIfUnlessEscaped.hpp"

using namespace std::string_literals;

namespace pimc::testing {

class ConsumeIfUnlessEscapedTests : public ::testing::Test {
protected:
    static auto notSpace() { return [] (char c) { return not std::isspace(c); }; }
    static auto bsEscape() { return [] (char c) -> bool { return c == '\\'; }; }
    static auto cons(fmt::memory_buffer& mb) { return [&mb] (char c) { mb.push_back(c); }; }

    std::string_view tsv1{R"(abc\x123\ xyz\)"};
    std::string      exp1{R"(abc\x123 xyz\)"};

    std::string_view tsv2{R"(abc\\123\ xyz\  )"};
    std::string      exp2{R"(abc\123 xyz )"};

    const char* v3   = R"(\ abc\ \123\  )";
    std::string exp3 = R"( abc \123 )";
};

TEST_F(ConsumeIfUnlessEscapedTests, Basic1) {
    auto& mb = getMemoryBuffer();
    auto [wl, ii] = consumeIfUnlessEscaped(
            tsv1, notSpace(), bsEscape(), cons(mb));
    auto result = fmt::to_string(mb);

    EXPECT_EQ(wl, 13u);
    EXPECT_EQ(wl, exp1.size());
    EXPECT_EQ(ii, tsv1.end());
    EXPECT_EQ(result, exp1);
}

TEST_F(ConsumeIfUnlessEscapedTests, Basic2) {
    auto& mb = getMemoryBuffer();
    auto [wl, ii] = consumeIfUnlessEscaped(
            tsv2, notSpace(), bsEscape(), cons(mb));
    auto result = fmt::to_string(mb);

    EXPECT_EQ(wl, exp2.size());
    EXPECT_NE(ii, tsv2.end());
    EXPECT_EQ(ii, tsv2.end()-1);
    EXPECT_EQ(result, exp2);
}

TEST_F(ConsumeIfUnlessEscapedTests, Basic3) {
    auto& mb = getMemoryBuffer();
    pimc::views::cstr rv3{v3};
    auto [wl, ii] = consumeIfUnlessEscaped(
            rv3, notSpace(), bsEscape(), cons(mb));
    auto result = fmt::to_string(mb);

    EXPECT_EQ(wl, exp3.size());
    EXPECT_NE(ii, rv3.end());
    EXPECT_EQ(result, exp3);
}

} // namespace pimc::testing