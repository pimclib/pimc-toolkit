#include <utility>
#include <type_traits>
#include <concepts>
#include <string>

#include <gtest/gtest.h>
#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Optional.hpp"

#include "Optional-Result-TestUtils.hpp"

using namespace std::string_literals;

namespace pimc::testing {

class OptionalTests : public ::testing::Test {
protected:

    char const *cstrValue1 = "ed25519 is a very nice elliptic curve";
    std::string strValue1 = "ed25519 is a very nice elliptic curve"s;
};

TEST_F(OptionalTests, TEDefaultConstructibleProperties) {
    constexpr auto DefaultConstructibleC1 =
            std::is_default_constructible_v<Optional<int>>;
    EXPECT_TRUE(DefaultConstructibleC1);

    std::string s{};
    auto r1 = Optional<std::string>{};

    EXPECT_FALSE(r1.hasValue());

    // We should be able to construct an empty optional even if the
    // value is not default contructible
    constexpr auto DefaultConstructibleC2 =
            std::is_default_constructible_v<Optional<NotDefaultConstructible>>;
    EXPECT_TRUE(DefaultConstructibleC2);
}

TEST_F(OptionalTests, TECopyConstructibleProperties) {
    constexpr auto TriviallyCopyConstructibleC =
            std::is_trivially_copy_constructible_v<Optional<int>>;
    EXPECT_TRUE(TriviallyCopyConstructibleC);

    int const v1{25519};
    const Optional<int> r1{v1};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_TRUE(*r1 == v1);

    constexpr auto NotTriviallyCopyConstructibleC =
            not std::is_trivially_copy_constructible_v<Optional<std::string>>;
    EXPECT_TRUE(NotTriviallyCopyConstructibleC);

    constexpr auto CopyConstructibleC =
            std::is_copy_constructible_v<Optional<std::string>>;
    EXPECT_TRUE(CopyConstructibleC);

    Optional<std::string> const r3{cstrValue1};
    EXPECT_TRUE(r3.hasValue());
    EXPECT_TRUE(*r3 == strValue1);

    Optional<std::string> const r4a{cstrValue1};
    auto const r4b = r4a;
    EXPECT_TRUE(r4b.hasValue());
    EXPECT_TRUE(*r4b == strValue1);

    int const v6 = 25519;
    Optional<int> const r6a{v6};
    auto const r6b = r6a;
    EXPECT_TRUE(r6a.hasValue());
    EXPECT_TRUE(r6b.hasValue());
    EXPECT_TRUE(*r6b == v6);

    Optional<std::string> const r8a{cstrValue1};
    auto const r8b = r8a;
    EXPECT_TRUE(r8a.hasValue());
    EXPECT_TRUE(r8b.hasValue());
    EXPECT_TRUE(*r8a == strValue1);
    EXPECT_TRUE(*r8b == strValue1);

    constexpr auto NotCopyConstructibleC =
            not std::is_copy_constructible_v<Optional<NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyConstructibleC);
}

TEST_F(OptionalTests, TEMoveConstructibleProperties) {
    constexpr auto TriviallyMoveConstructibleC =
            std::is_trivially_move_constructible_v<Optional<int>>;
    EXPECT_TRUE(TriviallyMoveConstructibleC);

    int const v1{25519};
    const Optional<int> r1a{v1};
    auto const r1b = std::move(r1a);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_TRUE(*r1b == v1);

   constexpr auto NotTriviallyMoveConstructibleC =
            not std::is_trivially_move_constructible_v<Optional<MoveOnly<std::string>>>;
    EXPECT_TRUE(NotTriviallyMoveConstructibleC);

    constexpr auto MoveConstructibleC =
            std::is_move_constructible_v<Optional<MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveConstructibleC);

    Optional<MoveOnly<std::string>> r4a{cstrValue1};
    EXPECT_TRUE(r4a.hasValue());
    EXPECT_TRUE(*r4a == strValue1);
    auto const r4b = std::move(r4a);
    EXPECT_TRUE(r4a.hasValue());
    EXPECT_TRUE(*r4a == ""s);
    EXPECT_TRUE(r4b.hasValue());
    EXPECT_TRUE(*r4b == strValue1);


    constexpr auto NotMoveConstructibleC =
            not std::is_move_constructible_v<Optional<NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveConstructibleC);
}

TEST_F(OptionalTests, TEConversionConstructionProperties) {
    constexpr auto ConstructibleC =
            std::is_constructible_v<
                    Optional<std::string>,
                    Optional<char const*> const&>;
    EXPECT_TRUE(ConstructibleC);

    constexpr auto ConvertibleC =
            std::is_convertible_v<
                    Optional<char const*> const&,
                    Optional<std::string>>;
    EXPECT_TRUE(ConvertibleC);

    Optional<char const*> const r1a{cstrValue1};
    auto const r1b = r1a;
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_TRUE(*r1b == cstrValue1);

    constexpr auto NotConstructibleC1 =
            not std::is_constructible_v<
                    Optional<int>,
                    Optional<std::string> const&>;
    EXPECT_TRUE(NotConstructibleC1);
}

