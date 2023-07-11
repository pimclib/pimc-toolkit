#include <utility>
#include <type_traits>
#include <concepts>
#include <string>

#include <gtest/gtest.h>
#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Result.hpp"

#include "Optional-Result-TestUtils.hpp"

using namespace std::string_literals;

namespace pimc::testing {

class ResultTests: public ::testing::Test {
protected:

    char const* cstrValue1 = "ed25519 is a very nice elliptic curve";
    std::string strValue1 = "ed25519 is a very nice elliptic curve"s;
};

TEST_F(ResultTests, TEDefaultConstructibleProperties) {
    constexpr auto DefaultConstructibleC =
            std::is_default_constructible_v<Result<int, int>>;
    EXPECT_TRUE(DefaultConstructibleC);

    std::string s{};
    auto r1 = Result<std::string, int>{};

    EXPECT_TRUE(*r1 == s);

    constexpr auto NotDefaultConstructibleC =
            not std::is_default_constructible_v<Result<NotDefaultConstructible, int>>;
    EXPECT_TRUE(NotDefaultConstructibleC);
}

TEST_F(ResultTests, TECopyConstructibleProperties) {
    constexpr auto TriviallyCopyConstructibleC =
            std::is_trivially_copy_constructible_v<Result<int, int>>;
    EXPECT_TRUE(TriviallyCopyConstructibleC);

    int const v1{25519};
    const Result<int, int> r1{v1};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_TRUE(*r1 == v1);

    auto const f2 = fail(25519);
    Result<int, int> const r2{f2};
    EXPECT_TRUE(r2.hasError());
    EXPECT_TRUE(r2 == f2);

    constexpr auto NotTriviallyCopyConstructibleValueC =
            not std::is_trivially_copy_constructible_v<Result<std::string, int>>;
    EXPECT_TRUE(NotTriviallyCopyConstructibleValueC);

    constexpr auto CopyConstructibleValueC =
            std::is_copy_constructible_v<Result<std::string, int>>;
    EXPECT_TRUE(CopyConstructibleValueC);

    Result<std::string, int> const r3{cstrValue1};
    EXPECT_TRUE(r3.hasValue());
    EXPECT_TRUE(*r3 == strValue1);

    Result<std::string, int> const r4a{cstrValue1};
    auto const r4b = r4a;
    EXPECT_TRUE(r4b.hasValue());
    EXPECT_TRUE(*r4b == strValue1);

    auto const f5 = fail(25519);
    Result<std::string, int> const r5a{f5};
    auto const r5b = r5a;
    auto r5ahe = r5a.hasError();
    auto r5bhe = r5b.hasError();
    EXPECT_TRUE(r5ahe);
    EXPECT_TRUE(r5bhe);
    EXPECT_TRUE(r5b == f5);

    constexpr auto NotTriviallyCopyConstructibleErrorC =
            not std::is_trivially_copy_constructible_v<Result<int, std::string>>;
    EXPECT_TRUE(NotTriviallyCopyConstructibleErrorC);

    constexpr auto CopyConstructibleErrorC =
            std::is_copy_constructible_v<Result<int, std::string>>;
    EXPECT_TRUE(CopyConstructibleErrorC);

    int const v6 = 25519;
    Result<int, std::string> const r6a{v6};
    auto const r6b = r6a;
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_TRUE(*r6b == v6);

    auto const f7 = fail(cstrValue1);
    Result<int, std::string> const r7a{f7};
    auto const r7b = r7a;
    EXPECT_TRUE(r7a.hasError());
    EXPECT_TRUE(r7b.hasError());
    EXPECT_TRUE(r7b == f7);

    constexpr auto NotTriviallyCopyConstructibleValueErrorC =
            not std::is_trivially_copy_constructible_v<Result<std::string, std::string>>;
    EXPECT_TRUE(NotTriviallyCopyConstructibleValueErrorC);

    constexpr auto CopyConstructibleValueErrorC =
            std::is_copy_constructible_v<Result<std::string, std::string>>;
    EXPECT_TRUE(CopyConstructibleValueErrorC);

    Result<std::string, std::string> const r8a{cstrValue1};
    auto const r8b = r8a;
    EXPECT_TRUE(r8a.hasValue());
    EXPECT_TRUE(r8b.hasValue());
    EXPECT_TRUE(*r8a == strValue1);
    EXPECT_TRUE(*r8b == strValue1);

    auto const f9 = fail(strValue1);
    Result<std::string, std::string> r9a{f9};
    auto const r9b = r9a;
    EXPECT_TRUE(r9a.hasError());
    EXPECT_TRUE(r9b.hasError());
    EXPECT_TRUE(r9a == f9);
    EXPECT_TRUE(r9b == f9);

    constexpr auto NotCopyConstructibleValueC =
            not std::is_copy_constructible_v<Result<NotCopyOrMoveable, int>>;
    EXPECT_TRUE(NotCopyConstructibleValueC);

    constexpr auto NotCopyConstructibleErrorC =
            not std::is_copy_constructible_v<Result<int, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyConstructibleErrorC);

    constexpr auto NotCopyConstructibleValueErrorC =
            not std::is_copy_constructible_v<Result<NotCopyOrMoveable, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyConstructibleValueErrorC);
}

TEST_F(ResultTests, TEMoveConstructibleProperties) {
    constexpr auto TriviallyMoveConstructibleC =
            std::is_trivially_move_constructible_v<Result<int, int>>;
    EXPECT_TRUE(TriviallyMoveConstructibleC);

    int const v1{25519};
    const Result<int, int> r1a{v1};
    auto const r1b = std::move(r1a);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_TRUE(*r1b == v1);

    auto const f2 = fail(25519);
    Result<int, int> const r2a{f2};
    auto const r2b = std::move(r2a);
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_TRUE(r2b == f2);

    constexpr auto NotTriviallyMoveConstructibleValueC =
            not std::is_trivially_move_constructible_v<Result<MoveOnly<std::string>, int>>;
    EXPECT_TRUE(NotTriviallyMoveConstructibleValueC);

    constexpr auto MoveConstructibleValueC =
            std::is_move_constructible_v<Result<MoveOnly<std::string>, int>>;
    EXPECT_TRUE(MoveConstructibleValueC);

    Result<MoveOnly<std::string>, int> r4a{cstrValue1};
    EXPECT_TRUE(r4a.hasValue());
    EXPECT_TRUE(*r4a == strValue1);
    auto const r4b = std::move(r4a);
    EXPECT_TRUE(r4a.hasValue());
    EXPECT_TRUE(*r4a == ""s);
    EXPECT_TRUE(r4b.hasValue());
    EXPECT_TRUE(*r4b == strValue1);

    auto const f5 = fail(25519);
    Result<MoveOnly<std::string>, int> r5a{f5};
    EXPECT_TRUE(r5a.hasError());
    EXPECT_TRUE(r5a.error() == 25519);
    auto const r5b = std::move(r5a);
    EXPECT_TRUE(r5a.hasError());
    EXPECT_TRUE(r5a.error() == 25519);
    EXPECT_TRUE(r5b.hasError());
    EXPECT_TRUE(r5b.error() == 25519);
    EXPECT_TRUE(r5b == f5);

    constexpr auto NotTriviallyMoveConstructibleErrorC =
            not std::is_trivially_move_constructible_v<Result<int, MoveOnly<std::string>>>;
    EXPECT_TRUE(NotTriviallyMoveConstructibleErrorC);

    constexpr auto MoveConstructibleErrorC =
            std::is_move_constructible_v<Result<int, MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveConstructibleErrorC);

    int const v6 = 25519;
    Result<int, MoveOnly<std::string>> r6a{v6};
    auto const r6b = std::move(r6a);
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_TRUE(*r6b == v6);

    auto const f7 = fail(cstrValue1);
    Result<int, MoveOnly<std::string>> r7a{f7};
    EXPECT_TRUE(r7a.hasError());
    EXPECT_TRUE(r7a.error() == strValue1);
    auto const r7b = std::move(r7a);
    EXPECT_TRUE(r7a.hasError());
    EXPECT_TRUE(r7a.error() == ""s);
    EXPECT_TRUE(r7b.hasError());
    EXPECT_TRUE(r7b.error() == f7.error());

    constexpr auto NotTriviallyMoveConstructibleValueErrorC =
            not std::is_trivially_move_constructible_v<
                    Result<MoveOnly<std::string>, MoveOnly<std::string>>>;
    EXPECT_TRUE(NotTriviallyMoveConstructibleValueErrorC);

    constexpr auto MoveConstructibleValueErrorC =
            std::is_move_constructible_v<
                    Result<MoveOnly<std::string>, MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveConstructibleValueErrorC);

    Result<MoveOnly<std::string>, MoveOnly<std::string>> r8a{cstrValue1};
    EXPECT_TRUE(r8a.hasValue());
    EXPECT_TRUE(*r8a == strValue1);
    auto const r8b = std::move(r8a);
    EXPECT_TRUE(r8a.hasValue());
    EXPECT_TRUE(*r8a == ""s);
    EXPECT_TRUE(r8b.hasValue());
    EXPECT_TRUE(*r8b == strValue1);

    auto const f9 = fail(cstrValue1);
    Result<MoveOnly<std::string>, MoveOnly<std::string>> r9a{f9};
    EXPECT_TRUE(r9a.hasError());
    EXPECT_TRUE(r9a.error() == strValue1);
    auto const r9b = std::move(r9a);
    EXPECT_TRUE(r9a.hasError());
    EXPECT_TRUE(r9b.hasError());
    EXPECT_TRUE(r9a.error() == ""s);
    EXPECT_TRUE(r9b.error() == strValue1);

    constexpr auto NotMoveConstructibleValueC =
            not std::is_move_constructible_v<Result<NotCopyOrMoveable, int>>;
    EXPECT_TRUE(NotMoveConstructibleValueC);

    constexpr auto NotMoveConstructibleErrorC =
            not std::is_move_constructible_v<Result<int, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveConstructibleErrorC);

    constexpr auto NotMoveConstructibleValueErrorC =
            not std::is_move_constructible_v<Result<NotCopyOrMoveable, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveConstructibleValueErrorC);
}

TEST_F(ResultTests, TEConversionConstructionProperties) {
    constexpr auto ConstructibleC =
            std::is_constructible_v<
                    Result<std::string, std::string>,
                    Result<char const*, char const*> const&>;
    EXPECT_TRUE(ConstructibleC);

    constexpr auto ConvertibleC =
            std::is_convertible_v<
                    Result<char const*, char const*> const&,
                    Result<std::string, std::string>>;
    EXPECT_TRUE(ConvertibleC);

    Result<char const*, char const*> const r1a{cstrValue1};
    auto const r1b = r1a;
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_TRUE(*r1b == cstrValue1);

    auto const f2 = fail(cstrValue1);
    Result<char const*, char const*> const r2a{f2};
    auto const r2b = r2a;
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2a.error() == cstrValue1);
    EXPECT_TRUE(r2b.hasError());
    EXPECT_TRUE(r2b == f2);

    constexpr auto NotConstructibleC1 =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<std::string, int> const&>;
    EXPECT_TRUE(NotConstructibleC1);

    constexpr auto NotConstructibleC2 =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<int, std::string> const&>;
    EXPECT_TRUE(NotConstructibleC2);

    constexpr auto NotConstructibleC3 =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<std::string, std::string> const&>;
    EXPECT_TRUE(NotConstructibleC3);
}

TEST_F(ResultTests, TEExplicitConversionCopyConstructionProperties) {
    constexpr auto ConstructibleC1 =
            std::is_constructible_v<
                    Result<Explicit<std::string>, std::string>,
                    Result<std::string, char const*> const&>;
    EXPECT_TRUE(ConstructibleC1);

    constexpr auto NotConvertibleC1 =
            not std::is_convertible_v<
                    Result<std::string, char const*> const&,
                    Result<Explicit<std::string>, std::string>>;
    EXPECT_TRUE(NotConvertibleC1);

    Result<std::string, char const*> const r1a{cstrValue1};
    Result<Explicit<std::string>, std::string> const r1b{r1a};
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_EQ(*r1a, strValue1);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);

    auto const f2 = fail(cstrValue1);
    Result<std::string, char const*> const r2a{f2};
    Result<Explicit<std::string>, std::string> const r2b{r2a};
    EXPECT_TRUE(r2a.hasError());
    EXPECT_EQ(r2a, f2);
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2b, f2);

    constexpr auto ConstructibleC2 =
            std::is_constructible_v<
                    Result<std::string, Explicit<std::string>>,
                    Result<char const*, std::string> const&>;
    EXPECT_TRUE(ConstructibleC2);

    constexpr auto NotConvertibleC2 =
            not std::is_convertible_v<
                    Result<char const*, std::string> const&,
                    Result<std::string, Explicit<std::string>>>;
    EXPECT_TRUE(NotConvertibleC2);

    Result<char const*, std::string> const r3a{cstrValue1};
    Result<std::string, Explicit<std::string>> const r3b{r3a};
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_EQ(*r3a, strValue1);
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3b, strValue1);

    auto const f4 = fail(cstrValue1);
    Result<char const*, std::string> const r4a{f4};
    Result<std::string, Explicit<std::string>> const r4b{r4a};
    EXPECT_TRUE(r4a.hasError());
    EXPECT_EQ(r4a, f2);
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4b.error(), f2.error());

    constexpr auto ConstructibleC3 =
            std::is_constructible_v<
                    Result<Explicit<std::string>, Explicit<std::string>>,
                    Result<std::string, std::string> const&>;
    EXPECT_TRUE(ConstructibleC3);

    constexpr auto NotConvertibleC3 =
            not std::is_convertible_v<
                    Result<std::string, std::string> const&,
                    Result<Explicit<std::string>, Explicit<std::string>>>;
    EXPECT_TRUE(NotConvertibleC3);

    Result<std::string, std::string> const r5a{cstrValue1};
    Result<Explicit<std::string>, Explicit<std::string>> const r5b{r5a};
    EXPECT_TRUE(r5a.hasValue());
    EXPECT_EQ(*r5a, strValue1);
    EXPECT_TRUE(r5b.hasValue());
    EXPECT_EQ(*r5b, strValue1);

    auto const f6 = fail(cstrValue1);
    Result<std::string, std::string> const r6a{f6};
    Result<Explicit<std::string>, Explicit<std::string>> const r6b{r6a};
    EXPECT_TRUE(r6a.hasError());
    EXPECT_EQ(r6a, f2);
    EXPECT_TRUE(r6b.hasError());
    EXPECT_EQ(r6b.error(), f2.error());

    constexpr auto NotConstructibleValueC =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<std::string, int>>;
    EXPECT_TRUE(NotConstructibleValueC);

    constexpr auto NotConstructibleErrorC =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<int, std::string>>;
    EXPECT_TRUE(NotConstructibleErrorC);

    constexpr auto NotConstructibleValueErrorC =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<std::string, std::string>>;
    EXPECT_TRUE(NotConstructibleValueErrorC);
}

TEST_F(ResultTests, TEImplicitConversionMoveConstructionProperties) {
    constexpr auto ConstructibleC1 =
            std::is_constructible_v<
                    Result<MoveOnly<std::string>, MoveOnly<std::string>>,
                    Result<std::string, std::string>&&>;
    EXPECT_TRUE(ConstructibleC1);

    constexpr auto ConvertibleC1 =
            std::is_convertible_v<
                    Result<std::string, std::string>&&,
                    Result<MoveOnly<std::string>, MoveOnly<std::string>>>;
    EXPECT_TRUE(ConvertibleC1);

    Result<std::string, std::string> r1a{cstrValue1};
    auto const r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);

    auto const f2 = fail(cstrValue1);
    Result<std::string, std::string> r2a{f2};
    auto const r2b = std::move(r2a);
    EXPECT_TRUE(r2a.hasError());
    EXPECT_EQ(r2a.error(), ""s);
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2b.error(), strValue1);
    EXPECT_EQ(r2b, f2);

    constexpr auto NotConstructibleC2 =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<std::string, int>&&>;
    EXPECT_TRUE(NotConstructibleC2);

    constexpr auto NotConstructibleC3 =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<int, std::string>&&>;
    EXPECT_TRUE(NotConstructibleC3);

    constexpr auto NotConstructibleC4 =
            not std::is_constructible_v<
                    Result<int, int>,
                    Result<std::string, std::string>&&>;
    EXPECT_TRUE(NotConstructibleC4);
}

