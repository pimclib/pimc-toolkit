#include <cstdint>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "pimc/core/TypeUtils.hpp"

namespace pimc::testing {

class TypeUtilsTests: public ::testing::Test {
protected:
};

TEST_F(TypeUtilsTests, ValidNumericTypes) {
    EXPECT_TRUE(IsUInt_v<uint8_t>);
    EXPECT_TRUE(IsUInt_v<uint16_t>);
    EXPECT_TRUE(IsUInt_v<uint32_t>);
    EXPECT_TRUE(IsUInt_v<uint64_t>);
    EXPECT_TRUE(IsUInt_v<unsigned>);

    EXPECT_TRUE(IsSInt_v<int8_t>);
    EXPECT_TRUE(IsSInt_v<int16_t>);
    EXPECT_TRUE(IsSInt_v<int32_t>);
    EXPECT_TRUE(IsSInt_v<int64_t>);
    EXPECT_TRUE(IsSInt_v<int>);
}

TEST_F(TypeUtilsTests, InvalidNumericTypes) {
    EXPECT_FALSE(IsUInt_v<int8_t>);
    EXPECT_FALSE(IsUInt_v<int16_t>);
    EXPECT_FALSE(IsUInt_v<int32_t>);
    EXPECT_FALSE(IsUInt_v<int64_t>);
    EXPECT_FALSE(IsUInt_v<int>);
    EXPECT_FALSE(IsUInt_v<unsigned*>);

    EXPECT_FALSE(IsSInt_v<uint8_t>);
    EXPECT_FALSE(IsSInt_v<uint16_t>);
    EXPECT_FALSE(IsSInt_v<uint32_t>);
    EXPECT_FALSE(IsSInt_v<uint64_t>);
    EXPECT_FALSE(IsSInt_v<unsigned>);
    EXPECT_FALSE(IsSInt_v<int*>);
}

TEST_F(TypeUtilsTests, AllTriviallyDestructibleTypes) {
    auto constexpr OneTriviallyDestructible =
            AllTriviallyDestructible<int>;
    EXPECT_TRUE(OneTriviallyDestructible);
    auto constexpr OneNonTriviallyDestructible =
            not AllTriviallyDestructible<std::string>;
    EXPECT_TRUE(OneNonTriviallyDestructible);
    auto constexpr ManyTriviallyDestructible =
            AllTriviallyDestructible<int, bool, char>;
    EXPECT_TRUE(ManyTriviallyDestructible);
    auto constexpr ManyNonTriviallyDestructible =
            not AllTriviallyDestructible<int, unsigned, std::string, bool>;
    EXPECT_TRUE(ManyNonTriviallyDestructible);
}

TEST_F(TypeUtilsTests, AllTriviallyCopyConstructibleTypes) {
    auto constexpr OneTriviallyCopyConstructible =
            AllTriviallyCopyConstructible<int>;
    EXPECT_TRUE(OneTriviallyCopyConstructible);
    auto constexpr OneNonTriviallyCopyConstructible =
            not AllTriviallyCopyConstructible<std::string>;
    EXPECT_TRUE(OneNonTriviallyCopyConstructible);
    auto constexpr ManyTriviallyCopyConstructible =
            AllTriviallyCopyConstructible<int, bool, char>;
    EXPECT_TRUE(ManyTriviallyCopyConstructible);
    auto constexpr ManyNonTriviallyCopyConstructible =
            not AllTriviallyCopyConstructible<int, unsigned, std::string, bool>;
    EXPECT_TRUE(ManyNonTriviallyCopyConstructible);
}

TEST_F(TypeUtilsTests, SomeNonTriviallyCopyConstructibleTypes) {
    auto constexpr OneTriviallyCopyConstructible =
            not SomeNotTriviallyCopyConstructible<int>;
    EXPECT_TRUE(OneTriviallyCopyConstructible);
    auto constexpr OneNonTriviallyCopyConstructible =
            SomeNotTriviallyCopyConstructible<std::string>;
    EXPECT_TRUE(OneNonTriviallyCopyConstructible);
    auto constexpr ManyTriviallyCopyConstructible =
            not SomeNotTriviallyCopyConstructible<int, bool, char>;
    EXPECT_TRUE(ManyTriviallyCopyConstructible);
    auto constexpr ManyNonTriviallyCopyConstructible =
            SomeNotTriviallyCopyConstructible<int, unsigned, std::string, bool>;
    EXPECT_TRUE(ManyNonTriviallyCopyConstructible);
}

TEST_F(TypeUtilsTests, SomeNotCopyConstructibleTypes) {
    auto constexpr OneTriviallyCopyConstructible =
            not SomeNotCopyConstructible<int>;
    EXPECT_TRUE(OneTriviallyCopyConstructible);
    auto constexpr OneNotCopyConstructible =
            SomeNotCopyConstructible<std::unique_ptr<int>>;
    EXPECT_TRUE(OneNotCopyConstructible);
    auto constexpr ManyTriviallyCopyConstructible =
            not SomeNotCopyConstructible<int, bool, char>;
    EXPECT_TRUE(ManyTriviallyCopyConstructible);
    auto constexpr ManyNotCopyConstructible =
            SomeNotCopyConstructible<int, unsigned, std::unique_ptr<int>, bool>;
    EXPECT_TRUE(ManyNotCopyConstructible);
}

} // namespace pimc:testing