TEST_F(OptionalTests, TEExplicitConversionCopyConstructionProperties) {
    constexpr auto ConstructibleC1 =
            std::is_constructible_v<
                    Optional<Explicit<std::string>>,
                    Optional<std::string> const&>;
    EXPECT_TRUE(ConstructibleC1);

    constexpr auto NotConvertibleC1 =
            not std::is_convertible_v<
                    Optional<std::string> const&,
                    Optional<Explicit<std::string>>>;
    EXPECT_TRUE(NotConvertibleC1);

    Optional<std::string> const r1a{cstrValue1};
    Optional<Explicit<std::string>> const r1b{r1a};
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_EQ(*r1a, strValue1);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto NotConstructibleC =
            not std::is_constructible_v<
                    Optional<int>,
                    Optional<std::string>>;
    EXPECT_TRUE(NotConstructibleC);
}

TEST_F(OptionalTests, TEImplicitConversionMoveConstructionProperties) {
    constexpr auto ConstructibleC1 =
            std::is_constructible_v<
                    Optional<MoveOnly<std::string>>,
                    Optional<std::string>&&>;
    EXPECT_TRUE(ConstructibleC1);

    constexpr auto ConvertibleC1 =
            std::is_convertible_v<
                    Optional<std::string>&&,
                    Optional<MoveOnly<std::string>>>;
    EXPECT_TRUE(ConvertibleC1);

    Optional<std::string> r1a{cstrValue1};
    auto const r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto NotConstructibleC2 =
            not std::is_constructible_v<
                    Optional<int>,
                    Optional<std::string>&&>;
    EXPECT_TRUE(NotConstructibleC2);
}

TEST_F(OptionalTests, TEExplicitConversionMoveConstructionProperties) {
    constexpr auto ConstructibleC1 =
            std::is_constructible_v<
                    Optional<Explicit<MoveOnly<std::string>>>,
                    Optional<std::string>&&>;
    EXPECT_TRUE(ConstructibleC1);

    constexpr auto NotConvertibleC2 =
            not std::is_convertible_v<
                    Optional<std::string>&&,
                    Optional<Explicit<MoveOnly<std::string>>>>;
    EXPECT_TRUE(NotConvertibleC2);

    Optional<std::string> r1a{cstrValue1};
    Optional<Explicit<MoveOnly<std::string>>> const r1b{std::move(r1a)};
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, strValue1);
}

TEST_F(OptionalTests, TEInPlaceValueConstructionProperties) {
    auto const v1 = std::string{cstrValue1, 34ul};
    Optional<std::string> const r1{InPlaceValue, cstrValue1, 34ul};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, v1);
}

TEST_F(OptionalTests, TEInPlaceValueInitializerListConstructionProperties) {
    auto const v1 = std::string{IL_ED25519, std::allocator<char>{}};
    Optional<std::string> const r1{InPlaceValue, IL_ED25519, std::allocator<char>{}};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, v1);
}

TEST_F(OptionalTests, TEDestructionProperties) {
    constexpr auto TriviallyDestructibleC1 =
            std::is_trivially_destructible_v<Optional<int>>;
    EXPECT_TRUE(TriviallyDestructibleC1);

    constexpr auto NotTriviallyDestructibleC2 =
            not std::is_trivially_destructible_v<Optional<BoolSetterOnDestr>>;
    EXPECT_TRUE(NotTriviallyDestructibleC2);

    bool called1{false};
    {
        Optional<BoolSetterOnDestr> const r1{InPlaceValue, &called1};
    };
    EXPECT_TRUE(called1);
}

TEST_F(OptionalTests, TECopyAssignabilityProperties) {
    constexpr auto CopyAssignableC1 =
            std::is_copy_assignable_v<Optional<std::string>>;
    EXPECT_TRUE(CopyAssignableC1);

    Optional<std::string> const r1a{cstrValue1};
    Optional<std::string> r1b;
    r1b = r1a;
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, strValue1);
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto NotCopyAssignableC4 =
            not std::is_copy_assignable_v<Optional<NotCopyOrMoveable>>;
    EXPECT_TRUE(NotCopyAssignableC4);

    Optional<BoolSetterOnDestr> r5a{};
    bool called5{false};
    Optional<BoolSetterOnDestr> r5b{&called5};
    EXPECT_FALSE(r5a.hasValue());
    EXPECT_TRUE(r5b.hasValue());
    r5b = r5a;
    EXPECT_FALSE(r5b.hasValue());
    EXPECT_TRUE(called5);

    constexpr auto TriviallyCopyAssignableC7 =
            std::is_trivially_copy_assignable_v<Optional<int>>;
    EXPECT_TRUE(TriviallyCopyAssignableC7);
}