TEST_F(ResultTests, TEExplicitConversionMoveConstructionProperties) {
    constexpr auto ConstructibleC1 =
            std::is_constructible_v<
                    Result<Explicit<MoveOnly<std::string>>, std::string>,
                    Result<std::string, char const*>&&>;
    EXPECT_TRUE(ConstructibleC1);

    constexpr auto NotConvertibleC2 =
            not std::is_convertible_v<
                    Result<std::string, char const*>&&,
                    Result<Explicit<MoveOnly<std::string>>, std::string>>;
    EXPECT_TRUE(NotConvertibleC2);

    Result<std::string, char const*> r1a{cstrValue1};
    Result<Explicit<MoveOnly<std::string>>, std::string> const r1b{std::move(r1a)};
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);

    auto const f2 = fail(cstrValue1);
    Result<std::string, char const*> r2a{f2};
    Result<Explicit<MoveOnly<std::string>>, std::string> const r2b{std::move(r2a)};
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2b, f2);

    constexpr auto ConstructibleC3 =
            std::is_constructible_v<
                    Result<std::string, Explicit<MoveOnly<std::string>>>,
                    Result<char const*, std::string>&&>;
    EXPECT_TRUE(ConstructibleC3);

    constexpr auto NotConvertibleC4 =
            not std::is_convertible_v<
                    Result<char const*, std::string>&&,
                    Result<std::string, Explicit<MoveOnly<std::string>>>>;
    EXPECT_TRUE(NotConvertibleC4);

    Result<char const*, std::string> r3a{cstrValue1};
    Result<std::string, Explicit<MoveOnly<std::string>>> const r3b{std::move(r3a)};
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_STREQ(*r3a, cstrValue1);
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3b, strValue1);

    auto const f4 = fail(cstrValue1);
    Result<char const*, std::string> r4a{f4};
    Result<std::string, Explicit<MoveOnly<std::string>>> const r4b{std::move(r4a)};
    EXPECT_TRUE(r4a.hasError());
    EXPECT_EQ(r4a.error(), ""s);
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4b.error(), strValue1);

    constexpr auto ConstructibleC5 =
            std::is_constructible_v<
                    Result<Explicit<MoveOnly<std::string>>, Explicit<MoveOnly<std::string>>>,
                    Result<std::string, std::string>&&>;
    EXPECT_TRUE(ConstructibleC5);

    // TODO Check me
    constexpr auto NotConvertibleC6 =
            not std::is_convertible_v<
                    Result<std::string, std::string>&&,
                    Result<Explicit<MoveOnly<std::string>>, Explicit<MoveOnly<std::string>>>
                    >;
    EXPECT_TRUE(NotConvertibleC6);

    Result<std::string, std::string> r5a{cstrValue1};
    Result<Explicit<MoveOnly<std::string>>, Explicit<MoveOnly<std::string>>>
    const r5b{std::move(r5a)};
    EXPECT_TRUE(r5a.hasValue());
    EXPECT_EQ(*r5a, ""s);
    EXPECT_TRUE(r5b.hasValue());
    EXPECT_EQ(*r5b, strValue1);

    auto const f6 = fail(cstrValue1);
    Result<std::string, std::string> r6a{f6};
    Result<Explicit<MoveOnly<std::string>>, Explicit<MoveOnly<std::string>>>
    const r6b{std::move(r6a)};
    EXPECT_TRUE(r6a.hasError());
    EXPECT_EQ(r6a.error(), ""s);
    EXPECT_TRUE(r6b.hasError());
    EXPECT_EQ(r6b.error(), strValue1);
}

TEST_F(ResultTests, TEInPlaceValueConstructionProperties) {
    auto const v1 = std::string{cstrValue1, 34ul};
    Result<std::string, int> const r1{InPlaceValue, cstrValue1, 34ul};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(r1, v1);
}

TEST_F(ResultTests, TEInPlaceValueInitializerListConstructionProperties) {
    auto const v1 = std::string{IL_ED25519, std::allocator<char>{}};
    Result<std::string, int> const r1{InPlaceValue, IL_ED25519, std::allocator<char>{}};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(r1, v1);
}

TEST_F(ResultTests, TEInPlaceErrorConstructionProperties) {
    auto const v1 = std::string{cstrValue1, 34ul};
    Result<int, std::string> const r1{InPlaceError, cstrValue1, 34ul};
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), v1);
}

TEST_F(ResultTests, TEInPlaceErrorInitializerListConstructionProperties) {
    auto const v1 = std::string{IL_ED25519, std::allocator<char>{}};
    Result<int, std::string> const r1{InPlaceError, IL_ED25519, std::allocator<char>{}};
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), v1);
}

TEST_F(ResultTests, TEFailureConstLValueRefConstructionProperties) {
    auto const f1 = fail<std::string>(cstrValue1);
    Result<int, std::string> const r1{f1};
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1, f1);
}

TEST_F(ResultTests, TEFailureRValueRefConstructionProperties) {
    auto f1a = fail<std::string>(cstrValue1);
    auto const f1b = f1a;
    Result<int, MoveOnly<std::string>> const r1{std::move(f1a)};
    EXPECT_EQ(f1a.error(), ""s);
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1, f1b);
}

TEST_F(ResultTests, TEDestructionProperties) {
    constexpr auto TriviallyDestructibleC1 =
            std::is_trivially_destructible_v<Result<int, int>>;
    EXPECT_TRUE(TriviallyDestructibleC1);

    constexpr auto NotTriviallyDestructibleC2 =
            not std::is_trivially_destructible_v<Result<BoolSetterOnDestr, int>>;
    EXPECT_TRUE(NotTriviallyDestructibleC2);
    bool called1{false};
    {
        Result<BoolSetterOnDestr, int> const r1{InPlaceValue, &called1};
    };
    EXPECT_TRUE(called1);

    constexpr auto NotTriviallyDestructibleC3 =
            not std::is_trivially_destructible_v<Result<int, BoolSetterOnDestr>>;
    EXPECT_TRUE(NotTriviallyDestructibleC3);
    bool called2{false};
    {
        Result<int, BoolSetterOnDestr> const r2{InPlaceError, &called2};
    };
    EXPECT_TRUE(called2);

    constexpr auto NotTriviallyDestructibleC4 =
            not std::is_trivially_destructible_v<Result<BoolSetterOnDestr, BoolSetterOnDestr>>;
    EXPECT_TRUE(NotTriviallyDestructibleC4);
    bool called3{false};
    {
        Result<BoolSetterOnDestr, BoolSetterOnDestr> const r1{InPlaceValue, &called3};
    };
    EXPECT_TRUE(called3);

    bool called4{false};
    {
        Result<BoolSetterOnDestr, BoolSetterOnDestr> const r1{InPlaceError, &called4};
    };
    EXPECT_TRUE(called4);

}

TEST_F(ResultTests, TECopyAssignabilityProperties) {
    constexpr auto CopyAssignableC1 =
            std::is_copy_assignable_v<Result<std::string, int>>;
    EXPECT_TRUE(CopyAssignableC1);

    Result<std::string, int> const r1a{cstrValue1};
    Result<std::string, int> r1b;
    r1b = r1a;
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, strValue1);
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto CopyAssignableC2 =
            std::is_copy_assignable_v<Result<int, std::string>>;
    EXPECT_TRUE(CopyAssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<int, std::string> const r2a{f2};
    Result<int, std::string> r2b{InPlaceError};
    r2b = r2a;
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a, f2);
    EXPECT_EQ(r2b, f2);

    constexpr auto CopyAssignableC3 =
            std::is_copy_assignable_v<Result<std::string, std::string>>;
    EXPECT_TRUE(CopyAssignableC3);

    Result<std::string, std::string> const r3a{cstrValue1};
    Result<std::string, std::string> r3b;
    r3b = r3a;
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3a, strValue1);
    EXPECT_EQ(*r3b, strValue1);

    auto const f4 = fail(cstrValue1);
    Result<std::string, std::string> const r4a{f4};
    Result<std::string, std::string> r4b;
    r4b = r4a;
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a, f4);
    EXPECT_EQ(r4b, f4);

    constexpr auto NotCopyAssignableC4 =
            not std::is_copy_assignable_v<Result<NotCopyOrMoveable, int>>;
    EXPECT_TRUE(NotCopyAssignableC4);

    constexpr auto NotCopyAssignableC5 =
            not std::is_copy_assignable_v<Result<int, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyAssignableC5);

    constexpr auto NotCopyAssignableC6 =
            not std::is_copy_assignable_v<Result<NotCopyOrMoveable, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyAssignableC6);

    auto const f5 = fail(cstrValue1);
    Result<BoolSetterOnDestr, std::string> r5a{f5};
    bool called5{false};
    Result<BoolSetterOnDestr, std::string> r5b{&called5};
    EXPECT_TRUE(r5a.hasError());
    EXPECT_TRUE(r5b.hasValue());
    r5b = r5a;
    EXPECT_TRUE(r5b.hasError());
    EXPECT_EQ(r5b, f5);
    EXPECT_TRUE(called5);

    Result<std::string, BoolSetterOnDestr> r6a{cstrValue1};
    bool called6{false};
    Result<std::string, BoolSetterOnDestr> r6b{fail(&called6)};
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasError());
    r6b = r6a;
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_EQ(*r6b, strValue1);
    EXPECT_TRUE(called6);

    constexpr auto TriviallyCopyAssignableC7 =
            std::is_trivially_copy_assignable_v<Result<int, int>>;
    EXPECT_TRUE(TriviallyCopyAssignableC7);
}

TEST_F(ResultTests, TEMoveAssignabilityProperties) {
    constexpr auto MoveAssignableC1 =
            std::is_move_assignable_v<Result<std::string, int>>;
    EXPECT_TRUE(MoveAssignableC1);

    Result<std::string, int> r1a{cstrValue1};
    Result<std::string, int> r1b;
    r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto MoveAssignableC2 =
            std::is_move_assignable_v<Result<int, std::string>>;
    EXPECT_TRUE(MoveAssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<int, std::string> r2a{f2};
    Result<int, std::string> r2b{InPlaceError};
    r2b = std::move(r2a);
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a.error(), ""s);
    EXPECT_EQ(r2b, f2);

    constexpr auto MoveAssignableC3 =
            std::is_move_assignable_v<Result<std::string, std::string>>;
    EXPECT_TRUE(MoveAssignableC3);

    Result<std::string, std::string> r3a{cstrValue1};
    Result<std::string, std::string> r3b;
    r3b = std::move(r3a);
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3a, ""s);
    EXPECT_EQ(*r3b, strValue1);

    auto const f4 = fail(cstrValue1);
    Result<std::string, std::string> r4a{f4};
    Result<std::string, std::string> r4b;
    r4b = std::move(r4a);
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a.error(), ""s);
    EXPECT_EQ(r4b, f4);

    constexpr auto NotMoveAssignableC4 =
            not std::is_move_assignable_v<Result<NotCopyOrMoveable, int>>;
    EXPECT_TRUE(NotMoveAssignableC4);

    constexpr auto NotMoveAssignableC5 =
            not std::is_move_assignable_v<Result<int, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveAssignableC5);

    constexpr auto NotMoveAssignableC6 =
            not std::is_move_assignable_v<Result<NotCopyOrMoveable, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveAssignableC6);

    auto const f5 = fail(cstrValue1);
    Result<BoolSetterOnDestr, std::string> r5a{f5};
    bool called5{false};
    Result<BoolSetterOnDestr, std::string> r5b{&called5};
    EXPECT_TRUE(r5a.hasError());
    EXPECT_TRUE(r5b.hasValue());
    r5b = std::move(r5a);
    EXPECT_TRUE(r5a.hasError());
    EXPECT_EQ(r5a.error(), ""s);
    EXPECT_TRUE(r5b.hasError());
    EXPECT_EQ(r5b, f5);
    EXPECT_TRUE(called5);

    Result<std::string, BoolSetterOnDestr> r6a{cstrValue1};
    bool called6{false};
    Result<std::string, BoolSetterOnDestr> r6b{fail(&called6)};
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasError());
    r6b = std::move(r6a);
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_EQ(*r6a, ""s);
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_EQ(*r6b, strValue1);
    EXPECT_TRUE(called6);

    constexpr auto TriviallyMoveAssignableC7 =
            std::is_trivially_move_assignable_v<Result<int, int>>;
    EXPECT_TRUE(TriviallyMoveAssignableC7);
}

TEST_F(ResultTests, TEThrowingAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Result<Throwing<std::string>, int>,
                    Result<std::string, int> const&>;
    EXPECT_TRUE(AssignableC1);

    constexpr auto NotNothrowAssignableC1 =
            not std::is_nothrow_assignable_v<
                    Result<Throwing<std::string>, int>,
                    Result<std::string, int> const&>;
    EXPECT_TRUE(NotNothrowAssignableC1);

    constexpr auto AssignableC2 =
            std::is_assignable_v<
                    Result<int, Throwing<std::string>>,
                    Result<int, std::string> const&>;
    EXPECT_TRUE(AssignableC2);

    constexpr auto NotNothrowAssignableC2 =
            not std::is_nothrow_assignable_v<
                    Result<int, Throwing<std::string>>,
                    Result<int, std::string> const&>;
    EXPECT_TRUE(NotNothrowAssignableC2);

    constexpr auto AssignableC3 =
            std::is_assignable_v<
                    Result<Throwing<std::string>, int>,
                    Result<std::string, int>&&>;
    EXPECT_TRUE(AssignableC3);

    constexpr auto NotNothrowAssignableC3 =
            not std::is_nothrow_assignable_v<
                    Result<Throwing<std::string>, int>,
                    Result<std::string, int>&&>;
    EXPECT_TRUE(NotNothrowAssignableC3);

    constexpr auto AssignableC4 =
            std::is_assignable_v<
                    Result<int, Throwing<std::string>>,
                    Result<int, std::string>&&>;
    EXPECT_TRUE(AssignableC4);

    constexpr auto NotNothrowAssignableC4 =
            not std::is_nothrow_assignable_v<
                    Result<int, Throwing<std::string>>,
                    Result<int, std::string>&&>;
    EXPECT_TRUE(NotNothrowAssignableC4);
}

TEST_F(ResultTests, TEConversionCopyAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Result<CopyOnly<std::string>, int>,
                    Result<std::string, long> const&>;
    EXPECT_TRUE(AssignableC1);

    Result<std::string, int> const r1a{cstrValue1};
    Result<CopyOnly<std::string>, int> r1b;
    r1b = r1a;
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, strValue1);
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto AssignableC2 =
            std::is_assignable_v<
                    Result<int, CopyOnly<std::string>>,
                    Result<long, std::string> const&>;
    EXPECT_TRUE(AssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<int, std::string> const r2a{f2};
    Result<int, CopyOnly<std::string>> r2b{InPlaceError};
    r2b = r2a;
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a, f2);
    EXPECT_EQ(r2b, f2);

    constexpr auto AssignableC3 =
            std::is_assignable_v<
                    Result<CopyOnly<std::string>, CopyOnly<std::string>>,
                    Result<std::string, std::string> const&>;
    EXPECT_TRUE(AssignableC3);

    Result<std::string, std::string> const r3a{cstrValue1};
    Result<CopyOnly<std::string>, CopyOnly<std::string>> r3b;
    r3b = r3a;
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3a, strValue1);
    EXPECT_EQ(*r3b, strValue1);

    auto const f4 = fail(cstrValue1);
    Result<std::string, std::string> const r4a{f4};
    Result<CopyOnly<std::string>, CopyOnly<std::string>> r4b;
    r4b = r4a;
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a, f4);
    EXPECT_EQ(r4b, f4);

    auto const f5 = fail(cstrValue1);
    Result<BoolSetterOnDestr, std::string> r5a{f5};
    bool called5{false};
    Result<BoolSetterOnDestr, CopyOnly<std::string>> r5b{&called5};
    EXPECT_TRUE(r5a.hasError());
    EXPECT_TRUE(r5b.hasValue());
    r5b = r5a;
    EXPECT_TRUE(r5b.hasError());
    EXPECT_EQ(r5b, f5);
    EXPECT_TRUE(called5);

    Result<std::string, BoolSetterOnDestr> r6a{cstrValue1};
    bool called6{false};
    Result<CopyOnly<std::string>, BoolSetterOnDestr> r6b{fail(&called6)};
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasError());
    r6b = r6a;
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_EQ(*r6b, strValue1);
    EXPECT_TRUE(called6);
}

TEST_F(ResultTests, TEConversionMoveAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Result<MoveOnly<std::string>, int>,
                    Result<std::string, int>&&>;
    EXPECT_TRUE(AssignableC1);

    Result<std::string, int> r1a{cstrValue1};
    Result<MoveOnly<std::string>, int> r1b;
    r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto AssignableC2 =
            std::is_assignable_v<
                    Result<int, MoveOnly<std::string>>,
                    Result<int, std::string>&&>;
    EXPECT_TRUE(AssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<int, std::string> r2a{f2};
    Result<int, MoveOnly<std::string>> r2b{InPlaceError};
    r2b = std::move(r2a);
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a.error(), ""s);
    EXPECT_EQ(r2b.error(), f2.error());

    constexpr auto AssignableC3 =
            std::is_assignable_v<
                    Result<MoveOnly<std::string>, MoveOnly<std::string>>,
                    Result<std::string, std::string>&&>;
    EXPECT_TRUE(AssignableC3);

    Result<std::string, std::string> r3a{cstrValue1};
    Result<MoveOnly<std::string>, MoveOnly<std::string>> r3b;
    r3b = std::move(r3a);
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3a, ""s);
    EXPECT_EQ(*r3b, strValue1);

    auto const f4 = fail(cstrValue1);
    Result<std::string, std::string> r4a{f4};
    Result<MoveOnly<std::string>, MoveOnly<std::string>> r4b;
    EXPECT_TRUE(r4b.hasValue());
    r4b = std::move(r4a);
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a.error(), ""s);
    EXPECT_EQ(r4b.error(), f4.error());

    auto const f5 = fail(cstrValue1);
    Result<BoolSetterOnDestr, std::string> r5a{f5};
    bool called5{false};
    Result<BoolSetterOnDestr, MoveOnly<std::string>> r5b{&called5};
    EXPECT_TRUE(r5a.hasError());
    EXPECT_TRUE(r5b.hasValue());
    r5b = std::move(r5a);
    EXPECT_TRUE(r5b.hasError());
    EXPECT_EQ(r5a.error(), ""s);
    EXPECT_EQ(r5b.error(), f5.error());
    EXPECT_TRUE(called5);

    Result<std::string, BoolSetterOnDestr> r6a{cstrValue1};
    bool called6{false};
    Result<MoveOnly<std::string>, BoolSetterOnDestr> r6b{fail(&called6)};
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasError());
    r6b = std::move(r6a);
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_EQ(*r6a, ""s);
    EXPECT_EQ(*r6b, strValue1);
    EXPECT_TRUE(called6);
}

TEST_F(ResultTests, TEValueAndFailureAssignabilityProperties) {
    std::string const s1{cstrValue1};
    Result<std::string, int> r1{};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, ""s);
    r1 = s1;
    EXPECT_EQ(*r1, strValue1);

    auto const f2 = fail<std::string>(cstrValue1);
    bool called2{false};
    Result<BoolSetterOnDestr, std::string> r2{&called2};
    EXPECT_TRUE(r2.hasValue());
    r2 = f2;
    EXPECT_TRUE(r2.hasError());
    EXPECT_EQ(r2, f2);
    EXPECT_TRUE(called2);

    std::string const s3{cstrValue1};
    bool called3{false};
    Result<std::string, BoolSetterOnDestr> r3{fail(&called3)};
    EXPECT_TRUE(r3.hasError());
    r3 = s3;
    EXPECT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, s3);
    EXPECT_TRUE(called3);

    auto const f4 = fail<std::string>(cstrValue1);
    Result<int, std::string> r4{InPlaceError};
    EXPECT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), ""s);
    r4 = f4;
    EXPECT_TRUE(r4.hasError());
    EXPECT_EQ(r4, f4);

    std::string s5{cstrValue1};
    Result<MoveOnly<std::string>, int> r5{};
    EXPECT_TRUE(r5.hasValue());
    EXPECT_EQ(*r5, ""s);
    r5 = std::move(s5);
    EXPECT_EQ(*r5, strValue1);
    EXPECT_TRUE(s5.empty());

    auto f6 = fail<std::string>(cstrValue1);
    bool called6{false};
    Result<BoolSetterOnDestr, MoveOnly<std::string>> r6{&called6};
    EXPECT_TRUE(r6.hasValue());
    r6 = std::move(f6);
    EXPECT_TRUE(r6.hasError());
    EXPECT_EQ(r6.error(), strValue1);
    EXPECT_TRUE(called6);
    EXPECT_TRUE(f6.error().empty());

    std::string s7{cstrValue1};
    bool called7{false};
    Result<MoveOnly<std::string>, BoolSetterOnDestr> r7{fail(&called7)};
    EXPECT_TRUE(r7.hasError());
    r7 = std::move(s7);
    EXPECT_TRUE(r7.hasValue());
    EXPECT_EQ(*r7, strValue1);
    EXPECT_TRUE(called7);
    EXPECT_TRUE(s7.empty());

    auto f8 = fail<std::string>(cstrValue1);
    Result<int, MoveOnly<std::string>> r8{InPlaceError};
    EXPECT_TRUE(r8.hasError());
    EXPECT_EQ(r8.error(), ""s);
    r8 = std::move(f8);
    EXPECT_TRUE(r8.hasError());
    EXPECT_EQ(r8.error(), strValue1);
    EXPECT_TRUE(f8.error().empty());
}

TEST_F(ResultTests, TEArrowOperatorProperties) {
    auto r1 = Result<int, int>{25519};
    EXPECT_EQ(&*r1, r1.operator->());
    constexpr auto SameTypeC1 = std::is_same_v<decltype(r1.operator->()), int*>;
    EXPECT_TRUE(SameTypeC1);

    auto const r2 = Result<int, int>{25519};
    EXPECT_EQ(&*r2, r2.operator->());
    constexpr auto SameTypeC2 = std::is_same_v<decltype(r2.operator->()), int const*>;
    EXPECT_TRUE(SameTypeC2);

    auto r3 = Result<std::string, int>{"12345"};
    EXPECT_EQ(r3->size(), 5ul);
}

TEST_F(ResultTests, TEStarOperatorProperties) {
    auto r1 = Result<int, int>{25519};
    constexpr auto SameTypeC1 = std::is_same_v<decltype(*r1), int&>;
    EXPECT_TRUE(SameTypeC1);
    *r1 = 1;
    EXPECT_EQ(r1.value(), 1);

    auto const r2 = Result<int, int>{25519};
    constexpr auto SameTypeC2 = std::is_same_v<decltype(*r2), int const&>;
    EXPECT_TRUE(SameTypeC2);
    EXPECT_EQ(*r2, 25519);

    auto r3 = Result<int, int>{25519};
    constexpr auto SameTypeC3 = std::is_same_v<decltype(*std::move(r3)), int&&>;
    EXPECT_TRUE(SameTypeC3);
    auto&& x3 = std::move(r3).operator*();
    EXPECT_EQ(x3, 25519);

    auto const r4 = Result<int, int>{25519};
    constexpr auto SameTypeC4 = std::is_same_v<decltype(*std::move(r4)), int const&&>;
    EXPECT_TRUE(SameTypeC4);
    auto const&& x4 = std::move(r4).operator*();
    EXPECT_EQ(x4, 25519);
}

TEST_F(ResultTests, TEOperatorBoolProperties) {
    auto r1 = Result<int, int>{};
    EXPECT_TRUE(static_cast<bool>(r1));

    auto r2 = Result<int, int>{fail(25519)};
    EXPECT_FALSE(static_cast<bool>(r2));
}

TEST_F(ResultTests, TEHasValueHasErrorProperties) {
    auto r1 = Result<int, int>{};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_FALSE(r1.hasError());

    auto r2 = Result<int, int>{fail(25519)};
    EXPECT_FALSE(r2.hasValue());
    EXPECT_TRUE(r2.hasError());
}

TEST_F(ResultTests, TEValueAccessorProperties) {
    auto r1 = Result<int, int>{25519};
    constexpr auto SameTypeC1 = std::is_same_v<decltype(r1.value()), int&>;
    EXPECT_TRUE(SameTypeC1);
    r1.value() = 1;
    EXPECT_EQ(r1.value(), 1);

    auto const r2 = Result<int, int>{25519};
    constexpr auto SameTypeC2 = std::is_same_v<decltype(r2.value()), int const&>;
    EXPECT_TRUE(SameTypeC2);
    EXPECT_EQ(r2.value(), 25519);

    auto r3 = Result<int, int>{25519};
    constexpr auto SameTypeC3 = std::is_same_v<decltype(std::move(r3).value()), int&&>;
    EXPECT_TRUE(SameTypeC3);
    auto&& x3 = std::move(r3).value();
    EXPECT_EQ(x3, 25519);

    auto const r4 = Result<int, int>{25519};
    constexpr auto SameTypeC4 = std::is_same_v<decltype(std::move(r4).value()), int const&&>;
    EXPECT_TRUE(SameTypeC4);
    auto const&& x4 = std::move(r4).value();
    EXPECT_EQ(x4, 25519);
}

TEST_F(ResultTests, TEErrorAccessorProperties) {
    auto r1 = Result<int, int>{fail(25519)};
    constexpr auto SameTypeC1 = std::is_same_v<decltype(r1.error()), int&>;
    EXPECT_TRUE(SameTypeC1);
    r1.error() = 1;
    EXPECT_EQ(r1.error(), 1);

    auto const r2 = Result<int, int>{fail(25519)};
    constexpr auto SameTypeC2 = std::is_same_v<decltype(r2.error()), int const&>;
    EXPECT_TRUE(SameTypeC2);
    EXPECT_EQ(r2.error(), 25519);

    auto r3 = Result<int, int>{fail(25519)};
    constexpr auto SameTypeC3 = std::is_same_v<decltype(std::move(r3).error()), int&&>;
    EXPECT_TRUE(SameTypeC3);
    auto&& x3 = std::move(r3).error();
    EXPECT_EQ(x3, 25519);

    auto const r4 = Result<int, int>{fail(25519)};
    constexpr auto SameTypeC4 = std::is_same_v<decltype(std::move(r4).error()), int const&&>;
    EXPECT_TRUE(SameTypeC4);
    auto const&& x4 = std::move(r4).error();
    EXPECT_EQ(x4, 25519);
}

TEST_F(ResultTests, TEValueOrAccessProperties) {
    int const v1alt{25519};
    Result<int, std::string> r1{1};
    auto const v1out = r1.valueOr(v1alt);
    EXPECT_EQ(v1out, 1);

    int const v2alt{25519};
    Result<int, std::string> r2{fail("error")};
    auto const v2out = r2.valueOr((v2alt));
    EXPECT_EQ(v2out, v2alt);

    Result<MoveOnly<std::string>, int> r3{cstrValue1};
    auto const v3out = std::move(r3).valueOr("substitute");
    EXPECT_EQ(v3out, strValue1);
    EXPECT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, ""s);

    Result<MoveOnly<std::string>, int> r4{fail(25519)};
    auto const v4out = std::move(r4).valueOr("substitute");
    EXPECT_EQ(v4out, "substitute"s);
    EXPECT_TRUE(r4.hasError());
}

TEST_F(ResultTests, TEErrorOrAccessProperties) {
    int const e1alt{25519};
    Result<std::string, int> r1{fail(1)};
    auto const e1out = r1.errorOr(e1alt);
    EXPECT_EQ(e1out, 1);

    int const e2alt{25519};
    Result<std::string, int> r2{strValue1};
    auto const e2out = r2.errorOr(e2alt);
    EXPECT_EQ(e2out, e2alt);

    Result<int, MoveOnly<std::string>> r3{fail(cstrValue1)};
    auto const e3out = std::move(r3).errorOr("substitute");
    EXPECT_EQ(e3out, strValue1);
    EXPECT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), ""s);

    Result<int, MoveOnly<std::string>> r4{25519};
    auto const v4out = std::move(r4).errorOr("substitute");
    EXPECT_EQ(v4out, "substitute"s);
    EXPECT_TRUE(r4.hasValue());
}

TEST_F(ResultTests, TEFlatMapProperties) {
    Result<int, int> r1a{25519};
    auto const r1b = r1a.flatMap([] (auto x) {
        return Result<std::string, int>{fmt::format("ed{}", x)};
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, "ed25519"s);

    Result<int, int> r2a{fail(25519)};
    auto const r2b = r2a.flatMap([] (auto x) {
        return Result<std::string, int>{fmt::format("ed{}", x)};
    });
    EXPECT_FALSE(r2b.hasValue());
    EXPECT_EQ(r2b.error(), 25519);

    Result<int, MoveOnly<std::string>> r3a{25519};
    auto const r3b = std::move(r3a).flatMap([] (auto x) {
        return Result<std::string, MoveOnly<std::string>>{fmt::format("ed{}", x)};
    });
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3b, "ed25519"s);

    Result<int, MoveOnly<std::string>> r4a{fail(cstrValue1)};
    auto const r4b = std::move(r4a).flatMap([] (auto x) {
        return Result<std::string, MoveOnly<std::string>>{fmt::format("ed{}", x)};
    });
    EXPECT_FALSE(r4b.hasValue());
    EXPECT_EQ(r4b.error(), strValue1);
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4a.error().empty());
}

TEST_F(ResultTests, TEMapProperties) {
    int const v1{25519};
    Result<int, int> r1a{v1};
    auto const r1b = r1a.map([] (auto x) {
        return fmt::format("ed{}", x);
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, "ed25519"s);

    int const v2{25519};
    Result<int, int> r2a{v2};
    auto const r2b = r1a.map([] (auto) {});
    EXPECT_TRUE(r2b.hasValue());
    constexpr auto TypeResultVoidIntConstC2 =
            std::is_same_v<decltype(r2b), Result<void, int> const>;
    EXPECT_TRUE(TypeResultVoidIntConstC2);

    auto const f3 = fail(25519);
    Result<int, int> r3a{f3};
    auto const r3b = r3a.map([] (auto x) {
        return fmt::format("ed{}", x);
    });
    EXPECT_FALSE(r3b.hasValue());
    constexpr auto TypeResultStringIntConstC3 =
            std::is_same_v<decltype(r3b), Result<std::string, int> const>;
    EXPECT_TRUE(TypeResultStringIntConstC3);
    EXPECT_EQ(r3b, f3);

    auto const f4 = fail(25519);
    Result<int, int> r4a{f4};
    auto const r4b = r4a.map([] (auto) {});
    EXPECT_FALSE(r4b.hasValue());
    constexpr auto TypeResultVoidIntConstC4 =
            std::is_same_v<decltype(r4b), Result<void, int> const>;
    EXPECT_TRUE(TypeResultVoidIntConstC4);

    auto const v5{25519};
    Result<int, MoveOnly<std::string>> r5a{v5};
    auto const r5b = std::move(r5a).map([] (auto x) {
        return fmt::format("ed{}", x);
    });
    EXPECT_TRUE(r5a.hasValue());
    EXPECT_TRUE(r5b.hasValue());
    constexpr auto TypeResultStringMoveOnlyStringConstC5 =
            std::is_same_v<decltype(r5b), Result<std::string, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultStringMoveOnlyStringConstC5);
    EXPECT_EQ(*r5b, "ed25519"s);

    auto const v6{25519};
    Result<int, MoveOnly<std::string>> r6a{v6};
    auto const r6b = std::move(r6a).map([] (auto) {});
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasValue());
    constexpr auto TypeResultVoidMoveOnlyStringConstC6 =
            std::is_same_v<decltype(r6b), Result<void, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultVoidMoveOnlyStringConstC6);

    auto const f7 = fail(cstrValue1);
    Result<int, MoveOnly<std::string>> r7a{f7};
    auto const r7b = std::move(r7a).map([] (auto x) {
        return fmt::format("ed{}", x);
    });
    EXPECT_FALSE(r7b.hasValue());
    EXPECT_TRUE(r7a.hasError());
    constexpr auto TypeResultStringMoveOnlyStringConstC7 =
            std::is_same_v<decltype(r7b), Result<std::string, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultStringMoveOnlyStringConstC7);
    EXPECT_EQ(r7b.error(), f7.error());
    EXPECT_EQ(r7a.error(), ""s);

    auto const f8 = fail(cstrValue1);
    Result<int, MoveOnly<std::string>> r8a{f8};
    auto const r8b = std::move(r8a).map([] (auto) {});
    EXPECT_FALSE(r8b.hasValue());
    constexpr auto TypeResultVoidMoveOnlyStringConstC8 =
            std::is_same_v<decltype(r8b), Result<void, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultVoidMoveOnlyStringConstC8);
    EXPECT_EQ(r8b.error(), strValue1);
}

TEST_F(ResultTests, TEFlatMapErrorProperties) {
    int const v1{25519};
    Result<int, int> const r1a{v1};
    auto const r1b = r1a.flatMapError([] (int const& ec) {
        return Result<int, std::string>{InPlaceError, fmt::format("error code {}", ec)};
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, 25519);

    auto const f2 = fail(25519);
    Result<int, int> const r2a{f2};
    auto const r2b = r2a.flatMapError([] (auto ec) {
        return Result<int, std::string>{InPlaceError, fmt::format("error code {}", ec)};
    });
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2b.error(), "error code 25519"s);

    Result<MoveOnly<std::string>, MoveOnly<std::string>> r3a{cstrValue1};
    auto const r3b = std::move(r3a).flatMapError([] (MoveOnly<std::string>&& x) {
        return Result<std::string, std::string>{InPlaceError, std::move(x)};
    });
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3b, strValue1);

    Result<MoveOnly<std::string>, MoveOnly<std::string>> r4a{fail(cstrValue1)};
    auto const r4b = std::move(r4a).flatMapError([] (MoveOnly<std::string>&& x) {
        return Result<std::string, size_t>{InPlaceError, std::move(x).size()};
    });
    EXPECT_FALSE(r4b.hasValue());
    EXPECT_EQ(r4b.error(), strValue1.size());
}

TEST_F(ResultTests, TEMapErrorProperties) {
    Result<int, int> r1a{25519};
    auto const r1b = r1a.mapError([] (auto x) { return fmt::format("ed{}", x); });
    EXPECT_TRUE(r1b.hasValue());
    constexpr auto TypeResultIntStringConstC1 =
            std::is_same_v<decltype(r1b), Result<int, std::string> const>;
    EXPECT_TRUE(TypeResultIntStringConstC1);
    EXPECT_EQ(*r1b, 25519);

    Result<int, int> r2a{fail(25519)};
    auto const r2b = r2a.mapError([] (auto x) { return fmt::format("ed{}", x); });
    EXPECT_FALSE(r2b.hasValue());
    constexpr auto TypeResultIntStringConstC2 =
            std::is_same_v<decltype(r1b), Result<int, std::string> const>;
    EXPECT_TRUE(TypeResultIntStringConstC2);
    EXPECT_EQ(r2b.error(), "ed25519"s);

    Result<MoveOnly<std::string>, int> r3a{cstrValue1};
    auto const r3b = std::move(r3a).mapError(
            [] (auto x) { return fmt::format("EC{}", x); });
    EXPECT_TRUE(r3b.hasValue());
    constexpr auto TypeResultMoveOnlyStringStringConstC3 =
            std::is_same_v<decltype(r3b), Result<MoveOnly<std::string>, std::string> const>;
    EXPECT_TRUE(TypeResultMoveOnlyStringStringConstC3);
    EXPECT_EQ(*r3b, strValue1);

    Result<MoveOnly<std::string>, int> r4a{fail(25519)};
    auto const r4b = std::move(r4a).mapError([] (auto x) { return fmt::format("ed{}", x); });
    EXPECT_FALSE(r4b.hasValue());
    constexpr auto TypeResultMoveOnlyStringStringConstC4 =
            std::is_same_v<decltype(r4b), Result<MoveOnly<std::string>, std::string> const>;
    EXPECT_TRUE(TypeResultMoveOnlyStringStringConstC4);
    EXPECT_EQ(r4b.error(), "ed25519"s);
}