TEST_F(OptionalTests, TEMoveAssignabilityProperties) {
    constexpr auto MoveAssignableC1 =
            std::is_move_assignable_v<Optional<std::string>>;
    EXPECT_TRUE(MoveAssignableC1);

    Optional<std::string> r1a{cstrValue1};
    Optional<std::string> r1b;
    r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_EQ(*r1b, strValue1);

    constexpr auto NotMoveAssignableC4 =
            not std::is_move_assignable_v<Optional<NotCopyOrMoveable>>;
    EXPECT_TRUE(NotMoveAssignableC4);

    Optional<BoolSetterOnDestr> r5a{};
    bool called5{false};
    Optional<BoolSetterOnDestr> r5b{&called5};
    EXPECT_FALSE(r5a.hasValue());
    EXPECT_TRUE(r5b.hasValue());
    r5b = std::move(r5a);
    EXPECT_FALSE(r5a.hasValue());
    EXPECT_FALSE(r5b.hasValue());
    EXPECT_TRUE(called5);
    constexpr auto TriviallyMoveAssignableC7 =
            std::is_trivially_move_assignable_v<Optional<int>>;
    EXPECT_TRUE(TriviallyMoveAssignableC7);
}

TEST_F(OptionalTests, TEThrowingAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Optional<Throwing<std::string>>,
                    Optional<std::string> const&>;
    EXPECT_TRUE(AssignableC1);

    constexpr auto NotNothrowAssignableC1 =
            not std::is_nothrow_assignable_v<
                    Optional<Throwing<std::string>>,
                    Optional<std::string> const&>;
    EXPECT_TRUE(NotNothrowAssignableC1);

    constexpr auto AssignableC3 =
            std::is_assignable_v<
                    Optional<Throwing<std::string>>,
                    Optional<std::string>&&>;
    EXPECT_TRUE(AssignableC3);

    constexpr auto NotNothrowAssignableC3 =
            not std::is_nothrow_assignable_v<
                    Optional<Throwing<std::string>>,
                    Optional<std::string>&&>;
    EXPECT_TRUE(NotNothrowAssignableC3);
}

TEST_F(OptionalTests, TEConversionCopyAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Optional<CopyOnly<std::string>>,
                    Optional<std::string> const&>;
    EXPECT_TRUE(AssignableC1);

    Optional<std::string> const r1a{cstrValue1};
    Optional<CopyOnly<std::string>> r1b;
    r1b = r1a;
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, strValue1);
    EXPECT_EQ(*r1b, strValue1);
}

TEST_F(OptionalTests, TEConversionMoveAssignabilityProperties) {
    constexpr auto AssignableC1 =
            std::is_assignable_v<
                    Optional<MoveOnly<std::string>>,
                    Optional<std::string>&&>;
    EXPECT_TRUE(AssignableC1);

    Optional<std::string> r1a{cstrValue1};
    Optional<MoveOnly<std::string>> r1b;
    r1b = std::move(r1a);
    EXPECT_TRUE(r1a.hasValue());
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1a, ""s);
    EXPECT_EQ(*r1b, strValue1);
}

TEST_F(OptionalTests, TEAssignabilityProperties) {
    std::string const s1{cstrValue1};
    Optional<std::string> r1{InPlaceValue};
    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(*r1, ""s);
    r1 = s1;
    EXPECT_EQ(*r1, strValue1);

    std::string s5{cstrValue1};
    Optional<MoveOnly<std::string>> r5{};
    EXPECT_FALSE(r5.hasValue());
    r5 = std::move(s5);
    EXPECT_TRUE(r5.hasValue());
    EXPECT_EQ(*r5, strValue1);
    EXPECT_TRUE(s5.empty());
}

TEST_F(OptionalTests, TEArrowOperatorProperties) {
    auto r1 = Optional<int>{25519};
    EXPECT_EQ(&*r1, r1.operator->());
    constexpr auto SameTypeC1 = std::is_same_v<decltype(r1.operator->()), int*>;
    EXPECT_TRUE(SameTypeC1);

    auto const r2 = Optional<int>{25519};
    EXPECT_EQ(&*r2, r2.operator->());
    constexpr auto SameTypeC2 = std::is_same_v<decltype(r2.operator->()), int const*>;
    EXPECT_TRUE(SameTypeC2);

    auto r3 = Optional<std::string>{"12345"};
    EXPECT_EQ(r3->size(), 5ul);
}

TEST_F(OptionalTests, TEStarOperatorProperties) {
    auto r1 = Optional<int>{25519};
    constexpr auto SameTypeC1 = std::is_same_v<decltype(*r1), int&>;
    EXPECT_TRUE(SameTypeC1);
    *r1 = 1;
    EXPECT_EQ(r1.value(), 1);

    auto const r2 = Optional<int>{25519};
    constexpr auto SameTypeC2 = std::is_same_v<decltype(*r2), int const&>;
    EXPECT_TRUE(SameTypeC2);
    EXPECT_EQ(*r2, 25519);

    auto r3 = Optional<int>{25519};
    constexpr auto SameTypeC3 = std::is_same_v<decltype(*std::move(r3)), int&&>;
    EXPECT_TRUE(SameTypeC3);
    auto&& x3 = std::move(r3).operator*();
    EXPECT_EQ(x3, 25519);

    auto const r4 = Optional<int>{25519};
    constexpr auto SameTypeC4 = std::is_same_v<decltype(*std::move(r4)), int const&&>;
    EXPECT_TRUE(SameTypeC4);
    auto const&& x4 = std::move(r4).operator*();
    EXPECT_EQ(x4, 25519);
}

TEST_F(OptionalTests, TEOperatorBoolProperties) {
    auto r1 = Optional<int>{0};
    EXPECT_TRUE(static_cast<bool>(r1));

    auto r2 = Optional<int>{};
    EXPECT_FALSE(static_cast<bool>(r2));
}

TEST_F(OptionalTests, TEHasValueProperties) {
    auto r1 = Optional<int>{};
    EXPECT_FALSE(r1.hasValue());

    auto r2 = Optional<int>{InPlaceValue};
    EXPECT_TRUE(r2.hasValue());
}

TEST_F(OptionalTests, TEValueAccessorProperties) {
    auto r1 = Optional<int>{25519};
    constexpr auto SameTypeC1 = std::is_same_v<decltype(r1.value()), int&>;
    EXPECT_TRUE(SameTypeC1);
    r1.value() = 1;
    EXPECT_EQ(r1.value(), 1);

    auto const r2 = Optional<int>{25519};
    constexpr auto SameTypeC2 = std::is_same_v<decltype(r2.value()), int const&>;
    EXPECT_TRUE(SameTypeC2);
    EXPECT_EQ(r2.value(), 25519);

    auto r3 = Optional<int>{25519};
    constexpr auto SameTypeC3 = std::is_same_v<decltype(std::move(r3).value()), int&&>;
    EXPECT_TRUE(SameTypeC3);
    auto&& x3 = std::move(r3).value();
    EXPECT_EQ(x3, 25519);

    auto const r4 = Optional<int>{25519};
    constexpr auto SameTypeC4 = std::is_same_v<decltype(std::move(r4).value()), int const&&>;
    EXPECT_TRUE(SameTypeC4);
    auto const&& x4 = std::move(r4).value();
    EXPECT_EQ(x4, 25519);
}

TEST_F(OptionalTests, TEValueOrAccessProperties) {
    int const v1alt{25519};
    Optional<int> r1{1};
    auto const v1out = r1.valueOr(v1alt);
    EXPECT_EQ(v1out, 1);

    int const v2alt{25519};
    Optional<int> r2{};
    auto const v2out = r2.valueOr(v2alt);
    EXPECT_EQ(v2out, v2alt);

    Optional<MoveOnly<std::string>> r3{cstrValue1};
    auto const v3out = std::move(r3).valueOr("substitute");
    EXPECT_EQ(v3out, strValue1);
    EXPECT_TRUE(r3.hasValue());
    EXPECT_EQ(*r3, ""s);

    Optional<MoveOnly<std::string>> r4{};
    auto const v4out = std::move(r4).valueOr("substitute");
    EXPECT_EQ(v4out, "substitute"s);
    EXPECT_FALSE(r4.hasValue());
}