TEST_F(ResultTests, TEValueErrorReferencesConstructionProperties) {
    constexpr auto NotDefaultConstructibleC1 =
            not std::is_default_constructible_v<Result<int&, int>>;
    EXPECT_TRUE(NotDefaultConstructibleC1);

    /* Default construction initializes the Result object in the value state */
    constexpr auto DefaultConstructibleC2 =
            std::is_default_constructible_v<Result<int, int&>>;
    EXPECT_TRUE(DefaultConstructibleC2);

    constexpr auto NotDefaultConstructibleC3 =
            not std::is_default_constructible_v<Result<int&, int&>>;
    EXPECT_TRUE(NotDefaultConstructibleC3);

    int v1{1};
    Result<int&, int> r1a{v1};
    auto r1b = r1a;
    EXPECT_EQ(&(*r1b), &v1);
    v1 = 25519;
    EXPECT_EQ(*r1a, 25519);
    EXPECT_EQ(*r1b, 25519);

    int v2{2};
    Result<int, int&> r2a{InPlaceError, v2};
    auto r2b = r2a;
    EXPECT_EQ(&(r2b.error()), &v2);
    EXPECT_EQ(r2b.error(), 2);
    EXPECT_EQ(r2a.error(), 2);
    v2 = 25519;
    EXPECT_EQ(r2b.error(), 25519);
    EXPECT_EQ(r2a.error(), 25519);

    constexpr auto CopyConstructibleC4 =
            std::is_copy_constructible_v<Result<int&, int>>;
    EXPECT_TRUE(CopyConstructibleC4);
    constexpr auto CopyConstructibleC5 =
            std::is_copy_constructible_v<Result<int, int&>>;
    EXPECT_TRUE(CopyConstructibleC5);
    constexpr auto CopyConstructibleC6 =
            std::is_copy_constructible_v<Result<int&, int&>>;
    EXPECT_TRUE(CopyConstructibleC6);
    constexpr auto MoveConstructibleC7 =
            std::is_move_constructible_v<Result<int&, MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveConstructibleC7);
    constexpr auto MoveConstructibleC8 =
            std::is_move_constructible_v<Result<MoveOnly<std::string>, int&>>;
    EXPECT_TRUE(MoveConstructibleC8);

    constexpr auto NotConstructibleFromC9 =
            not std::is_constructible_v<Result<int&, int>, Result<int, int> const&>;
    EXPECT_TRUE(NotConstructibleFromC9);

    constexpr auto ContructibleFromC10 =
            std::is_constructible_v<Result<VValue&, int>, Result<CValue&, int> const&>;
    EXPECT_TRUE(ContructibleFromC10);

    int v3{12345};
    auto cval3 = CValue{v3};
    Result<CValue&, int> r3a{cval3};
    Result<VValue&, int> r3b{r3a};
    EXPECT_EQ(&(*r3b), &cval3);
    EXPECT_EQ(r3b->getValue(), v3);

    int v4{54321};
    auto cval4 = CValue{v4};
    Result<int, CValue&> r4a{InPlaceError, cval4};
    Result<int, VValue&> r4b{r4a};
    EXPECT_EQ(&(r4b.error()), &cval4);
    EXPECT_EQ(r4b.error().getValue(), v4);

    constexpr auto NotConstructibleFromC11 =
            not std::is_constructible_v<
                    Result<int&, MoveOnly<std::string>>,
                    Result<int, MoveOnly<std::string>>>;
    EXPECT_TRUE(NotConstructibleFromC11);

    constexpr auto NotConstructibleFromC12 =
            not std::is_constructible_v<
                    Result<MoveOnly<std::string>, int&>,
                    Result<MoveOnly<std::string>, int>>;
    EXPECT_TRUE(NotConstructibleFromC12);

    constexpr auto ConstructibleFromC13 =
            std::is_constructible_v<
                    Result<VValue&, MoveOnly<std::string>>,
                    Result<CValue&, MoveOnly<std::string>>>;
    EXPECT_TRUE(ConstructibleFromC13);
    constexpr auto ConstructibleFromC14 =
            std::is_constructible_v<
                    Result<MoveOnly<std::string>, VValue&>,
                    Result<MoveOnly<std::string>, CValue&>>;
    EXPECT_TRUE(ConstructibleFromC14);

    int const v5{123};
    auto cval5 = CValue{v5};
    auto r5a = Result<CValue&, MoveOnly<std::string>>{cval5};
    auto const r5b = Result<VValue&, MoveOnly<std::string>>{std::move(r5a)};
    EXPECT_EQ(&(*r5b), &cval5);
    EXPECT_EQ(r5b->getValue(), v5);

    int const v6{666};
    auto cval6 = CValue{v6};
    auto r6a = Result<MoveOnly<std::string>, CValue&>{InPlaceError, cval6};
    auto const r6b = Result<MoveOnly<std::string>, VValue&>{std::move(r6a)};
    EXPECT_EQ(&(r6b.error()), &cval6);
    EXPECT_EQ(r6b.error().getValue(), v6);

    auto cval7 = CValue{25519};
    Result<VValue&, int> r7{InPlaceValue, cval7};
    EXPECT_TRUE(r7.hasValue());
    EXPECT_EQ(&(*r7), &cval7);
    EXPECT_EQ(r7->getValue(), 25519);

    auto cval8 = CValue{25519};
    Result<int, VValue&> r8{InPlaceError, cval8};
    EXPECT_FALSE(r8.hasValue());
    EXPECT_EQ(&(r8.error()), &cval8);
    EXPECT_EQ(r8.error().getValue(), 25519);

    Result<std::string, int&> r9{strValue1};
    EXPECT_TRUE(r9.hasValue());
    EXPECT_EQ(*r9, strValue1);

    auto const f10 = fail<std::string>(cstrValue1);
    Result<int&, std::string> r10{f10};
    EXPECT_TRUE(r10.hasError());
    EXPECT_EQ(r10, f10);

    auto v11 = strValue1;
    Result<MoveOnly<std::string>, int&> r11{std::move(v11)};
    EXPECT_TRUE(r11.hasValue());
    EXPECT_EQ(*r11, strValue1);
    EXPECT_TRUE(v11.empty());

    auto f12 = fail<std::string>(cstrValue1);
    Result<int&, MoveOnly<std::string>> r12{std::move(f12)};
    EXPECT_TRUE(r12.hasError());
    EXPECT_EQ(r12.error(), strValue1);
    EXPECT_TRUE(f12.error().empty());
}

TEST_F(ResultTests, TEValueErrorReferencesAssignabilityProperties) {
    auto v1a = CValue{1};
    auto v1b = CValue{25519};
    auto const r1a = Result<VValue&, int>{v1a};
    auto r1b = Result<VValue&, int>{v1b};
    EXPECT_EQ(&(*r1b), &v1b);
    EXPECT_EQ(r1b->getValue(), 25519);
    r1b = r1a;
    EXPECT_EQ(&(*r1b), &v1a);
    EXPECT_EQ(r1b->getValue(), 1);

    auto v2a = CValue{1};
    auto v2b = CValue{25519};
    auto const r2a = Result<int, VValue&>{InPlaceError, v2a};
    auto r2b = Result<int, VValue&>{InPlaceError, v2b};
    EXPECT_EQ(&(r2b.error()), &v2b);
    EXPECT_EQ(r2b.error().getValue(), 25519);
    r2b = r2a;
    EXPECT_EQ(&(r2b.error()), &v2a);
    EXPECT_EQ(r2b.error().getValue(), 1);

    auto v3 = CValue{25519};
    Result<VValue&, int> r3a{v3};
    Result<VValue&, int> r3b{fail(1)};
    EXPECT_FALSE(r3b.hasValue());
    r3b = r3a;
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(&(*r3b), &v3);
    EXPECT_EQ(r3b->getValue(), 25519);

    auto v4 = CValue{25519};
    Result<int, VValue&> r4a{InPlaceError, v4};
    Result<int, VValue&> r4b{1};
    EXPECT_TRUE(r4b.hasValue());
    EXPECT_EQ(*r4b, 1);
    r4b = r4a;
    EXPECT_FALSE(r4b.hasValue());
    EXPECT_EQ(&(r4b.error()), &v4);
    EXPECT_EQ(r4b.error().getValue(), 25519);

    auto v5a = CValue{1};
    auto v5b = CValue{25519};
    Result<VValue&, MoveOnly<std::string>> r5a{v5a};
    Result<VValue&, MoveOnly<std::string>> r5b{v5b};
    EXPECT_EQ(&(*r5b), &v5b);
    EXPECT_EQ(r5b->getValue(), 25519);
    r5b = std::move(r5a);
    EXPECT_EQ(&(*r5b), &v5a);
    EXPECT_EQ(r5b->getValue(), 1);

    auto v6a = CValue{2};
    auto v6b = CValue{12345};
    Result<MoveOnly<std::string>, VValue&> r6a{InPlaceError, v6a};
    Result<MoveOnly<std::string>, VValue&> r6b{InPlaceError, v6b};
    EXPECT_TRUE(r6b.hasError());
    EXPECT_EQ(&(r6b.error()), &v6b);
    EXPECT_EQ(r6b.error().getValue(), 12345);
    r6b = std::move(r6a);
    EXPECT_EQ(&(r6b.error()), &v6a);
    EXPECT_EQ(r6b.error().getValue(), 2);

    auto v7 = CValue{15};
    Result<VValue&, MoveOnly<std::string>> r7a{v7};
    Result<VValue&, MoveOnly<std::string>> r7b{fail(cstrValue1)};
    EXPECT_FALSE(r7b.hasValue());
    EXPECT_EQ(r7b.error(), strValue1);
    r7b = std::move(r7a);
    EXPECT_TRUE(r7b.hasValue());
    EXPECT_EQ(&(*r7b), &v7);
    EXPECT_EQ(r7b->getValue(), 15);

    auto v8 = CValue{777};
    Result<MoveOnly<std::string>, VValue&> r8a{InPlaceError, v8};
    Result<MoveOnly<std::string>, VValue&> r8b{cstrValue1};
    EXPECT_TRUE(r8b.hasValue());
    EXPECT_EQ(*r8b, strValue1);
    r8b = std::move(r8a);
    EXPECT_FALSE(r8b.hasValue());
    EXPECT_EQ(&(r8b.error()), &v8);
    EXPECT_EQ(r8b.error().getValue(), 777);

    auto v9a = CValue{999};
    auto v9b = CValue{1};
    Result<CValue&, int> r9a{v9a};
    Result<VValue&, int> r9b{v9b};
    EXPECT_TRUE(r9b.hasValue());
    EXPECT_EQ(&(*r9b), &v9b);
    EXPECT_EQ(r9b->getValue(), 1);
    r9b = r9a;
    EXPECT_EQ(&(*r9b), &v9a);
    EXPECT_EQ(r9b->getValue(), 999);

    auto v10a = CValue{55};
    auto v10b = CValue{0};
    Result<int, CValue&> r10a{InPlaceError, v10a};
    Result<int, VValue&> r10b{InPlaceError, v10b};
    EXPECT_FALSE(r10b.hasValue());
    EXPECT_EQ(&(r10b.error()), &v10b);
    EXPECT_EQ(r10b.error().getValue(), 0);
    r10b = r10a;
    EXPECT_EQ(&(r10b.error()), &v10a);
    EXPECT_EQ(r10b.error().getValue(), 55);

    auto v11 = CValue{25519};
    Result<CValue&, int> r11a{v11};
    Result<VValue&, int> r11b{fail(1)};
    EXPECT_FALSE(r11b.hasValue());
    r11b = r11a;
    EXPECT_TRUE(r11b.hasValue());
    EXPECT_EQ(&(*r11b), &v11);
    EXPECT_EQ(r11b->getValue(), 25519);

    auto v12 = CValue{25519};
    Result<int, CValue&> r12a{InPlaceError, v12};
    Result<int, VValue&> r12b{1};
    EXPECT_TRUE(r12b.hasValue());
    EXPECT_EQ(*r12b, 1);
    r12b = r12a;
    EXPECT_FALSE(r12b.hasValue());
    EXPECT_EQ(&(r12b.error()), &v12);
    EXPECT_EQ(r12b.error().getValue(), 25519);

    auto v13a = CValue{1};
    auto v13b = CValue{25519};
    Result<CValue&, MoveOnly<std::string>> r13a{v13a};
    Result<VValue&, MoveOnly<std::string>> r13b{v13b};
    EXPECT_EQ(&(*r13b), &v13b);
    EXPECT_EQ(r13b->getValue(), 25519);
    r13b = std::move(r13a);
    EXPECT_EQ(&(*r13b), &v13a);
    EXPECT_EQ(r13b->getValue(), 1);

    auto v14a = CValue{2};
    auto v14b = CValue{12345};
    Result<MoveOnly<std::string>, CValue&> r14a{InPlaceError, v14a};
    Result<MoveOnly<std::string>, VValue&> r14b{InPlaceError, v14b};
    EXPECT_TRUE(r14b.hasError());
    EXPECT_EQ(&(r14b.error()), &v14b);
    EXPECT_EQ(r14b.error().getValue(), 12345);
    r14b = std::move(r14a);
    EXPECT_EQ(&(r14b.error()), &v14a);
    EXPECT_EQ(r14b.error().getValue(), 2);

    auto v15 = CValue{15};
    Result<CValue&, MoveOnly<std::string>> r15a{v15};
    Result<VValue&, MoveOnly<std::string>> r15b{fail(cstrValue1)};
    EXPECT_FALSE(r15b.hasValue());
    EXPECT_EQ(r15b.error(), strValue1);
    r15b = std::move(r15a);
    EXPECT_TRUE(r15b.hasValue());
    EXPECT_EQ(&(*r15b), &v15);
    EXPECT_EQ(r15b->getValue(), 15);

    auto v16 = CValue{777};
    Result<MoveOnly<std::string>, CValue&> r16a{InPlaceError, v16};
    Result<MoveOnly<std::string>, VValue&> r16b{cstrValue1};
    EXPECT_TRUE(r16b.hasValue());
    EXPECT_EQ(*r16b, strValue1);
    r16b = std::move(r16a);
    EXPECT_FALSE(r16b.hasValue());
    EXPECT_EQ(&(r16b.error()), &v16);
    EXPECT_EQ(r16b.error().getValue(), 777);

    int v17a{25519};
    int v17b{12};
    auto r17 = Result<int&, std::string>{v17a};
    EXPECT_TRUE(r17.hasValue());
    EXPECT_EQ(&(*r17), &v17a);
    EXPECT_EQ(*r17, 25519);
    r17 = v17b;
    EXPECT_EQ(&(*r17), &v17b);
    EXPECT_EQ(*r17, 12);

    int v18a{666};
    int v18b{5};
    Result<std::string, int&> r18{InPlaceError, v18a};
    EXPECT_FALSE(r18.hasValue());
    EXPECT_EQ(&(r18.error()), &v18a);
    EXPECT_EQ(r18.error(), 666);
    r18 = Failure<int&>(v18b);
    EXPECT_EQ(&(r18.error()), &v18b);
    EXPECT_EQ(r18.error(), 5);

    int v19{111};
    Result<int&, int> r19{fail(666)};
    r19 = v19;
    EXPECT_TRUE(r19.hasValue());
    EXPECT_EQ(&(*r19), &v19);
    EXPECT_EQ(*r19, 111);

    int v20{666};
    Result<int, int&> r20{777};
    r20 = Failure<int&>{v20};
    EXPECT_FALSE(r20.hasValue());
    EXPECT_EQ(&(r20.error()), &v20);
    EXPECT_EQ(r20.error(), 666);
}

TEST_F(ResultTests, TEValueErrorReferenceAccessorProperties) {
    int v1{1};
    Result<int&, int> r1{v1};
    constexpr auto IntRefTypeC1 = std::is_same_v<decltype(*r1), int&>;
    EXPECT_TRUE(IntRefTypeC1);
    EXPECT_EQ(&(*r1), &v1);

    int v2{2};
    Result<int&, int> r2{v2};
    constexpr auto IntRefTypeC2 = std::is_same_v<decltype(r2.value()), int&>;
    EXPECT_TRUE(IntRefTypeC2);
    EXPECT_EQ(&(*r2), &v2);

    int v3{3};
    Result<int, int&> r3{InPlaceError, v3};
    constexpr auto IntRefTypeC3 = std::is_same_v<decltype(r3.error()), int&>;
    EXPECT_TRUE(IntRefTypeC3);
    EXPECT_EQ(&(r3.error()), &v3);

    int v4{4};
    Result<int&, int> const r4{v4};
    // *r4 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC4 = std::is_same_v<decltype(*r4), int&>;
    EXPECT_TRUE(IntRefTypeC4);
    EXPECT_EQ(&(*r4), &v4);

    int v5{5};
    Result<int&, int> const r5{v5};
    // *r5 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC5 = std::is_same_v<decltype(r5.value()), int&>;
    EXPECT_TRUE(IntRefTypeC5);
    EXPECT_EQ(&(*r5), &v5);

    int v6{6};
    Result<int, int&> const r6{InPlaceError, v6};
    // r6.error() return lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC6 = std::is_same_v<decltype(r6.error()), int&>;
    EXPECT_TRUE(IntRefTypeC6);
    EXPECT_EQ(&(r6.error()), &v6);

    int v7a{7};
    Result<int&, int> r7{v7a};
    constexpr auto IntRefTypeC7 = std::is_same_v<decltype(*std::move(r7)), int&>;
    EXPECT_TRUE(IntRefTypeC7);
    int& v7b = *std::move(r7);
    EXPECT_EQ(&v7a, &v7b);

    int v8a{8};
    Result<int&, int> r8{v8a};
    constexpr auto IntRefTypeC8 = std::is_same_v<decltype(std::move(r8).value()), int&>;
    EXPECT_TRUE(IntRefTypeC8);
    int& v8b = *std::move(r8);
    EXPECT_EQ(&v8a, &v8b);

    int v9a{9};
    Result<int, int&> const r9{InPlaceError, v9a};
    // r9.error() returns lvalue reference that doesn't propagate constness!
    constexpr auto IntRefTypeC9 = std::is_same_v<decltype(std::move(r9).error()), int&>;
    EXPECT_TRUE(IntRefTypeC9);
    int& v9b = std::move(r9).error();
    EXPECT_EQ(&v9a, &v9b);

    int v10a{10};
    Result<int&, int> const r10{v10a};
    // *r6 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC10 = std::is_same_v<decltype(*std::move(r10)), int&>;
    EXPECT_TRUE(IntRefTypeC10);
    int& v10b = *std::move(r10);
    EXPECT_EQ(&v10a, &v10b);

    int v11a{11};
    Result<int&, int> const r11{v11a};
    // *r6 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC11 = std::is_same_v<decltype(std::move(r11).value()), int&>;
    EXPECT_TRUE(IntRefTypeC11);
    int& v11b = *std::move(r11);
    EXPECT_EQ(&v11a, &v11b);

    int v12a{12};
    Result<int, int&> const r12{InPlaceError, v12a};
    // No constness is getting propagated here!
    constexpr auto IntRefTypeC12 = std::is_same_v<decltype(std::move(r12).error()), int&>;
    EXPECT_TRUE(IntRefTypeC12);
    int& v12b = std::move(r12).error();
    EXPECT_EQ(&v12a, &v12b);
}

TEST_F(ResultTests, VoidEDefaultConstructionProperties) {
    constexpr auto DefaultConstructibleC1 =
            std::is_default_constructible_v<Result<void, int>>;
    EXPECT_TRUE(DefaultConstructibleC1);

    Result<void, int> r1{};
    EXPECT_TRUE(r1.hasValue());
}

TEST_F(ResultTests, VoidECopyConstructionProperties) {
    constexpr auto TriviallyCopyConstructibleC1 =
            std::is_trivially_copy_constructible_v<Result<void, int>>;
    EXPECT_TRUE(TriviallyCopyConstructibleC1);

    Result<void, int> const r1a;
    auto const r1b = r1a;
    EXPECT_TRUE(r1b.hasValue());

    auto const f2 = fail(25519);
    Result<void, int> const r2a{f2};
    auto const r2b = r2a;
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2b, f2);

    constexpr auto NotTriviallyCopyConstructibleC2 =
            not std::is_trivially_copy_constructible_v<Result<void, std::string>>;
    EXPECT_TRUE(NotTriviallyCopyConstructibleC2);
    constexpr auto CopyConstructibleC3 =
            std::is_copy_constructible_v<Result<void, std::string>>;
    EXPECT_TRUE(CopyConstructibleC3);

    Result<void, std::string> const r3a;
    auto const r3b = r3a;
    EXPECT_TRUE(r3b.hasValue());

    auto const f4 = fail(cstrValue1);
    Result<void, std::string> r4a{f4};
    auto const r4b = r4a;
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4b, f4);

    constexpr auto NotCopyConstructibleC3 =
            not std::is_copy_constructible_v<Result<void, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyConstructibleC3);
}

TEST_F(ResultTests, VoidEMoveConstructionProperties) {
    constexpr auto TriviallyMoveConstructibleC1 =
            std::is_trivially_move_constructible_v<Result<void, int>>;
    EXPECT_TRUE(TriviallyMoveConstructibleC1);

    Result<void, int> r1a{};
    auto const r1b = std::move(r1a);
    EXPECT_TRUE(r1b.hasValue());

    auto const f2 = fail(25519);
    Result<void, int> r2a{f2};
    auto const r2b = std::move(r2a);
    EXPECT_FALSE(r2b.hasValue());
    EXPECT_EQ(r2b, f2);
    EXPECT_EQ(r2b.error(), 25519);

    constexpr auto NotTriviallyMoveConstructibleC2 =
            not std::is_trivially_move_constructible_v<Result<void, MoveOnly<std::string>>>;
    EXPECT_TRUE(NotTriviallyMoveConstructibleC2);
    constexpr auto MoveConstructibleC3 =
            std::is_move_constructible_v<Result<void, MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveConstructibleC3);

    Result<void, MoveOnly<std::string>> r3a{};
    auto const r3b = std::move(r3a);
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());

    auto const f4 = fail(cstrValue1);
    Result<void, MoveOnly<std::string>> r4a{f4};
    auto const r4b = std::move(r4a);
    EXPECT_TRUE(r4a.hasError());
    EXPECT_EQ(r4a.error(), ""s);
    EXPECT_TRUE(r4b.hasError());
    /* FIXME This doesn't compile
    EXPECT_EQ(r4b, f4);
     */
    EXPECT_EQ(r4b.error(), strValue1);

    constexpr auto NotMoveConstructibleC3 =
            not std::is_move_constructible_v<Result<void, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveConstructibleC3);
}

TEST_F(ResultTests, VoidEConversionCopyConstructionProperties) {
    constexpr auto ConstructibleFromC1 =
            std::is_constructible_v<
                    Result<void, int>,
                    Result<void, long> const&>;
    EXPECT_TRUE(ConstructibleFromC1);


    Result<void, long> const r1a{fail(25519l)};
    auto const r1b = Result<void, int>{r1a};
    EXPECT_TRUE(r1b.hasError());
    EXPECT_EQ(r1b.error(), r1a.error());

    constexpr auto NotConstructibleFromC2 =
            not std::is_constructible_v<Result<void, int>, Result<void, std::string>>;
    EXPECT_TRUE(NotConstructibleFromC2);
}

TEST_F(ResultTests, VoidEConversionMoveConstructionProperties) {
    constexpr auto ConstructibleFromC1 =
            std::is_constructible_v<
                    Result<void, std::string>,
                    Result<int, MoveOnly<std::string>>&&>;
    EXPECT_TRUE(ConstructibleFromC1);

    auto const f1 = fail(cstrValue1);
    Result<int, MoveOnly<std::string>> r1a{f1};
    auto const r1b = Result<void, std::string>{std::move(r1a)};
    EXPECT_TRUE(r1a.hasError());
    EXPECT_EQ(r1a.error(), ""s);
    EXPECT_TRUE(r1b.hasError());
    EXPECT_EQ(r1b, f1);

    constexpr auto NotContstructibleFromC2 =
            not std::is_constructible_v<Result<void, int>, Result<void, std::string>&&>;
    EXPECT_TRUE(NotContstructibleFromC2);
}

TEST_F(ResultTests, VoidEInPlaceValueConstructionProperties) {
    Result<void, int> r1{InPlaceValue};
    EXPECT_TRUE(r1.hasValue());
}

TEST_F(ResultTests, VoidEInPlaceErrorConstructionProperties) {
    auto const v1 = std::string{cstrValue1, 34ul};
    Result<void, std::string> const r1{InPlaceError, cstrValue1, 34ul};
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), v1);
}

TEST_F(ResultTests, VoidEInPlaceErrorInitializerListConstructionProperties) {
    auto const v1 = std::string{IL_ED25519, std::allocator<char>{}};
    Result<void, std::string> const r1{InPlaceError, IL_ED25519, std::allocator<char>{}};
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), v1);
}

TEST_F(ResultTests, VoidEFailureConstLValueRefConstructionProperties) {
    auto const f1 = fail<std::string>(cstrValue1);
    Result<void, std::string> const r1{f1};
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1, f1);
}

TEST_F(ResultTests, VoidEFailureRValueRefConstructionProperties) {
    auto f1a = fail<std::string>(cstrValue1);
    auto const f1b = f1a;
    Result<void, MoveOnly<std::string>> const r1{std::move(f1a)};
    EXPECT_EQ(f1a.error(), ""s);
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1, f1b);
}

TEST_F(ResultTests, VoidEDestructionProperties) {
    constexpr auto TriviallyDestructibleC1 =
            std::is_trivially_destructible_v<Result<void, int>>;
    EXPECT_TRUE(TriviallyDestructibleC1);

    constexpr auto NotTriviallyDestructibleC2 =
            not std::is_trivially_destructible_v<Result<void, BoolSetterOnDestr>>;
    EXPECT_TRUE(NotTriviallyDestructibleC2);
    bool called2{false};
    {
        Result<void, BoolSetterOnDestr> const r2{InPlaceError, &called2};
    };
    EXPECT_TRUE(called2);
}

TEST_F(ResultTests, VoidECopyAssignabilityProperties) {
    constexpr auto CopyAssignableC1 =
            std::is_copy_assignable_v<Result<void, std::string>>;
    EXPECT_TRUE(CopyAssignableC1);

    constexpr auto NotCopyAssignableC2 =
            not std::is_copy_assignable_v<Result<void, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyAssignableC2);

    constexpr auto CopyAssignableC3 =
            std::is_copy_assignable_v<Result<void, int>>;
    EXPECT_TRUE(CopyAssignableC3);

    Result<void, int> const r1a{};
    Result<void, int> r1b;
    r1b = r1a;
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());

    constexpr auto CopyAssignableC4 =
            std::is_copy_assignable_v<Result<void, std::string>>;
    EXPECT_TRUE(CopyAssignableC4);

    auto const f2 = fail(cstrValue1);
    Result<void, std::string> const r2a{f2};
    Result<void, std::string> r2b{InPlaceError};
    r2b = r2a;
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a, f2);
    EXPECT_EQ(r2b, f2);

    constexpr auto CopyAssignableC5 =
            std::is_copy_assignable_v<Result<void, std::string>>;
    EXPECT_TRUE(CopyAssignableC5);

    auto const f4 = fail(cstrValue1);
    Result<void, std::string> const r4a{f4};
    EXPECT_TRUE(r4a.hasError());
    Result<void, std::string> r4b;
    EXPECT_TRUE(r4b.hasValue());
    r4b = r4a;
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a, f4);
    EXPECT_EQ(r4b, f4);

    constexpr auto NotCopyAssignableC6 =
            not std::is_copy_assignable_v<Result<void, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyAssignableC6);

    Result<void, BoolSetterOnDestr> r6a{};
    bool called6{false};
    Result<void, BoolSetterOnDestr> r6b{fail(&called6)};
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasError());
    r6b = r6a;
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_TRUE(called6);

    constexpr auto TriviallyCopyAssignableC7 =
            std::is_trivially_copy_assignable_v<Result<void, int>>;
    EXPECT_TRUE(TriviallyCopyAssignableC7);
}

TEST_F(ResultTests, VoidEMoveAssignabilityProperties) {
    constexpr auto MoveAssignableC1 =
            std::is_move_assignable_v<Result<void, int>>;
    EXPECT_TRUE(MoveAssignableC1);

    Result<void, int> r1a{};
    Result<void, int> r1b;
    r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());

    constexpr auto MoveAssignableC2 =
            std::is_move_assignable_v<Result<void, MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveAssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<void, MoveOnly<std::string>> r2a{f2};
    Result<void, MoveOnly<std::string>> r2b{InPlaceError};
    r2b = std::move(r2a);
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a.error(), ""s);
    EXPECT_EQ(r2b.error(), strValue1);

    constexpr auto NotMoveAssignableC3 =
            not std::is_move_assignable_v<Result<void, NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveAssignableC3);

    Result<void, BoolSetterOnDestr> r3a{};
    bool called3{false};
    Result<void, BoolSetterOnDestr> r3b{fail(&called3)};
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasError());
    r3b = std::move(r3a);
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_TRUE(called3);

    constexpr auto TriviallyMoveAssignableC4 =
            std::is_trivially_move_assignable_v<Result<void, int>>;
    EXPECT_TRUE(TriviallyMoveAssignableC4);
}

TEST_F(ResultTests, VoidEThrowingAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Result<void, Throwing<std::string>>,
                    Result<void, std::string> const&>;
    EXPECT_TRUE(AssignableC1);

    constexpr auto NotNothrowAssignableC1 =
            not std::is_nothrow_assignable_v<
                    Result<void, Throwing<std::string>>,
                    Result<void, std::string> const&>;
    EXPECT_TRUE(NotNothrowAssignableC1);

    constexpr auto AssignableC2 =
            std::is_assignable_v<
                    Result<void, Throwing<std::string>>,
                    Result<void, std::string>&&>;
    EXPECT_TRUE(AssignableC2);

    constexpr auto NotNothrowAssignableC2 =
            not std::is_nothrow_assignable_v<
                    Result<void, Throwing<std::string>>,
                    Result<void, std::string>&&>;
    EXPECT_TRUE(NotNothrowAssignableC2);
}

TEST_F(ResultTests, VoidEConversionCopyAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Result<void, std::string>,
                    Result<void, char const*> const&>;
    EXPECT_TRUE(AssignableC1);

    Result<void, char const*> const r1a{fail(cstrValue1)};
    Result<void, std::string> r1b;
    EXPECT_TRUE(r1a.hasError());
    EXPECT_TRUE(r1b.hasValue());
    r1b = r1a;
    EXPECT_FALSE(r1b.hasValue());
    EXPECT_EQ(r1b.error(), strValue1);

    constexpr auto AssignableC2 =
            std::is_assignable_v<
                    Result<void, CopyOnly<std::string>>,
                    Result<void, std::string> const&>;
    EXPECT_TRUE(AssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<void, std::string> const r2a{f2};
    Result<void, CopyOnly<std::string>> r2b{InPlaceError};
    r2b = r2a;
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a, f2);
    EXPECT_EQ(r2b, f2);

    Result<void, std::string> const r3a{};
    Result<void, CopyOnly<std::string>> r3b;
    r3b = r3a;
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());

    auto const f4 = fail(cstrValue1);
    Result<void, std::string> const r4a{f4};
    EXPECT_TRUE(r4a.hasError());
    Result<void, CopyOnly<std::string>> r4b;
    EXPECT_TRUE(r4b.hasValue());
    r4b = r4a;
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a, f4);
    EXPECT_EQ(r4b, f4);

    Result<void, BoolSetterOnDestr> r6a{};
    bool called6{false};
    Result<void, BoolSetterOnDestr> r6b{fail(&called6)};
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasError());
    r6b = r6a;
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_TRUE(called6);
}

TEST_F(ResultTests, VoidEConversionMoveAssignabilityProperties) {
    constexpr auto AssignableFromC1 =
            std::is_assignable_v<
                    Result<void, std::string>,
                    Result<void, char const*>&&>;
    EXPECT_TRUE(AssignableFromC1);

    Result<void, char const*> r1a{fail(cstrValue1)};
    Result<void, std::string> r1b;
    EXPECT_TRUE(r1a.hasError());
    EXPECT_TRUE(r1b.hasValue());
    r1b = std::move(r1a);
    EXPECT_FALSE(r1b.hasValue());
    EXPECT_EQ(r1b.error(), strValue1);

    constexpr auto AssignableC2 =
            std::is_assignable_v<
                    Result<void, MoveOnly<std::string>>,
                    Result<void, std::string>&&>;
    EXPECT_TRUE(AssignableC2);

    auto const f2 = fail(cstrValue1);
    Result<void, std::string> r2a{f2};
    Result<void, MoveOnly<std::string>> r2b{InPlaceError};
    r2b = std::move(r2a);
    EXPECT_TRUE(r2a.hasError());
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2a.error(), ""s);
    EXPECT_EQ(r2b.error(), f2.error());

    Result<void, std::string> r3a{};
    Result<void, MoveOnly<std::string>> r3b;
    r3b = std::move(r3a);
    EXPECT_TRUE(r3a.hasValue());
    EXPECT_TRUE(r3b.hasValue());

    auto const f4 = fail(cstrValue1);
    Result<void, std::string> r4a{f4};
    Result<void, MoveOnly<std::string>> r4b;
    EXPECT_TRUE(r4b.hasValue());
    r4b = std::move(r4a);
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(r4a.error(), ""s);
    EXPECT_EQ(r4b.error(), f4.error());

    Result<void, BoolSetterOnDestr> r5a{};
    bool called5{false};
    Result<void, BoolSetterOnDestr> r5b{fail(&called5)};
    EXPECT_TRUE(r5a.hasValue());
    EXPECT_TRUE(r5b.hasError());
    r5b = std::move(r5a);
    EXPECT_TRUE(r5b.hasValue());
    EXPECT_TRUE(called5);
}

TEST_F(ResultTests, VoidEFailureAssignabilityProperties) {
    auto const f1 = fail<std::string>(cstrValue1);
    Result<void, std::string> r1{};
    EXPECT_TRUE(r1.hasValue());
    r1 = f1;
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1, f1);

    auto const f2 = fail<std::string>(cstrValue1);
    Result<void, std::string> r2{InPlaceError};
    EXPECT_TRUE(r2.hasError());
    EXPECT_EQ(r2.error(), ""s);
    r2 = f2;
    EXPECT_TRUE(r2.hasError());
    EXPECT_EQ(r2, f2);

    auto f3 = fail<std::string>(cstrValue1);
    Result<void, MoveOnly<std::string>> r3{};
    EXPECT_TRUE(r3.hasValue());
    r3 = std::move(f3);
    EXPECT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), strValue1);
    EXPECT_TRUE(f3.error().empty());

    auto f4 = fail<std::string>(cstrValue1);
    Result<void, MoveOnly<std::string>> r4{InPlaceError};
    EXPECT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), ""s);
    r4 = std::move(f4);
    EXPECT_TRUE(r4.hasError());
    EXPECT_EQ(r4.error(), strValue1);
    EXPECT_TRUE(f4.error().empty());
}

TEST_F(ResultTests, VoidEOperatorBoolProperties) {
    auto r1 = Result<void, int>{};
    EXPECT_TRUE(static_cast<bool>(r1));

    auto r2 = Result<void, int>{fail(25519)};
    EXPECT_FALSE(static_cast<bool>(r2));
}

TEST_F(ResultTests, VoidEHasValueHasErrorProperties) {
    auto r1 = Result<void, int>{};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_FALSE(r1.hasError());

    auto r2 = Result<void, int>{fail(25519)};
    EXPECT_FALSE(r2.hasValue());
    EXPECT_TRUE(r2.hasError());
}

TEST_F(ResultTests, VoidEErrorAccessorProperties) {
    auto r1 = Result<void, int>{fail(25519)};
    constexpr auto SameTypeC1 = std::is_same_v<decltype(r1.error()), int&>;
    EXPECT_TRUE(SameTypeC1);
    r1.error() = 1;
    EXPECT_EQ(r1.error(), 1);

    auto const r2 = Result<void, int>{fail(25519)};
    constexpr auto SameTypeC2 = std::is_same_v<decltype(r2.error()), int const&>;
    EXPECT_TRUE(SameTypeC2);
    EXPECT_EQ(r2.error(), 25519);

    auto r3 = Result<void, int>{fail(25519)};
    constexpr auto SameTypeC3 = std::is_same_v<decltype(std::move(r3).error()), int&&>;
    EXPECT_TRUE(SameTypeC3);
    auto&& x3 = std::move(r3).error();
    EXPECT_EQ(x3, 25519);

    auto const r4 = Result<void, int>{fail(25519)};
    constexpr auto SameTypeC4 = std::is_same_v<decltype(std::move(r4).error()), int const&&>;
    EXPECT_TRUE(SameTypeC4);
    auto const&& x4 = std::move(r4).error();
    EXPECT_EQ(x4, 25519);
}