TEST_F(OptionalTests, TEFlatMapProperties) {
    Optional<int> r1a{25519};
    auto const r1b = r1a.flatMap([] (auto x) {
        return Optional<std::string>{fmt::format("ed{}", x)};
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, "ed25519"s);

    Optional<int> r2a{};
    auto const r2b = r2a.flatMap([] (auto x) {
        return Optional<std::string>{fmt::format("ed{}", x)};
    });
    EXPECT_FALSE(r2b.hasValue());
}

TEST_F(OptionalTests, TEMapProperties) {
    int const v1{25519};
    Optional<int> r1a{v1};
    auto const r1b = r1a.map([] (auto x) {
        return fmt::format("ed{}", x);
    });
    EXPECT_TRUE(r1b.hasValue());
    EXPECT_EQ(*r1b, "ed25519"s);
}

TEST_F(OptionalTests, TEReferencesConstructionProperties) {
    constexpr auto DefaultConstructibleC1 =
            std::is_default_constructible_v<Optional<int&>>;
    EXPECT_TRUE(DefaultConstructibleC1);

    int v1{1};
    Optional<int&> r1a{v1};
    auto r1b = r1a;
    EXPECT_EQ(&(*r1b), &v1);
    v1 = 25519;
    EXPECT_EQ(*r1a, 25519);
    EXPECT_EQ(*r1b, 25519);

    int v2{2};
    Optional<int&> r2a{};
    EXPECT_FALSE(r2a.hasValue());
    r2a = v2;
    EXPECT_TRUE(r2a.hasValue());
    EXPECT_EQ(&(*r2a), &v2);
    EXPECT_EQ(*r2a, 2);
    v2 = 5;
    EXPECT_EQ(*r2a, 5);

    constexpr auto CopyConstructibleC4 =
            std::is_copy_constructible_v<Optional<int&>>;
    EXPECT_TRUE(CopyConstructibleC4);
    constexpr auto MoveConstructibleC7 =
            std::is_move_constructible_v<Optional<int&>>;
    EXPECT_TRUE(MoveConstructibleC7);
    constexpr auto MoveConstructibleC8 =
            std::is_move_constructible_v<Optional<MoveOnly<std::string>>>;
    EXPECT_TRUE(MoveConstructibleC8);

    constexpr auto NotConstructibleFromC9 =
            not std::is_constructible_v<Optional<int&>, Optional<int> const&>;
    EXPECT_TRUE(NotConstructibleFromC9);

    constexpr auto ContructibleFromC10 =
            std::is_constructible_v<Optional<VValue&>, Optional<CValue&> const&>;
    EXPECT_TRUE(ContructibleFromC10);

    int v3{12345};
    auto cval3 = CValue{v3};
    Optional<CValue&> r3a{cval3};
    Optional<VValue&> r3b{r3a};
    EXPECT_EQ(&(*r3b), &cval3);
    EXPECT_EQ(r3b->getValue(), v3);

    constexpr auto ConstructibleFromC13 =
            std::is_constructible_v<
                    Optional<VValue&>,
                    Optional<CValue&>>;
    EXPECT_TRUE(ConstructibleFromC13);


    auto cval7 = CValue{25519};
    Optional<VValue&> r7{InPlaceValue, cval7};
    EXPECT_TRUE(r7.hasValue());
    EXPECT_EQ(&(*r7), &cval7);
    EXPECT_EQ(r7->getValue(), 25519);
}

TEST_F(OptionalTests, TEReferencesAssignabilityProperties) {
    auto v1a = CValue{1};
    auto v1b = CValue{25519};
    auto const r1a = Optional<VValue&>{v1a};
    auto r1b = Optional<VValue&>{v1b};
    EXPECT_EQ(&(*r1b), &v1b);
    EXPECT_EQ(r1b->getValue(), 25519);
    r1b = r1a;
    EXPECT_EQ(&(*r1b), &v1a);
    EXPECT_EQ(r1b->getValue(), 1);

    auto v3 = CValue{25519};
    Optional<VValue&> r3a{v3};
    Optional<VValue&> r3b{};
    EXPECT_FALSE(r3b.hasValue());
    r3b = r3a;
    EXPECT_TRUE(r3b.hasValue());
    EXPECT_EQ(&(*r3b), &v3);
    EXPECT_EQ(r3b->getValue(), 25519);

    auto v9a = CValue{999};
    auto v9b = CValue{1};
    Optional<CValue&> r9a{v9a};
    Optional<VValue&> r9b{v9b};
    EXPECT_TRUE(r9b.hasValue());
    EXPECT_EQ(&(*r9b), &v9b);
    EXPECT_EQ(r9b->getValue(), 1);
    r9b = r9a;
    EXPECT_EQ(&(*r9b), &v9a);
    EXPECT_EQ(r9b->getValue(), 999);

    auto v11 = CValue{25519};
    Optional<CValue&> r11a{v11};
    Optional<VValue&> r11b{};
    EXPECT_FALSE(r11b.hasValue());
    r11b = r11a;
    EXPECT_TRUE(r11b.hasValue());
    EXPECT_EQ(&(*r11b), &v11);
    EXPECT_EQ(r11b->getValue(), 25519);
    v11 = 91552;
    EXPECT_EQ(r11a->getValue(), 91552);
    EXPECT_EQ(r11b->getValue(), 91552);

    int v17a{25519};
    int v17b{12};
    auto r17 = Optional<int&>{v17a};
    EXPECT_TRUE(r17.hasValue());
    EXPECT_EQ(&(*r17), &v17a);
    EXPECT_EQ(*r17, 25519);
    r17 = v17b;
    EXPECT_EQ(&(*r17), &v17b);
    EXPECT_EQ(*r17, 12);
}

TEST_F(OptionalTests, TEReferenceAccessorProperties) {
    int v1{1};
    Optional<int&> r1{v1};
    constexpr auto IntRefTypeC1 = std::is_same_v<decltype(*r1), int&>;
    EXPECT_TRUE(IntRefTypeC1);
    EXPECT_EQ(&(*r1), &v1);

    int v2{2};
    Optional<int&> r2{v2};
    constexpr auto IntRefTypeC2 = std::is_same_v<decltype(r2.value()), int&>;
    EXPECT_TRUE(IntRefTypeC2);
    EXPECT_EQ(&(*r2), &v2);

    int v4{4};
    Optional<int&> const r4{v4};
    // *r4 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC4 = std::is_same_v<decltype(*r4), int&>;
    EXPECT_TRUE(IntRefTypeC4);
    EXPECT_EQ(&(*r4), &v4);

    int v5{5};
    Optional<int&> const r5{v5};
    // *r5 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC5 = std::is_same_v<decltype(r5.value()), int&>;
    EXPECT_TRUE(IntRefTypeC5);
    EXPECT_EQ(&(*r5), &v5);

    int v7a{7};
    Optional<int&> r7{v7a};
    constexpr auto IntRefTypeC7 = std::is_same_v<decltype(*std::move(r7)), int&>;
    EXPECT_TRUE(IntRefTypeC7);
    int& v7b = *std::move(r7);
    EXPECT_EQ(&v7a, &v7b);

    int v8a{8};
    Optional<int&> r8{v8a};
    constexpr auto IntRefTypeC8 = std::is_same_v<decltype(std::move(r8).value()), int&>;
    EXPECT_TRUE(IntRefTypeC8);
    int& v8b = *std::move(r8);
    EXPECT_EQ(&v8a, &v8b);

    int v10a{10};
    Optional<int&> const r10{v10a};
    // *r6 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC10 = std::is_same_v<decltype(*std::move(r10)), int&>;
    EXPECT_TRUE(IntRefTypeC10);
    int& v10b = *std::move(r10);
    EXPECT_EQ(&v10a, &v10b);

    int v11a{11};
    Optional<int&> const r11{v11a};
    // *r6 returns lvalue reference that does not propagate constness!
    constexpr auto IntRefTypeC11 = std::is_same_v<decltype(std::move(r11).value()), int&>;
    EXPECT_TRUE(IntRefTypeC11);
    int& v11b = *std::move(r11);
    EXPECT_EQ(&v11a, &v11b);
}

TEST_F(OptionalTests, TEEquals) {
    Optional<int> const lhs1{25519};
    Optional<long> const rhs1{25519};
    EXPECT_TRUE(lhs1 == rhs1);

    Optional<int> const lhs2{25519};
    Optional<long> const rhs2{0};
    EXPECT_FALSE(lhs2 == rhs2);


    Optional<int> const lhs5{25519};
    Optional<long> const rhs5{};
    EXPECT_FALSE(lhs5 == rhs5);

    Optional<int> const lhs6{};
    Optional<long> const rhs6{25519};
    EXPECT_FALSE(lhs6 == rhs6);
}

TEST_F(OptionalTests, TENotEquals) {
    Optional<int> const lhs1{25519};
    Optional<long> const rhs1{25519};
    EXPECT_FALSE(lhs1 != rhs1);

    Optional<int> const lhs2{25519};
    Optional<long> const rhs2{0};
    EXPECT_TRUE(lhs2 != rhs2);

    Optional<int> const lhs5{25519};
    Optional<long> const rhs5{};
    EXPECT_TRUE(lhs5 != rhs5);

    Optional<int> const lhs6{};
    Optional<long> const rhs6{25519};
    EXPECT_TRUE(lhs6 != rhs6);
}

TEST_F(OptionalTests, TELessThan) {
    Optional<int> const lhs1{1};
    Optional<long> const rhs1{100};
    EXPECT_TRUE(lhs1 < rhs1);

    Optional<int> const lhs2{100};
    Optional<long> const rhs2{1};
    EXPECT_FALSE(lhs2 < rhs2);

    Optional<int> const lhs3{};
    Optional<long> const rhs3{};
    EXPECT_FALSE(lhs3 < rhs3);

    Optional<int> const lhs5{1};
    Optional<long> const rhs5{};
    EXPECT_FALSE(lhs5 < rhs5);

    Optional<int> const lhs6{};
    Optional<long> const rhs6{1};
    EXPECT_TRUE(lhs6 < rhs6);

    Optional<int> const lhs7{1};
    Optional<long> const rhs7{1};
    EXPECT_FALSE(lhs7 < rhs7);
}

TEST_F(OptionalTests, TELessThanOrEqual) {
    Optional<int> const lhs1{1};
    Optional<long> const rhs1{100};
    EXPECT_TRUE(lhs1 <= rhs1);

    Optional<int> const lhs2{100};
    Optional<long> const rhs2{1};
    EXPECT_FALSE(lhs2 <= rhs2);

    Optional<int> const lhs3{};
    Optional<long> const rhs3{};
    EXPECT_TRUE(lhs3 <= rhs3);

    Optional<int> const lhs5{1};
    Optional<long> const rhs5{};
    EXPECT_FALSE(lhs5 <= rhs5);

    Optional<int> const lhs6{};
    Optional<long> const rhs6{1};
    EXPECT_TRUE(lhs6 <= rhs6);

    Optional<int> const lhs7{1};
    Optional<long> const rhs7{1};
    EXPECT_TRUE(lhs7 <= rhs7);
}

TEST_F(OptionalTests, TEGreaterThan) {
    Optional<int> const lhs1{1};
    Optional<long> const rhs1{100};
    EXPECT_FALSE(lhs1 > rhs1);

    Optional<int> const lhs2{100};
    Optional<long> const rhs2{1};
    EXPECT_TRUE(lhs2 > rhs2);

    Optional<int> const lhs3{};
    Optional<long> const rhs3{};
    EXPECT_FALSE(lhs3 > rhs3);

    Optional<int> const lhs5{1};
    Optional<long> const rhs5{};
    EXPECT_TRUE(lhs5 > rhs5);

    Optional<int> const lhs6{};
    Optional<long> const rhs6{1};
    EXPECT_FALSE(lhs6 > rhs6);

    Optional<int> const lhs7{1};
    Optional<long> const rhs7{1};
    EXPECT_FALSE(lhs7 > rhs7);
}

TEST_F(OptionalTests, TEGreaterThanOrEqual) {
    Optional<int> const lhs1{1};
    Optional<long> const rhs1{100};
    EXPECT_FALSE(lhs1 >= rhs1);

    Optional<int> const lhs2{100};
    Optional<long> const rhs2{1};
    EXPECT_TRUE(lhs2 >= rhs2);

    Optional<int> const lhs3{};
    Optional<long> const rhs3{};
    EXPECT_TRUE(lhs3 >= rhs3);

    Optional<int> const lhs5{1};
    Optional<long> const rhs5{};
    EXPECT_TRUE(lhs5 >= rhs5);

    Optional<int> const lhs6{};
    Optional<long> const rhs6{1};
    EXPECT_FALSE(lhs6 >= rhs6);

    Optional<int> const lhs7{1};
    Optional<long> const rhs7{1};
    EXPECT_TRUE(lhs7 >= rhs7);
}

TEST_F(OptionalTests, TEAgainstValueEquals) {
    Optional <int> lhs1{0};
    long const rhs1{0l};
    EXPECT_TRUE(lhs1 == rhs1);

    long const lhs2{25519l};
    Optional<int> rhs2{25519};
    EXPECT_TRUE(lhs2 == rhs2);

    Optional <int> lhs3{25519};
    long const rhs3{0l};
    EXPECT_FALSE(lhs3 == rhs3);

    long const lhs4{25519l};
    Optional<int> rhs4{0};
    EXPECT_FALSE(lhs4 == rhs4);

    Optional<int> lhs5{};
    long const rhs5{0};
    EXPECT_FALSE(lhs5 == rhs5);

    long const lhs6{0};
    Optional<int> rhs6{};
    EXPECT_FALSE(lhs6 == rhs6);
}

TEST_F(OptionalTests, TEAgainstValueNotEquals) {
    Optional <int> lhs1{0};
    long const rhs1{0l};
    EXPECT_FALSE(lhs1 != rhs1);

    long const lhs2{25519l};
    Optional<int> rhs2{25519};
    EXPECT_FALSE(lhs2 != rhs2);

    Optional <int> lhs3{25519};
    long const rhs3{0l};
    EXPECT_TRUE(lhs3 != rhs3);

    long const lhs4{25519l};
    Optional<int> rhs4{0};
    EXPECT_TRUE(lhs4 != rhs4);

    Optional<int> lhs5{};
    long const rhs5{0};
    EXPECT_TRUE(lhs5 != rhs5);

    long const lhs6{0};
    Optional<int> rhs6{};
    EXPECT_TRUE(lhs6 != rhs6);
}

TEST_F(OptionalTests, TEAgainstValueLessThan) {
    Optional<int> const lhs1{1};
    long const rhs1{100};
    EXPECT_TRUE(lhs1 < rhs1);

    Optional<int> const lhs2{100};
    long const rhs2{1};
    EXPECT_FALSE(lhs2 < rhs2);

    long const lhs3{1};
    Optional<int> const rhs3{100};
    EXPECT_TRUE(lhs3 < rhs3);

    long const lhs4{100};
    Optional<int> const rhs4{1};
    EXPECT_FALSE(lhs4 < rhs4);

    Optional<int> const lhs5{};
    long const rhs5{1};
    EXPECT_TRUE(lhs5 < rhs5);

    long const lhs6{1};
    Optional<int> const rhs6{};
    EXPECT_FALSE(lhs6 < rhs6);

    Optional<int> const lhs7{1};
    long const rhs7{1};
    EXPECT_FALSE(lhs7 < rhs7);

    long const rhs8{1};
    Optional<int> const lhs8{1};
    EXPECT_FALSE(lhs8 < rhs8);
}

TEST_F(OptionalTests, TEAgainstValueLessThanOrEqual) {
    Optional<int> const lhs1{1};
    long const rhs1{100};
    EXPECT_TRUE(lhs1 <= rhs1);

    Optional<int> const lhs2{100};
    long const rhs2{1};
    EXPECT_FALSE(lhs2 <= rhs2);

    long const lhs3{1};
    Optional<int> const rhs3{100};
    EXPECT_TRUE(lhs3 <= rhs3);

    long const lhs4{100};
    Optional<int> const rhs4{1};
    EXPECT_FALSE(lhs4 <= rhs4);

    Optional<int> const lhs5{};
    long const rhs5{1};
    EXPECT_TRUE(lhs5 <= rhs5);

    long const lhs6{1};
    Optional<int> const rhs6{};
    EXPECT_FALSE(lhs6 <= rhs6);

    Optional<int> const lhs7{1};
    long const rhs7{1};
    EXPECT_TRUE(lhs7 <= rhs7);

    long const rhs8{1};
    Optional<int> const lhs8{1};
    EXPECT_TRUE(lhs8 <= rhs8);
}


TEST_F(OptionalTests, TEAgainstValueGreaterThan) {
    Optional<int> const lhs1{1};
    long const rhs1{100};
    EXPECT_FALSE(lhs1 > rhs1);

    Optional<int> const lhs2{100};
    long const rhs2{1};
    EXPECT_TRUE(lhs2 > rhs2);

    long const lhs3{1};
    Optional<int> const rhs3{100};
    EXPECT_FALSE(lhs3 > rhs3);

    long const lhs4{100};
    Optional<int> const rhs4{1};
    EXPECT_TRUE(lhs4 > rhs4);

    Optional<int> const lhs5{};
    long const rhs5{1};
    EXPECT_FALSE(lhs5 > rhs5);

    long const lhs6{1};
    Optional<int> const rhs6{};
    EXPECT_TRUE(lhs6 > rhs6);

    Optional<int> const lhs7{1};
    long const rhs7{1};
    EXPECT_FALSE(lhs7 > rhs7);

    long const rhs8{1};
    Optional<int> const lhs8{1};
    EXPECT_FALSE(lhs8 > rhs8);
}

TEST_F(OptionalTests, TEAgainstValueGreaterThanOrEqual) {
    Optional<int> const lhs1{1};
    long const rhs1{100};
    EXPECT_FALSE(lhs1 >= rhs1);

    Optional<int> const lhs2{100};
    long const rhs2{1};
    EXPECT_TRUE(lhs2 >= rhs2);

    long const lhs3{1};
    Optional<int> const rhs3{100};
    EXPECT_FALSE(lhs3 >= rhs3);

    long const lhs4{100};
    Optional<int> const rhs4{1};
    EXPECT_TRUE(lhs4 >= rhs4);

    Optional<int> const lhs5{};
    long const rhs5{1};
    EXPECT_FALSE(lhs5 >= rhs5);

    long const lhs6{1};
    Optional<int> const rhs6{};
    EXPECT_TRUE(lhs6 >= rhs6);

    Optional<int> const lhs7{1};
    long const rhs7{1};
    EXPECT_TRUE(lhs7 >= rhs7);

    long const rhs8{1};
    Optional<int> const lhs8{1};
    EXPECT_TRUE(lhs8 >= rhs8);
}

TEST_F(OptionalTests, Swap) {
    Optional<std::string> lhs1{"abc"};
    Optional<std::string> rhs1{"xyz"};
    swap(lhs1, rhs1);
    EXPECT_EQ(*lhs1, "xyz"s);
    EXPECT_EQ(*rhs1, "abc"s);

    Optional<std::string> lhs2{"abc"};
    Optional<std::string> rhs2{};
    swap(lhs2, rhs2);
    ASSERT_FALSE(lhs2.hasValue());
    ASSERT_TRUE(rhs2.hasValue());
    EXPECT_EQ(*rhs2, "abc"s);

    Optional<std::string> lhs3{};
    Optional<std::string> rhs3{"xyz"};
    swap(lhs3, rhs3);
    ASSERT_TRUE(lhs3.hasValue());
    ASSERT_FALSE(rhs3.hasValue());
    EXPECT_EQ(lhs3, "xyz"s);

    Optional<std::string> lhs4{};
    Optional<std::string> rhs4{};
    swap(lhs4, rhs4);
    ASSERT_FALSE(lhs4.hasValue());
    ASSERT_FALSE(rhs4.hasValue());
}

// LINE 3257

} // namespace pimc::testing