TEST_F(ResultTests, VoidEErrorOrAccessProperties) {
    int const e1alt{25519};
    Result<void, int> r1{fail(1)};
    auto const e1out = r1.errorOr(e1alt);
    EXPECT_EQ(e1out, 1);

    int const e2alt{25519};
    Result<void, int> r2{};
    auto const e2out = r2.errorOr(e2alt);
    EXPECT_EQ(e2out, e2alt);

    Result<void, MoveOnly<std::string>> r3{fail(cstrValue1)};
    auto const e3out = std::move(r3).errorOr("substitute");
    EXPECT_EQ(e3out, strValue1);
    EXPECT_TRUE(r3.hasError());
    EXPECT_EQ(r3.error(), ""s);

    Result<void, MoveOnly<std::string>> r4{};
    auto const v4out = std::move(r4).errorOr("substitute");
    EXPECT_EQ(v4out, "substitute"s);
    EXPECT_TRUE(r4.hasValue());
}

TEST_F(ResultTests, VoidEFlatMapProperties) {
    Result<void, int> r1a{};
    auto const r1b = r1a.flatMap([this] {
        return Result<std::string, int>{strValue1};
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);

    Result<void, int> r2a{fail(25519)};
    auto const r2b = r2a.flatMap([this] {
        return Result<std::string, int>{strValue1};
    });
    EXPECT_FALSE(r2b.hasValue());
    EXPECT_EQ(r2b.error(), 25519);

    Result<void, MoveOnly<std::string>> r3a{};
    auto const r3b = std::move(r3a).flatMap([this] {
        return Result<std::string, MoveOnly<std::string>>{strValue1};
    });
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(*r3b, strValue1);

    Result<void, MoveOnly<std::string>> r4a{fail(cstrValue1)};
    auto const r4b = std::move(r4a).flatMap([this] {
        return Result<std::string, MoveOnly<std::string>>{strValue1};
    });
    EXPECT_FALSE(r4b.hasValue());
    EXPECT_EQ(r4b.error(), strValue1);
    EXPECT_TRUE(r4a.hasError());
    EXPECT_TRUE(r4a.error().empty());
}

TEST_F(ResultTests, VoidEMapProperties) {
    int const v1{25519};
    Result<void, int> r1a;
    auto const r1b = r1a.map([v1]  {
        return fmt::format("ed{}", v1);
    });
    constexpr auto TypeResultVoidIntConstC1 =
            std::is_same_v<decltype(r1b), Result<std::string, int> const>;
    EXPECT_TRUE(TypeResultVoidIntConstC1);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, "ed25519"s);

    Result<void, int> r2a{};
    auto const r2b = r1a.map([] {});
    constexpr auto TypeResultVoidIntConstC2 =
            std::is_same_v<decltype(r2b), Result<void, int> const>;
    EXPECT_TRUE(r2b.hasValue());
    EXPECT_TRUE(TypeResultVoidIntConstC2);

    int const v3{1};
    auto const f3 = fail(25519);
    Result<void, int> r3a{f3};
    auto const r3b = r3a.map([v3] {
        return fmt::format("ed{}", v3);
    });
    EXPECT_FALSE(r3b.hasValue());
    constexpr auto TypeResultStringIntConstC3 =
            std::is_same_v<decltype(r3b), Result<std::string, int> const>;
    EXPECT_TRUE(TypeResultStringIntConstC3);
    EXPECT_EQ(r3b, f3);

    auto const f4 = fail(25519);
    Result<void, int> r4a{f4};
    auto const r4b = r4a.map([] {});
    EXPECT_FALSE(r4b.hasValue());
    constexpr auto TypeResultVoidIntConstC4 =
            std::is_same_v<decltype(r4b), Result<void, int> const>;
    EXPECT_TRUE(TypeResultVoidIntConstC4);

    auto const v5{25519};
    Result<void, MoveOnly<std::string>> r5a{};
    auto const r5b = std::move(r5a).map([v5] {
        return fmt::format("ed{}", v5);
    });
    EXPECT_TRUE(r5a.hasValue());
    EXPECT_TRUE(r5b.hasValue());
    constexpr auto TypeResultStringMoveOnlyStringConstC5 =
            std::is_same_v<decltype(r5b), Result<std::string, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultStringMoveOnlyStringConstC5);
    EXPECT_EQ(*r5b, "ed25519"s);

    Result<void, MoveOnly<std::string>> r6a{};
    auto const r6b = std::move(r6a).map([] {});
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasValue());
    constexpr auto TypeResultVoidMoveOnlyStringConstC6 =
            std::is_same_v<decltype(r6b), Result<void, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultVoidMoveOnlyStringConstC6);

    int const v7{25519};
    auto const f7 = fail(cstrValue1);
    Result<void, MoveOnly<std::string>> r7a{f7};
    auto const r7b = std::move(r7a).map([v7] {
        return fmt::format("ed25519", v7);
    });
    EXPECT_FALSE(r7b.hasValue());
    EXPECT_TRUE(r7a.hasError());
    constexpr auto TypeResultStringMoveOnlyStringConstC7 =
            std::is_same_v<decltype(r7b), Result<std::string, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultStringMoveOnlyStringConstC7);
    EXPECT_EQ(r7b.error(), f7.error());
    EXPECT_EQ(r7a.error(), ""s);

    auto const f8 = fail(cstrValue1);
    Result<void, MoveOnly<std::string>> r8a{f8};
    auto const r8b = std::move(r8a).map([] {});
    EXPECT_FALSE(r8b.hasValue());
    constexpr auto TypeResultVoidMoveOnlyStringConstC8 =
            std::is_same_v<decltype(r8b), Result<void, MoveOnly<std::string>> const>;
    EXPECT_TRUE(TypeResultVoidMoveOnlyStringConstC8);
    EXPECT_EQ(r8b.error(), strValue1);
    EXPECT_TRUE(r8a.error().empty());
}

TEST_F(ResultTests, VoidEFlatMapErrorProperties) {
    Result<void, int> const r1a{};
    auto const r1b = r1a.flatMapError([] (int const& ec) {
        return Result<int, std::string>{InPlaceError, fmt::format("error code {}", ec)};
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, 0);

    auto const f2 = fail(25519);
    Result<void, int> const r2a{f2};
    auto const r2b = r2a.flatMapError([] (auto ec) {
        return Result<void, std::string>{InPlaceError, fmt::format("error code {}", ec)};
    });
    EXPECT_TRUE(r2b.hasError());
    EXPECT_EQ(r2b.error(), "error code 25519"s);

    Result<void, MoveOnly<std::string>> r3a{};
    auto const r3b = std::move(r3a).flatMapError([] (MoveOnly<std::string>&& x) {
        return Result<void, std::string>{InPlaceError, std::move(x)};
    });
    EXPECT_TRUE(r3b.hasValue());

    Result<void, MoveOnly<std::string>> r4a{fail(cstrValue1)};
    auto const r4b = std::move(r4a).flatMapError([] (MoveOnly<std::string>&& x) {
        auto v = std::move(x);
        return Result<void, size_t>{InPlaceError, v.size()};
    });
    EXPECT_FALSE(r4b.hasValue());
    EXPECT_EQ(r4b.error(), strValue1.size());
    EXPECT_TRUE(r4a.error().empty());
}

TEST_F(ResultTests, VoidEMapErrorProperties) {
    Result<void, int> r1a{};
    auto const r1b = r1a.mapError([] (auto x) { return fmt::format("ed{}", x); });
    EXPECT_TRUE(r1b.hasValue());
    constexpr auto TypeResultIntStringConstC1 =
            std::is_same_v<decltype(r1b), Result<void, std::string> const>;
    EXPECT_TRUE(TypeResultIntStringConstC1);

    Result<void, int> r2a{fail(25519)};
    auto const r2b = r2a.mapError([] (auto x) { return fmt::format("ed{}", x); });
    EXPECT_FALSE(r2b.hasValue());
    constexpr auto TypeResultIntStringConstC2 =
            std::is_same_v<decltype(r1b), Result<void, std::string> const>;
    EXPECT_TRUE(TypeResultIntStringConstC2);
    EXPECT_EQ(r2b.error(), "ed25519"s);

    Result<void, int> r3a{};
    auto const r3b = std::move(r3a).mapError(
            [] (auto x) { return fmt::format("EC{}", x); });
    EXPECT_TRUE(r3b.hasValue());
    constexpr auto TypeResultMoveOnlyStringStringConstC3 =
            std::is_same_v<decltype(r3b), Result<void, std::string> const>;
    EXPECT_TRUE(TypeResultMoveOnlyStringStringConstC3);

    Result<void, int> r4a{fail(25519)};
    auto const r4b = std::move(r4a).mapError(
            [] (auto x) { return fmt::format("EC{}", x); });
    EXPECT_FALSE(r4b.hasValue());
    constexpr auto TypeResultMoveOnlyStringStringConstC4 =
            std::is_same_v<decltype(r4b), Result<void, std::string> const>;
    EXPECT_TRUE(TypeResultMoveOnlyStringStringConstC4);
    EXPECT_EQ(r4b.error(), "EC25519"s);
}

TEST_F(ResultTests, VoidEErrorReferencesConstructionProperties) {
    /* Default construction initializes the Result object in the value state */
    constexpr auto DefaultConstructibleC1 =
            std::is_default_constructible_v<Result<void, int&>>;
    EXPECT_TRUE(DefaultConstructibleC1);

    int v1{1};
    Result<void, int&> r1a{InPlaceError, v1};
    auto r1b = r1a;
    EXPECT_EQ(&(r1b.error()), &v1);
    EXPECT_EQ(r1b.error(), 1);
    EXPECT_EQ(r1a.error(), 1);
    v1 = 25519;
    EXPECT_EQ(r1b.error(), 25519);
    EXPECT_EQ(r1b.error(), 25519);

    constexpr auto CopyConstructibleC2 =
            std::is_copy_constructible_v<Result<void, int&>>;
    EXPECT_TRUE(CopyConstructibleC2);

    constexpr auto NotConstructibleFromC3 =
            not std::is_constructible_v<Result<void, int&>, Result<void, int> const&>;
    EXPECT_TRUE(NotConstructibleFromC3);

    constexpr auto ContructibleFromC4 =
            std::is_constructible_v<Result<void, VValue&>, Result<void, CValue&> const&>;
    EXPECT_TRUE(ContructibleFromC4);

    int v2{54321};
    auto cval2 = CValue{v2};
    Result<void, CValue&> r2a{InPlaceError, cval2};
    Result<void, VValue&> r2b{r2a};
    EXPECT_EQ(&(r2b.error()), &cval2);
    EXPECT_EQ(r2b.error().getValue(), v2);
}

TEST_F(ResultTests, VoidEErrorReferencesAssignabilityProperties) {
    auto v1a = CValue{1};
    auto v1b = CValue{25519};
    auto const r1a = Result<void, VValue&>{InPlaceError, v1a};
    auto r1b = Result<void, VValue&>{InPlaceError, v1b};
    EXPECT_EQ(&(r1b.error()), &v1b);
    EXPECT_EQ(r1b.error().getValue(), 25519);
    r1b = r1a;
    EXPECT_EQ(&(r1b.error()), &v1a);
    EXPECT_EQ(r1b.error().getValue(), 1);

    auto v2 = CValue{25519};
    Result<void, VValue&> r2a{InPlaceError, v2};
    Result<void, VValue&> r2b{};
    EXPECT_TRUE(r2b.hasValue());
    r2b = r2a;
    EXPECT_FALSE(r2b.hasValue());
    EXPECT_EQ(&(r2b.error()), &v2);
    EXPECT_EQ(r2b.error().getValue(), 25519);

    auto v3a = CValue{2};
    auto v3b = CValue{12345};
    Result<void, VValue&> r3a{InPlaceError, v3a};
    Result<void, VValue&> r3b{InPlaceError, v3b};
    EXPECT_TRUE(r3b.hasError());
    EXPECT_EQ(&(r3b.error()), &v3b);
    EXPECT_EQ(r3b.error().getValue(), 12345);
    r3b = std::move(r3a);
    EXPECT_EQ(&(r3b.error()), &v3a);
    EXPECT_EQ(r3b.error().getValue(), 2);

    auto v4a = CValue{2};
    auto v4b = CValue{12345};
    Result<void, CValue&> r4a{InPlaceError, v4a};
    Result<void, VValue&> r4b{InPlaceError, v4b};
    EXPECT_TRUE(r4b.hasError());
    EXPECT_EQ(&(r4b.error()), &v4b);
    EXPECT_EQ(r4b.error().getValue(), 12345);
    r4b = std::move(r4a);
    EXPECT_EQ(&(r4b.error()), &v4a);
    EXPECT_EQ(r4b.error().getValue(), 2);

    auto v5 = CValue{777};
    Result<void, VValue&> r5a{InPlaceError, v5};
    Result<void, VValue&> r5b{};
    EXPECT_TRUE(r5b.hasValue());
    r5b = std::move(r5a);
    EXPECT_FALSE(r5b.hasValue());
    EXPECT_EQ(&(r5b.error()), &v5);
    EXPECT_EQ(r5b.error().getValue(), 777);

    int v6a{666};
    int v6b{5};
    Result<void, int&> r6{InPlaceError, v6a};
    EXPECT_FALSE(r6.hasValue());
    EXPECT_EQ(&(r6.error()), &v6a);
    EXPECT_EQ(r6.error(), 666);
    r6 = Failure<int&>(v6b);
    EXPECT_EQ(&(r6.error()), &v6b);
    EXPECT_EQ(r6.error(), 5);

    int v7{666};
    Result<void, int&> r7;
    EXPECT_TRUE(r7.hasValue());
    r7 = Failure<int&>{v7};
    EXPECT_FALSE(r7.hasValue());
    EXPECT_EQ(&(r7.error()), &v7);
    EXPECT_EQ(r7.error(), 666);
}

TEST_F(ResultTests, VoidEErrorReferenceAccessorProperties) {
    int v1{1};
    Result<void, int&> r1{InPlaceError, v1};
    constexpr auto IntRefTypeC1 = std::is_same_v<decltype(r1.error()), int&>;
    EXPECT_TRUE(IntRefTypeC1);
    EXPECT_EQ(&(r1.error()), &v1);

    int v2{2};
    Result<void, int&> const r2{InPlaceError, v2};
    // r2.error() return lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC2 = std::is_same_v<decltype(r2.error()), int&>;
    EXPECT_TRUE(IntRefTypeC2);
    EXPECT_EQ(&(r2.error()), &v2);

    int v3a{3};
    Result<void, int&> const r3{InPlaceError, v3a};
    // r3.error() returns lvalue reference that doesn't propagate constness!
    constexpr auto IntRefTypeC3 = std::is_same_v<decltype(std::move(r3).error()), int&>;
    EXPECT_TRUE(IntRefTypeC3);
    int& v3b = std::move(r3).error();
    EXPECT_EQ(&v3a, &v3b);

    int v4a{4};
    Result<void, int&> const r4{InPlaceError, v4a};
    // No constness is getting propagated here!
    constexpr auto IntRefTypeC4 = std::is_same_v<decltype(std::move(r4).error()), int&>;
    EXPECT_TRUE(IntRefTypeC4);
    int& v4b = std::move(r4).error();
    EXPECT_EQ(&v4a, &v4b);
}

TEST_F(ResultTests, ReferenceMappingProperties) {
    Result<std::string const&, int> r1a{strValue1};
    auto const r1b = r1a.map([] (std::string const& v) { return v.size(); });
    constexpr auto TypeC1 = std::is_same_v<decltype(r1b), Result<size_t, int> const>;
    EXPECT_TRUE(TypeC1);
    EXPECT_EQ(*r1b, strValue1.size());
    auto const r1c = r1a.flatMap([] (std::string const& s) {
        return Result<std::string, long>{fmt::format(">>{}<<", s)};
    });
    auto exp1 = ">>"s + strValue1 + "<<"s;
    EXPECT_EQ(*r1c, exp1);

    int v2{-100};
    Result<std::string const&, int&> r2a{fail(std::ref(v2))};
    auto const r2b = r2a.mapError([] (int& v) {
        auto s = fmt::format("error code {}", v);
        v = 25519;
        return s;
    });
    EXPECT_EQ(r2a.error(), 25519);
    EXPECT_EQ(v2, 25519);
    constexpr auto TypeC2 =
            std::is_same_v<decltype(r2b), Result<std::string const&, std::string> const>;
    EXPECT_TRUE(TypeC2);
    EXPECT_EQ(r2b.error(), "error code -100"s);
    auto const r2c = r2a.flatMapError([] (int& v) {
        XY xy{v, -v};
        v = -100;
        return Result<std::string const&, XY>{fail(xy)};
    });
    EXPECT_EQ(r2a.error(), -100);
    EXPECT_EQ(v2, -100);
    EXPECT_EQ(r2c.error().x(), 25519);
    EXPECT_EQ(r2c.error().y(), -25519);

    fmt::memory_buffer buf3;
    auto bi3 = std::back_inserter(buf3);
    fmt::format_to(bi3, "AAA");
    Result<void, fmt::memory_buffer&> r3a{fail(ref(buf3))};
    auto const r3b = r3a.mapError([] (fmt::memory_buffer& b) {
        auto s = fmt::format("--{}", fmt::to_string(b));
        auto bi3b = std::back_inserter(b);
        fmt::format_to(bi3b, "BBB");
        return s;
    });
    constexpr auto TypeC3 =
            std::is_same_v<decltype(r3b), Result<void, std::string> const>;
    EXPECT_TRUE(TypeC3);
    EXPECT_EQ(r3b.error(), "--AAA"s);
    auto s3a1 = fmt::to_string(r3a.error());
    EXPECT_EQ(s3a1, "AAABBB"s);
    auto const r3c = r3a.flatMapError([] (fmt::memory_buffer& b) {
        auto s = fmt::format("++{}", fmt::to_string(b));
        auto bi3c = std::back_inserter(b);
        fmt::format_to(bi3c, "CCC");
        return Result<bool, std::string>{fail(s)};
    });
    EXPECT_TRUE(r3c.hasError());
    EXPECT_EQ(r3c.error(), "++AAABBB"s);
    auto s3a2 = fmt::to_string(r3a.error());
    EXPECT_EQ(s3a2, "AAABBBCCC"s);
    auto s3 = fmt::to_string(buf3);
    EXPECT_EQ(s3, s3a2);
}

TEST_F(ResultTests, TEEquals) {
    Result<int, int> const lhs1{25519};
    Result<long, long> const rhs1{25519};
    EXPECT_TRUE(lhs1 == rhs1);

    Result<int, int> const lhs2{25519};
    Result<long, long> const rhs2{0};
    EXPECT_FALSE(lhs2 == rhs2);

    Result<int, int> const lhs3{fail(25519)};
    Result<long, long> const rhs3{fail(25519)};;
    EXPECT_TRUE(lhs3 == rhs3);

    Result<int, int> const lhs4{fail(25519)};
    Result<long, long> const rhs4{fail(0)};
    EXPECT_FALSE(lhs2 == rhs2);

    Result<int, int> const lhs5{25519};
    Result<long, long> const rhs5{fail(25519)};
    EXPECT_FALSE(lhs5 == rhs5);

    Result<int, int> const lhs6{fail(25519)};
    Result<long, long> const rhs6{25519};
    EXPECT_FALSE(lhs6 == rhs6);
}

TEST_F(ResultTests, TENotEquals) {
    Result<int, int> const lhs1{25519};
    Result<long, long> const rhs1{25519};
    EXPECT_FALSE(lhs1 != rhs1);

    Result<int, int> const lhs2{25519};
    Result<long, long> const rhs2{0};
    EXPECT_TRUE(lhs2 != rhs2);

    Result<int, int> const lhs3{fail(25519)};
    Result<long, long> const rhs3{fail(25519)};;
    EXPECT_FALSE(lhs3 != rhs3);

    Result<int, int> const lhs4{fail(25519)};
    Result<long, long> const rhs4{fail(0)};
    EXPECT_TRUE(lhs2 != rhs2);

    Result<int, int> const lhs5{25519};
    Result<long, long> const rhs5{fail(25519)};
    EXPECT_TRUE(lhs5 != rhs5);

    Result<int, int> const lhs6{fail(25519)};
    Result<long, long> const rhs6{25519};
    EXPECT_TRUE(lhs6 != rhs6);
}

TEST_F(ResultTests, TELessThan) {
    Result<int, int> const lhs1{1};
    Result<long, long> const rhs1{100};
    EXPECT_TRUE(lhs1 < rhs1);

    Result<int, int> const lhs2{100};
    Result<long, long> const rhs2{1};
    EXPECT_FALSE(lhs2 < rhs2);

    Result<int, int> const lhs3{fail(1)};
    Result<long, long> const rhs3{fail(100)};
    EXPECT_TRUE(lhs3 < rhs3);

    Result<int, int> const lhs4{fail(100)};
    Result<long, long> const rhs4{fail(1)};
    EXPECT_FALSE(lhs4 < rhs4);

    Result<int, int> const lhs5{1};
    Result<long, long> const rhs5{fail(100)};
    EXPECT_FALSE(lhs5 < rhs5);

    Result<int, int> const lhs6{fail(100)};
    Result<long, long> const rhs6{1};
    EXPECT_TRUE(lhs6 < rhs6);

    Result<int, int> const lhs7{1};
    Result<long, long> const rhs7{1};
    EXPECT_FALSE(lhs7 < rhs7);

    Result<int, int> const lhs8{fail(1)};
    Result<long, long> const rhs8{fail(1)};
    EXPECT_FALSE(lhs8 < rhs8);
}

TEST_F(ResultTests, TELessThanOrEqual) {
    Result<int, int> const lhs1{1};
    Result<long, long> const rhs1{100};
    EXPECT_TRUE(lhs1 <= rhs1);

    Result<int, int> const lhs2{100};
    Result<long, long> const rhs2{1};
    EXPECT_FALSE(lhs2 <= rhs2);

    Result<int, int> const lhs3{fail(1)};
    Result<long, long> const rhs3{fail(100)};
    EXPECT_TRUE(lhs3 <= rhs3);

    Result<int, int> const lhs4{fail(100)};
    Result<long, long> const rhs4{fail(1)};
    EXPECT_FALSE(lhs4 <= rhs4);

    Result<int, int> const lhs5{1};
    Result<long, long> const rhs5{fail(100)};
    EXPECT_FALSE(lhs5 <= rhs5);

    Result<int, int> const lhs6{fail(100)};
    Result<long, long> const rhs6{1};
    EXPECT_TRUE(lhs6 <= rhs6);

    Result<int, int> const lhs7{1};
    Result<long, long> const rhs7{1};
    EXPECT_TRUE(lhs7 <= rhs7);

    Result<int, int> const lhs8{fail(1)};
    Result<long, long> const rhs8{fail(1)};
    EXPECT_TRUE(lhs8 <= rhs8);
}

TEST_F(ResultTests, TEGreaterThan) {
    Result<int, int> const lhs1{1};
    Result<long, long> const rhs1{100};
    EXPECT_FALSE(lhs1 > rhs1);

    Result<int, int> const lhs2{100};
    Result<long, long> const rhs2{1};
    EXPECT_TRUE(lhs2 > rhs2);

    Result<int, int> const lhs3{fail(1)};
    Result<long, long> const rhs3{fail(100)};
    EXPECT_FALSE(lhs3 > rhs3);

    Result<int, int> const lhs4{fail(100)};
    Result<long, long> const rhs4{fail(1)};
    EXPECT_TRUE(lhs4 > rhs4);

    Result<int, int> const lhs5{1};
    Result<long, long> const rhs5{fail(100)};
    EXPECT_TRUE(lhs5 > rhs5);

    Result<int, int> const lhs6{fail(100)};
    Result<long, long> const rhs6{1};
    EXPECT_FALSE(lhs6 > rhs6);

    Result<int, int> const lhs7{1};
    Result<long, long> const rhs7{1};
    EXPECT_FALSE(lhs7 > rhs7);

    Result<int, int> const lhs8{fail(1)};
    Result<long, long> const rhs8{fail(1)};
    EXPECT_FALSE(lhs8 > rhs8);
}

TEST_F(ResultTests, TEGreaterThanOrEqual) {
    Result<int, int> const lhs1{1};
    Result<long, long> const rhs1{100};
    EXPECT_FALSE(lhs1 >= rhs1);

    Result<int, int> const lhs2{100};
    Result<long, long> const rhs2{1};
    EXPECT_TRUE(lhs2 >= rhs2);

    Result<int, int> const lhs3{fail(1)};
    Result<long, long> const rhs3{fail(100)};
    EXPECT_FALSE(lhs3 >= rhs3);

    Result<int, int> const lhs4{fail(100)};
    Result<long, long> const rhs4{fail(1)};
    EXPECT_TRUE(lhs4 >= rhs4);

    Result<int, int> const lhs5{1};
    Result<long, long> const rhs5{fail(100)};
    EXPECT_TRUE(lhs5 >= rhs5);

    Result<int, int> const lhs6{fail(100)};
    Result<long, long> const rhs6{1};
    EXPECT_FALSE(lhs6 >= rhs6);

    Result<int, int> const lhs7{1};
    Result<long, long> const rhs7{1};
    EXPECT_TRUE(lhs7 >= rhs7);

    Result<int, int> const lhs8{fail(1)};
    Result<long, long> const rhs8{fail(1)};
    EXPECT_TRUE(lhs8 >= rhs8);
}

TEST_F(ResultTests, VoidEEquals) {
    Result<void, int> lhs1;
    Result<void, long> rhs1;
    EXPECT_TRUE(lhs1 == rhs1);

    Result<void, int> lhs2{fail(25519)};
    Result<void, long> rhs2{fail(25519)};
    EXPECT_TRUE(lhs2 == rhs2);

    Result<void, int> lhs3{fail(1)};
    Result<void, long> rhs3{fail(100)};
    EXPECT_FALSE(lhs3 == rhs3);

    Result<void, int> lhs4;
    Result<void, long> rhs4{fail(100)};
    EXPECT_FALSE(lhs4 == rhs4);

    Result<void, int> lhs5{fail(100)};
    Result<void, long> rhs5;
    EXPECT_FALSE(lhs5 == rhs5);
}

TEST_F(ResultTests, VoidENotEquals) {
    Result<void, int> lhs1;
    Result<void, long> rhs1;
    EXPECT_FALSE(lhs1 != rhs1);

    Result<void, int> lhs2{fail(25519)};
    Result<void, long> rhs2{fail(25519)};
    EXPECT_FALSE(lhs2 != rhs2);

    Result<void, int> lhs3{fail(1)};
    Result<void, long> rhs3{fail(100)};
    EXPECT_TRUE(lhs3 != rhs3);

    Result<void, int> lhs4;
    Result<void, long> rhs4{fail(100)};
    EXPECT_TRUE(lhs4 != rhs4);

    Result<void, int> lhs5{fail(100)};
    Result<void, long> rhs5;
    EXPECT_TRUE(lhs5 != rhs5);
}

TEST_F(ResultTests, VoidELessThan) {
    Result<void, int> lhs1;
    Result<void, long> rhs1;
    EXPECT_FALSE(lhs1 < rhs1);

    Result<void, int> lhs2{fail(1)};
    Result<void, long> rhs2{fail(100)};
    EXPECT_TRUE(lhs2 < rhs2);

    Result<void, int> lhs3{fail(100)};
    Result<void, long> rhs3{fail(1)};
    EXPECT_FALSE(lhs3 < rhs3);

    Result<void, int> lhs4{};
    Result<void, long> rhs4{fail(-100)};
    EXPECT_FALSE(lhs4 < rhs4);

    Result<void, int> lhs5{fail(100)};
    Result<void, long> rhs5;
    EXPECT_TRUE(lhs5 < rhs5);

    Result<void, int> lhs6{fail(25519)};
    Result<void, long> rhs6{fail(25519)};
    EXPECT_FALSE(lhs6 < rhs6);
}

TEST_F(ResultTests, VoidELessThanOrEqual) {
    Result<void, int> lhs1;
    Result<void, long> rhs1;
    EXPECT_TRUE(lhs1 <= rhs1);

    Result<void, int> lhs2{fail(1)};
    Result<void, long> rhs2{fail(100)};
    EXPECT_TRUE(lhs2 <= rhs2);

    Result<void, int> lhs3{fail(100)};
    Result<void, long> rhs3{fail(1)};
    EXPECT_FALSE(lhs3 <= rhs3);

    Result<void, int> lhs4{};
    Result<void, long> rhs4{fail(-100)};
    EXPECT_FALSE(lhs4 <= rhs4);

    Result<void, int> lhs5{fail(100)};
    Result<void, long> rhs5;
    EXPECT_TRUE(lhs5 <= rhs5);

    Result<void, int> lhs6{fail(25519)};
    Result<void, long> rhs6{fail(25519)};
    EXPECT_TRUE(lhs6 <= rhs6);
}


TEST_F(ResultTests, VoidEGreaterThan) {
    Result<void, int> lhs1;
    Result<void, long> rhs1;
    EXPECT_FALSE(lhs1 > rhs1);

    Result<void, int> lhs2{fail(1)};
    Result<void, long> rhs2{fail(100)};
    EXPECT_FALSE(lhs2 > rhs2);

    Result<void, int> lhs3{fail(100)};
    Result<void, long> rhs3{fail(1)};
    EXPECT_TRUE(lhs3 > rhs3);

    Result<void, int> lhs4{};
    Result<void, long> rhs4{fail(-100)};
    EXPECT_TRUE(lhs4 > rhs4);

    Result<void, int> lhs5{fail(100)};
    Result<void, long> rhs5;
    EXPECT_FALSE(lhs5 > rhs5);

    Result<void, int> lhs6{fail(25519)};
    Result<void, long> rhs6{fail(25519)};
    EXPECT_FALSE(lhs6 > rhs6);
}

TEST_F(ResultTests, VoidEGreaterThanOrEqual) {
    Result<void, int> lhs1;
    Result<void, long> rhs1;
    EXPECT_TRUE(lhs1 >= rhs1);

    Result<void, int> lhs2{fail(1)};
    Result<void, long> rhs2{fail(100)};
    EXPECT_FALSE(lhs2 >= rhs2);

    Result<void, int> lhs3{fail(100)};
    Result<void, long> rhs3{fail(1)};
    EXPECT_TRUE(lhs3 >= rhs3);

    Result<void, int> lhs4{};
    Result<void, long> rhs4{fail(-100)};
    EXPECT_TRUE(lhs4 >= rhs4);

    Result<void, int> lhs5{fail(100)};
    Result<void, long> rhs5;
    EXPECT_FALSE(lhs5 >= rhs5);

    Result<void, int> lhs6{fail(25519)};
    Result<void, long> rhs6{fail(25519)};
    EXPECT_TRUE(lhs6 >= rhs6);
}

TEST_F(ResultTests, TEAgainstValueEquals) {
    Result <int, std::string> lhs1{0};
    long const rhs1{0l};
    EXPECT_TRUE(lhs1 == rhs1);

    long const lhs2{25519l};
    Result<int, std::string> rhs2{25519};
    EXPECT_TRUE(lhs2 == rhs2);

    Result <int, std::string> lhs3{25519};
    long const rhs3{0l};
    EXPECT_FALSE(lhs3 == rhs3);

    long const lhs4{25519l};
    Result<int, std::string> rhs4{0};
    EXPECT_FALSE(lhs4 == rhs4);

    Result<int, std::string> lhs5{fail("0")};
    long const rhs5{0};
    EXPECT_FALSE(lhs5 == rhs5);

    long const lhs6{0};
    Result<int, std::string> rhs6{fail("0")};
    EXPECT_FALSE(lhs6 == rhs6);
}

TEST_F(ResultTests, TEAgainstValueNotEquals) {
    Result <int, std::string> lhs1{0};
    long const rhs1{0l};
    EXPECT_FALSE(lhs1 != rhs1);

    long const lhs2{25519l};
    Result<int, std::string> rhs2{25519};
    EXPECT_FALSE(lhs2 != rhs2);

    Result <int, std::string> lhs3{25519};
    long const rhs3{0l};
    EXPECT_TRUE(lhs3 != rhs3);

    long const lhs4{25519l};
    Result<int, std::string> rhs4{0};
    EXPECT_TRUE(lhs4 != rhs4);

    Result<int, std::string> lhs5{fail("0")};
    long const rhs5{0};
    EXPECT_TRUE(lhs5 != rhs5);

    long const lhs6{0};
    Result<int, std::string> rhs6{fail("0")};
    EXPECT_TRUE(lhs6 != rhs6);
}

TEST_F(ResultTests, TEAgainstValueLessThan) {
    Result<int, int> const lhs1{1};
    long const rhs1{100};
    EXPECT_TRUE(lhs1 < rhs1);

    Result<int, int> const lhs2{100};
    long const rhs2{1};
    EXPECT_FALSE(lhs2 < rhs2);

    long const lhs3{1};
    Result<int, int> const rhs3{100};
    EXPECT_TRUE(lhs3 < rhs3);

    long const lhs4{100};
    Result<int, int> const rhs4{1};
    EXPECT_FALSE(lhs4 < rhs4);

    Result<int, int> const lhs5{fail(100)};
    long const rhs5{1};
    EXPECT_TRUE(lhs5 < rhs5);

    long const lhs6{1};
    Result<int, int> const rhs6{fail(100)};
    EXPECT_FALSE(lhs6 < rhs6);

    Result<int, int> const lhs7{1};
    long const rhs7{1};
    EXPECT_FALSE(lhs7 < rhs7);

    long const rhs8{1};
    Result<int, int> const lhs8{1};
    EXPECT_FALSE(lhs8 < rhs8);
}

TEST_F(ResultTests, TEAgainstValueLessThanOrEqual) {
    Result<int, int> const lhs1{1};
    long const rhs1{100};
    EXPECT_TRUE(lhs1 <= rhs1);

    Result<int, int> const lhs2{100};
    long const rhs2{1};
    EXPECT_FALSE(lhs2 <= rhs2);

    long const lhs3{1};
    Result<int, int> const rhs3{100};
    EXPECT_TRUE(lhs3 <= rhs3);

    long const lhs4{100};
    Result<int, int> const rhs4{1};
    EXPECT_FALSE(lhs4 <= rhs4);

    Result<int, int> const lhs5{fail(100)};
    long const rhs5{1};
    EXPECT_TRUE(lhs5 <= rhs5);

    long const lhs6{1};
    Result<int, int> const rhs6{fail(100)};
    EXPECT_FALSE(lhs6 <= rhs6);

    Result<int, int> const lhs7{1};
    long const rhs7{1};
    EXPECT_TRUE(lhs7 <= rhs7);

    long const rhs8{1};
    Result<int, int> const lhs8{1};
    EXPECT_TRUE(lhs8 <= rhs8);
}


TEST_F(ResultTests, TEAgainstValueGreaterThan) {
    Result<int, int> const lhs1{1};
    long const rhs1{100};
    EXPECT_FALSE(lhs1 > rhs1);

    Result<int, int> const lhs2{100};
    long const rhs2{1};
    EXPECT_TRUE(lhs2 > rhs2);

    long const lhs3{1};
    Result<int, int> const rhs3{100};
    EXPECT_FALSE(lhs3 > rhs3);

    long const lhs4{100};
    Result<int, int> const rhs4{1};
    EXPECT_TRUE(lhs4 > rhs4);

    Result<int, int> const lhs5{fail(100)};
    long const rhs5{1};
    EXPECT_FALSE(lhs5 > rhs5);

    long const lhs6{1};
    Result<int, int> const rhs6{fail(100)};
    EXPECT_TRUE(lhs6 > rhs6);

    Result<int, int> const lhs7{1};
    long const rhs7{1};
    EXPECT_FALSE(lhs7 > rhs7);

    long const rhs8{1};
    Result<int, int> const lhs8{1};
    EXPECT_FALSE(lhs8 > rhs8);
}

TEST_F(ResultTests, TEAgainstValueGreaterThanOrEqual) {
    Result<int, int> const lhs1{1};
    long const rhs1{100};
    EXPECT_FALSE(lhs1 >= rhs1);

    Result<int, int> const lhs2{100};
    long const rhs2{1};
    EXPECT_TRUE(lhs2 >= rhs2);

    long const lhs3{1};
    Result<int, int> const rhs3{100};
    EXPECT_FALSE(lhs3 >= rhs3);

    long const lhs4{100};
    Result<int, int> const rhs4{1};
    EXPECT_TRUE(lhs4 >= rhs4);

    Result<int, int> const lhs5{fail(100)};
    long const rhs5{1};
    EXPECT_FALSE(lhs5 >= rhs5);

    long const lhs6{1};
    Result<int, int> const rhs6{fail(100)};
    EXPECT_TRUE(lhs6 >= rhs6);

    Result<int, int> const lhs7{1};
    long const rhs7{1};
    EXPECT_TRUE(lhs7 >= rhs7);

    long const rhs8{1};
    Result<int, int> const lhs8{1};
    EXPECT_TRUE(lhs8 >= rhs8);
}

TEST_F(ResultTests, TEAgainstFailureEquals) {
    Result<int, std::string> lhs1{1};
    auto const rhs1 = fail("1");
    EXPECT_FALSE(lhs1 == rhs1);

    auto const lhs2 = fail("1");
    Result<int, std::string> rhs2{1};
    EXPECT_FALSE(lhs2 == rhs2);

    Result<int, std::string> lhs3{InPlaceError, strValue1};
    auto const rhs3 = fail(cstrValue1);
    EXPECT_TRUE(lhs3 == rhs3);

    auto const lhs4 = fail(cstrValue1);
    Result<int, std::string> rhs4{InPlaceError, cstrValue1};
    EXPECT_TRUE(lhs4 == rhs4);

    Result<int, std::string> lhs5{InPlaceError, "100"};
    auto const rhs5 = fail("1");
    EXPECT_FALSE(lhs5 == rhs5);

    auto const rhs6 = fail("100");
    Result<int, std::string> lhs6{InPlaceError, "1"};
    EXPECT_FALSE(lhs6 == rhs6);
}

TEST_F(ResultTests, TEAgainstFailureNotEquals) {
    Result<int, std::string> lhs1{1};
    auto const rhs1 = fail("1");
    EXPECT_TRUE(lhs1 != rhs1);

    auto const lhs2 = fail("1");
    Result<int, std::string> rhs2{1};
    EXPECT_TRUE(lhs2 != rhs2);

    Result<int, std::string> lhs3{InPlaceError, strValue1};
    auto const rhs3 = fail(cstrValue1);
    EXPECT_FALSE(lhs3 != rhs3);

    auto const lhs4 = fail(cstrValue1);
    Result<int, std::string> rhs4{InPlaceError, cstrValue1};
    EXPECT_FALSE(lhs4 != rhs4);

    Result<int, std::string> lhs5{InPlaceError, "100"};
    auto const rhs5 = fail("1");
    EXPECT_TRUE(lhs5 != rhs5);

    auto const rhs6 = fail("100");
    Result<int, std::string> lhs6{InPlaceError, "1"};
    EXPECT_TRUE(lhs6 != rhs6);
}

TEST_F(ResultTests, TEAgainstFailureLessThan) {
    Result<int, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_TRUE(lhs1 < rhs1);

    Result<int, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_FALSE(lhs2 < rhs2);

    auto const lhs3 = fail(1);
    Result<int, int> const rhs3{fail(100)};
    EXPECT_TRUE(lhs3 < rhs3);

    auto const lhs4 = fail(100);
    Result<int, int> const rhs4{fail(1)};
    EXPECT_FALSE(lhs4 < rhs4);

    Result<int, int> const lhs5{100};
    auto const rhs5 = fail(1);
    EXPECT_FALSE(lhs5 < rhs5);

    auto const lhs6 = fail(1);
    Result<int, int> const rhs6{100};
    EXPECT_TRUE(lhs6 < rhs6);

    Result<int, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_FALSE(lhs7 < rhs7);

    auto const rhs8 = fail(1);
    Result<int, int> const lhs8{fail(1)};
    EXPECT_FALSE(lhs8 < rhs8);
}

TEST_F(ResultTests, TEAgainstFailureLessThanOrEqual) {
    Result<int, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_TRUE(lhs1 <= rhs1);

    Result<int, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_FALSE(lhs2 <= rhs2);

    auto const lhs3 = fail(1);
    Result<int, int> const rhs3{fail(100)};
    EXPECT_TRUE(lhs3 <= rhs3);

    auto const lhs4 = fail(100);
    Result<int, int> const rhs4{fail(1)};
    EXPECT_FALSE(lhs4 <= rhs4);

    Result<int, int> const lhs5{100};
    auto const rhs5 = fail(1);
    EXPECT_FALSE(lhs5 <= rhs5);

    auto const lhs6 = fail(1);
    Result<int, int> const rhs6{100};
    EXPECT_TRUE(lhs6 <= rhs6);

    Result<int, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_TRUE(lhs7 <= rhs7);

    auto const rhs8 = fail(1);
    Result<int, int> const lhs8{fail(1)};
    EXPECT_TRUE(lhs8 <= rhs8);
}

TEST_F(ResultTests, TEAgainstFailureGreaterThan) {
    Result<int, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_FALSE(lhs1 > rhs1);

    Result<int, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_TRUE(lhs2 > rhs2);

    auto const lhs3 = fail(1);
    Result<int, int> const rhs3{fail(100)};
    EXPECT_FALSE(lhs3 > rhs3);

    auto const lhs4 = fail(100);
    Result<int, int> const rhs4{fail(1)};
    EXPECT_TRUE(lhs4 > rhs4);

    Result<int, int> const lhs5{100};
    auto const rhs5 = fail(1);
    EXPECT_TRUE(lhs5 > rhs5);

    auto const lhs6 = fail(1);
    Result<int, int> const rhs6{100};
    EXPECT_FALSE(lhs6 > rhs6);

    Result<int, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_FALSE(lhs7 > rhs7);

    auto const rhs8 = fail(1);
    Result<int, int> const lhs8{fail(1)};
    EXPECT_FALSE(lhs8 > rhs8);
}

TEST_F(ResultTests, TEAgainstFailureGreaterThanOrEqual) {
    Result<int, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_FALSE(lhs1 >= rhs1);

    Result<int, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_TRUE(lhs2 >= rhs2);

    auto const lhs3 = fail(1);
    Result<int, int> const rhs3{fail(100)};
    EXPECT_FALSE(lhs3 >= rhs3);

    auto const lhs4 = fail(100);
    Result<int, int> const rhs4{fail(1)};
    EXPECT_TRUE(lhs4 >= rhs4);

    Result<int, int> const lhs5{100};
    auto const rhs5 = fail(1);
    EXPECT_TRUE(lhs5 >= rhs5);

    auto const lhs6 = fail(1);
    Result<int, int> const rhs6{100};
    EXPECT_FALSE(lhs6 >= rhs6);

    Result<int, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_TRUE(lhs7 >= rhs7);

    auto const rhs8 = fail(1);
    Result<int, int> const lhs8{fail(1)};
    EXPECT_TRUE(lhs8 >= rhs8);
}


TEST_F(ResultTests, VoidEAgainstFailureEquals) {
    Result<void, std::string> lhs1;
    auto const rhs1 = fail("1");
    EXPECT_FALSE(lhs1 == rhs1);

    auto const lhs2 = fail("1");
    Result<void, std::string> rhs2;
    EXPECT_FALSE(lhs2 == rhs2);

    Result<void, std::string> lhs3{InPlaceError, strValue1};
    auto const rhs3 = fail(cstrValue1);
    EXPECT_TRUE(lhs3 == rhs3);

    auto const lhs4 = fail(cstrValue1);
    Result<void, std::string> rhs4{InPlaceError, cstrValue1};
    EXPECT_TRUE(lhs4 == rhs4);

    Result<void, std::string> lhs5{InPlaceError, "100"};
    auto const rhs5 = fail("1");
    EXPECT_FALSE(lhs5 == rhs5);

    auto const rhs6 = fail("100");
    Result<void, std::string> lhs6{InPlaceError, "1"};
    EXPECT_FALSE(lhs6 == rhs6);
}

TEST_F(ResultTests, VoidEAgainstFailureNotEquals) {
    Result<void, std::string> lhs1;
    auto const rhs1 = fail("1");
    EXPECT_TRUE(lhs1 != rhs1);

    auto const lhs2 = fail("1");
    Result<void, std::string> rhs2;
    EXPECT_TRUE(lhs2 != rhs2);

    Result<void, std::string> lhs3{InPlaceError, strValue1};
    auto const rhs3 = fail(cstrValue1);
    EXPECT_FALSE(lhs3 != rhs3);

    auto const lhs4 = fail(cstrValue1);
    Result<void, std::string> rhs4{InPlaceError, cstrValue1};
    EXPECT_FALSE(lhs4 != rhs4);

    Result<void, std::string> lhs5{InPlaceError, "100"};
    auto const rhs5 = fail("1");
    EXPECT_TRUE(lhs5 != rhs5);

    auto const rhs6 = fail("100");
    Result<void, std::string> lhs6{InPlaceError, "1"};
    EXPECT_TRUE(lhs6 != rhs6);
}

TEST_F(ResultTests, VoidEAgainstFailureLessThan) {
    Result<void, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_TRUE(lhs1 < rhs1);

    Result<void, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_FALSE(lhs2 < rhs2);

    auto const lhs3 = fail(1);
    Result<void, int> const rhs3{fail(100)};
    EXPECT_TRUE(lhs3 < rhs3);

    auto const lhs4 = fail(100);
    Result<void, int> const rhs4{fail(1)};
    EXPECT_FALSE(lhs4 < rhs4);

    Result<void, int> const lhs5;
    auto const rhs5 = fail(1);
    EXPECT_FALSE(lhs5 < rhs5);

    auto const lhs6 = fail(1);
    Result<void, int> const rhs6;
    EXPECT_TRUE(lhs6 < rhs6);

    Result<void, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_FALSE(lhs7 < rhs7);

    auto const rhs8 = fail(1);
    Result<void, int> const lhs8{fail(1)};
    EXPECT_FALSE(lhs8 < rhs8);
}

TEST_F(ResultTests, VoidEAgainstFailureLessThanOrEqual) {
    Result<void, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_TRUE(lhs1 <= rhs1);

    Result<void, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_FALSE(lhs2 <= rhs2);

    auto const lhs3 = fail(1);
    Result<void, int> const rhs3{fail(100)};
    EXPECT_TRUE(lhs3 <= rhs3);

    auto const lhs4 = fail(100);
    Result<void, int> const rhs4{fail(1)};
    EXPECT_FALSE(lhs4 <= rhs4);

    Result<void, int> const lhs5;
    auto const rhs5 = fail(1);
    EXPECT_FALSE(lhs5 <= rhs5);

    auto const lhs6 = fail(1);
    Result<void, int> const rhs6;
    EXPECT_TRUE(lhs6 <= rhs6);

    Result<void, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_TRUE(lhs7 <= rhs7);

    auto const rhs8 = fail(1);
    Result<void, int> const lhs8{fail(1)};
    EXPECT_TRUE(lhs8 <= rhs8);
}

TEST_F(ResultTests, VoidEAgainstFailureGreaterThan) {
    Result<void, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_FALSE(lhs1 > rhs1);

    Result<void, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_TRUE(lhs2 > rhs2);

    auto const lhs3 = fail(1);
    Result<void, int> const rhs3{fail(100)};
    EXPECT_FALSE(lhs3 > rhs3);

    auto const lhs4 = fail(100);
    Result<void, int> const rhs4{fail(1)};
    EXPECT_TRUE(lhs4 > rhs4);

    Result<void, int> const lhs5;
    auto const rhs5 = fail(1);
    EXPECT_TRUE(lhs5 > rhs5);

    auto const lhs6 = fail(1);
    Result<void, int> const rhs6;
    EXPECT_FALSE(lhs6 > rhs6);

    Result<void, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_FALSE(lhs7 > rhs7);

    auto const rhs8 = fail(1);
    Result<void, int> const lhs8{fail(1)};
    EXPECT_FALSE(lhs8 > rhs8);
}

TEST_F(ResultTests, VoidEAgainstFailureGreaterThanOrEqual) {
    Result<void, int> const lhs1{fail(1)};
    auto const rhs1 = fail(100);
    EXPECT_FALSE(lhs1 >= rhs1);

    Result<void, int> const lhs2{fail(100)};
    auto const rhs2 = fail(1);
    EXPECT_TRUE(lhs2 >= rhs2);

    auto const lhs3 = fail(1);
    Result<void, int> const rhs3{fail(100)};
    EXPECT_FALSE(lhs3 >= rhs3);

    auto const lhs4 = fail(100);
    Result<void, int> const rhs4{fail(1)};
    EXPECT_TRUE(lhs4 >= rhs4);

    Result<void, int> const lhs5;
    auto const rhs5 = fail(1);
    EXPECT_TRUE(lhs5 >= rhs5);

    auto const lhs6 = fail(1);
    Result<void, int> const rhs6;
    EXPECT_FALSE(lhs6 >= rhs6);

    Result<void, int> const lhs7{fail(1)};
    auto const rhs7 = fail(1);
    EXPECT_TRUE(lhs7 >= rhs7);

    auto const rhs8 = fail(1);
    Result<void, int> const lhs8{fail(1)};
    EXPECT_TRUE(lhs8 >= rhs8);
}

TEST_F(ResultTests, TESwap) {
    Result<int, std::string> lhs1{1};
    Result<int, std::string> rhs1{100};
    auto const lhs1c = lhs1;
    auto const rhs1c = rhs1;
    swap(lhs1, rhs1);
    EXPECT_TRUE(lhs1.hasValue());
    EXPECT_TRUE(rhs1.hasValue());
    EXPECT_EQ(lhs1, rhs1c);
    EXPECT_EQ(rhs1, lhs1c);

    Result<int, std::string> lhs2{fail(cstrValue1)};
    Result<int, std::string> rhs2{fail("1")};
    auto const lhs2c = lhs2;
    auto const rhs2c = rhs2;
    swap(lhs2, rhs2);
    EXPECT_TRUE(lhs2.hasError());
    EXPECT_TRUE(rhs2.hasError());
    EXPECT_EQ(lhs2, rhs2c);
    EXPECT_EQ(rhs2, lhs2c);

    Result<int, std::string> lhs3{25519};
    Result<int, std::string> rhs3{fail(cstrValue1)};
    auto const lhs3c = lhs3;
    auto const rhs3c = rhs3;
    swap(lhs3, rhs3);
    EXPECT_TRUE(lhs3.hasError());
    EXPECT_TRUE(rhs3.hasValue());
    EXPECT_EQ(lhs3, rhs3c);
    EXPECT_EQ(rhs3, lhs3c);

    Result<int, std::string> lhs4{fail(cstrValue1)};
    Result<int, std::string> rhs4{25519};
    auto const lhs4c = lhs4;
    auto const rhs4c = rhs4;
    swap(lhs4, rhs4);
    EXPECT_TRUE(lhs4.hasValue());
    EXPECT_TRUE(rhs4.hasError());
    EXPECT_EQ(lhs4, rhs4c);
    EXPECT_EQ(rhs4, lhs4c);
}

TEST_F(ResultTests, VoidESwap) {
    Result<void, std::string> lhs1{};
    Result<void, std::string> rhs1{};
    auto const lhs1c = lhs1;
    auto const rhs1c = rhs1;
    swap(lhs1, rhs1);
    EXPECT_TRUE(lhs1.hasValue());
    EXPECT_TRUE(rhs1.hasValue());
    EXPECT_EQ(lhs1, rhs1c);
    EXPECT_EQ(rhs1, lhs1c);

    Result<void, std::string> lhs2{fail(cstrValue1)};
    Result<void, std::string> rhs2{fail("1")};
    auto const lhs2c = lhs2;
    auto const rhs2c = rhs2;
    swap(lhs2, rhs2);
    EXPECT_TRUE(lhs2.hasError());
    EXPECT_TRUE(rhs2.hasError());
    EXPECT_EQ(lhs2, rhs2c);
    EXPECT_EQ(rhs2, lhs2c);

    Result<void, std::string> lhs3{};
    Result<void, std::string> rhs3{fail(cstrValue1)};
    auto const lhs3c = lhs3;
    auto const rhs3c = rhs3;
    swap(lhs3, rhs3);
    EXPECT_TRUE(lhs3.hasError());
    EXPECT_TRUE(rhs3.hasValue());
    EXPECT_EQ(lhs3, rhs3c);
    EXPECT_EQ(rhs3, lhs3c);

    Result<void, std::string> lhs4{fail(cstrValue1)};
    Result<void, std::string> rhs4{};
    auto const lhs4c = lhs4;
    auto const rhs4c = rhs4;
    swap(lhs4, rhs4);
    EXPECT_TRUE(lhs4.hasValue());
    EXPECT_TRUE(rhs4.hasError());
    EXPECT_EQ(lhs4, rhs4c);
    EXPECT_EQ(rhs4, lhs4c);
}

} // namespace pimc::testing
