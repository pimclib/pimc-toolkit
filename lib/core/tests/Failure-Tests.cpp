#include <utility>
#include <type_traits>
#include <string>

#include <gtest/gtest.h>

#include "pimc/core/Result.hpp"

using namespace std::string_literals;

namespace pimc::testing {

namespace {

struct NotNoThrowDfltConstructible {
    NotNoThrowDfltConstructible() noexcept(false) {}
};

struct NotTrivialDfltConstructible {
    NotTrivialDfltConstructible() {}
};

struct NotDfltConstructible {
    NotDfltConstructible() = delete;
};

struct NotCopyOrMoveable {
    NotCopyOrMoveable() = default;
    NotCopyOrMoveable(NotCopyOrMoveable const&) = delete;
    NotCopyOrMoveable(NotCopyOrMoveable&&) = delete;
};

template <typename T>
struct ExplicitType: public T {
    template <typename ... Ts>
    requires std::constructible_from<T, Ts...>
    explicit ExplicitType(Ts&& ... args)
    : T{std::forward<Ts>(args)...} {}

    explicit ExplicitType() = default;
    explicit ExplicitType(ExplicitType const&) = default;
    explicit ExplicitType(ExplicitType&&) = default;

    auto operator= (ExplicitType const&) -> ExplicitType& = default;
    auto operator= (ExplicitType&&) -> ExplicitType& = default;
};

template <typename T>
struct MoveOnly: public T {
    MoveOnly() = default;
    MoveOnly(T&& t): T{std::move(t)} {}
    MoveOnly(MoveOnly const&) = delete;
    MoveOnly(MoveOnly&&) = default;

    auto operator= (MoveOnly const&) -> MoveOnly& = delete;
    auto operator= (MoveOnly&&) -> MoveOnly& = default;
};

class DefaultConstructValue {
public:
    inline static constexpr int DefaultValue{123};

    DefaultConstructValue(): value_{DefaultValue} {}

    [[nodiscard]]
    int value() const { return value_; }

private:
    int value_;
};

} // anon.namespace

class FailureTests: public ::testing::Test {
protected:

    char const* cstrValue1 = "ed25519 is a very nice elliptic curve";
    std::string strValue1 = "ed25519 is a very nice elliptic curve"s;
};

TEST_F(FailureTests, BasicConstructionProperties) {
    EXPECT_TRUE(std::is_default_constructible_v<Failure<int>>);
    EXPECT_TRUE(std::is_nothrow_default_constructible_v<Failure<int>>);
    EXPECT_FALSE(std::is_nothrow_default_constructible_v<Failure<NotNoThrowDfltConstructible>>);

    EXPECT_TRUE(std::is_trivially_default_constructible_v<Failure<int>>);

    EXPECT_FALSE(std::is_trivially_default_constructible_v<Failure<NotTrivialDfltConstructible>>);
    EXPECT_TRUE(std::is_default_constructible_v<Failure<NotTrivialDfltConstructible>>);

    Failure<DefaultConstructValue> f;
    EXPECT_EQ(f.error().value(), DefaultConstructValue::DefaultValue);

    EXPECT_FALSE(std::is_default_constructible_v<Failure<NotDfltConstructible>>);
}

TEST_F(FailureTests, CopyConstructionProperties) {
    auto constexpr CopyConstructible = std::is_constructible_v<Failure<int>, int const&>;
    EXPECT_TRUE(CopyConstructible);

    auto constexpr NotCopyConstructible =
            not std::is_constructible_v<Failure<NotCopyOrMoveable>, NotCopyOrMoveable const&>;
    EXPECT_TRUE(NotCopyConstructible);

    auto const f1 = Failure<int>{25519};
    auto const f2 = f1;
    EXPECT_EQ(f1, f2);
    EXPECT_EQ(f1.error(), f2.error());

    auto fc = [] <typename T> (T const& tx) -> Failure<T> {
        return Failure<T>{tx};
    };
    std::string msg{strValue1};
    auto fs1 = fc(msg);
    EXPECT_EQ(fs1.error(), strValue1);
}

TEST_F(FailureTests, MoveConstructionProperties) {
    auto constexpr MoveConstructible =
            std::is_constructible_v<Failure<std::string>, std::string&&>;
    EXPECT_TRUE(MoveConstructible);

    auto constexpr NotMoveConstructible =
            not std::is_constructible_v<Failure<NotCopyOrMoveable>, NotCopyOrMoveable&&>;
    EXPECT_TRUE(NotMoveConstructible);

    auto msg1 = strValue1;
    auto f1 = fail(std::move(msg1));
    EXPECT_TRUE(msg1.empty());
    EXPECT_EQ(f1.error(), strValue1);
}

TEST_F(FailureTests, ConstructionInPlace) {
    const auto s1 = "1234567890"s;
    Failure<std::string> f1{std::in_place, s1, 4ul};
    EXPECT_EQ(f1.error(), "567890"s);

    Failure<std::string> f2{
        std::in_place,
        {'E', 'D', '2', '5', '5', '1', '9'},
        std::allocator<char>{}
    };
    char const* exp2 = "ED25519";
    EXPECT_STREQ(f2.error().c_str(), exp2);
}

TEST_F(FailureTests, ConstructionMisc) {
    using T1 = const char*;

    auto constexpr Convertible =
            std::is_convertible_v<const T1&, Failure<std::string>>;
    EXPECT_TRUE(Convertible);

    auto constexpr Constructible =
            std::is_constructible_v<Failure<std::string>, const T1&>;
    EXPECT_TRUE(Constructible);

    Failure<std::string> const f1 = cstrValue1;

    EXPECT_EQ(f1.error(), strValue1);
    auto NotConvertible =
            not std::is_convertible_v<const int&, Failure<std::string>>;
    EXPECT_TRUE(NotConvertible);
}

TEST_F(FailureTests, ConstructionExplicit) {
    using T1 = const char*;
    using D1 = Failure<ExplicitType<std::string>>;

    auto constexpr Constructible =
            std::is_constructible_v<D1, const T1&>;
    EXPECT_TRUE(Constructible);

    auto constexpr NotConvertible =
            not std::is_convertible_v<const T1&, D1>;
    EXPECT_TRUE(NotConvertible);

    auto const f1 = D1{cstrValue1};
    ExplicitType<std::string> const e1{strValue1};
    EXPECT_EQ(f1.error(), e1);

    auto constexpr NotConstructible =
            not std::is_constructible_v<D1, const int&>;
    EXPECT_TRUE(NotConstructible);
}

TEST_F(FailureTests, CopyConversionProperties) {
    auto constexpr CopyConstructible =
            std::is_constructible_v<Failure<std::string>, Failure<char const*>>;
    EXPECT_TRUE(CopyConstructible);

    auto const f1 = fail(cstrValue1);
    auto s1 = strValue1;
    auto const f2 = Failure<std::string>{std::move(s1)};
    EXPECT_EQ(f1, f2);

    auto constexpr NotCopyConstructible =
            not std::is_constructible_v<Failure<int>, Failure<std::string>>;
    EXPECT_TRUE(NotCopyConstructible);
}

TEST_F(FailureTests, MoveConversionProperties) {
    auto constexpr MoveConstructible =
            std::is_constructible_v<Failure<MoveOnly<std::string>>, Failure<std::string>&&>;
    EXPECT_TRUE(MoveConstructible);

    auto s1 = strValue1;
    auto f1 = fail(std::move(s1));
    auto const fe = f1;
    auto const f2 = Failure<MoveOnly<std::string>>{std::move(f1)};
    EXPECT_EQ(f2, fe);

    auto constexpr NotMoveConvertible =
            not std::is_constructible_v<Failure<MoveOnly<std::string>>, Failure<int>&&>;
    EXPECT_TRUE(NotMoveConvertible);
}

TEST_F(FailureTests, ErrorAssignmentProperties) {
    auto constexpr Assignable =
            std::is_assignable_v<Failure<std::string>, char const*>;
    EXPECT_TRUE(Assignable);

    auto f1 = fail<std::string>("RSA"s);

    f1 = cstrValue1;
    EXPECT_EQ(f1.error(), strValue1);

    auto constexpr NotAssignable =
            not std::is_assignable_v<Failure<int>, char const*>;
    EXPECT_TRUE(NotAssignable);
}

TEST_F(FailureTests, FailureAssignmentProperties) {
    auto constexpr Assignable =
            std::is_assignable_v<Failure<std::string>, Failure<char const*>>;
    EXPECT_TRUE(Assignable);

    auto const fe = fail(cstrValue1);
    auto f1 = fail<std::string>("RSA");
    f1 = fe;
    EXPECT_EQ(f1, fe);

    auto constexpr NotAssignable =
            not std::is_assignable_v<Failure<unsigned>, Failure<char const*>>;
    EXPECT_TRUE(NotAssignable);
}

TEST_F(FailureTests, FailureMoveAssignableProperties) {
    auto constexpr Assignable =
            std::is_assignable_v<Failure<MoveOnly<std::string>>, Failure<std::string>&&>;
    EXPECT_TRUE(Assignable);

    auto f1 = fail(strValue1);
    auto const fe = f1;
    auto f2 = Failure<MoveOnly<std::string>>{"RSA"s};
    f2 = std::move(f1);
    EXPECT_EQ(f2, fe);
    EXPECT_TRUE(f1.error().empty());

    auto constexpr NotAssignable =
            not std::is_assignable_v<Failure<MoveOnly<std::string>>, Failure<bool>>;
    EXPECT_TRUE(NotAssignable);
}

TEST_F(FailureTests, ErrorValueAccess) {
    auto f1 = Failure<unsigned>{25519u};
    auto constexpr DeclTypeLValueRef =
            std::is_same_v<decltype(f1.error()), unsigned&>;
    EXPECT_TRUE(DeclTypeLValueRef);

    auto f2 = Failure<unsigned>{25519u};
    auto constexpr DeclTypeRValueRef =
            std::is_same_v<decltype(std::move(f2).error()), unsigned&&>;
    EXPECT_TRUE(DeclTypeRValueRef);

    auto const f3 = Failure<unsigned>{25519u};
    auto constexpr DeclTypeLValueConstRef =
            std::is_same_v<decltype(f3.error()), unsigned const&>;
    EXPECT_TRUE(DeclTypeLValueConstRef);

    auto const f4 = Failure<unsigned>{25519u};
    auto constexpr DeclTypeRValueConstRef =
            std::is_same_v<decltype(std::move(f4).error()), unsigned const&&>;
    EXPECT_TRUE(DeclTypeRValueConstRef);
}

TEST_F(FailureTests, FailureOverRefConstructability) {
    auto constexpr FailureOverRefNotDefaultConstructible =
            not std::is_default_constructible_v<Failure<int&>>;
    EXPECT_TRUE(FailureOverRefNotDefaultConstructible);

    auto constexpr FailureOverRefNotConstructibleFromConstLValueConstRef =
            not std::is_constructible_v<Failure<int&>, const int&>;
    EXPECT_TRUE(FailureOverRefNotConstructibleFromConstLValueConstRef);

    auto constexpr FailureOverRefNotConstructibleFromConstRValueRef =
            not std::is_constructible_v<Failure<int&>, int&&>;
    EXPECT_TRUE(FailureOverRefNotConstructibleFromConstRValueRef);

    auto constexpr FailureOverRefConstructibleFromLValueRef =
            std::is_constructible_v<Failure<int&>, int&>;
    EXPECT_TRUE(FailureOverRefConstructibleFromLValueRef);

    int v = 100;
    Failure<int&> f{v};
    v = 25519;
    EXPECT_EQ(f.error(), 25519);
    EXPECT_EQ(&f.error(), &v);
}

TEST_F(FailureTests, ErrorRefAccess) {
    unsigned v1{25519u};
    Failure<unsigned&> f1{v1};
    auto constexpr FailureOverRefAccessedAsLValueRef =
            std::is_same_v<decltype(f1.error()), unsigned&>;
    EXPECT_TRUE(FailureOverRefAccessedAsLValueRef);
    EXPECT_EQ(&f1.error(), &v1);

    unsigned v2{25519u};
    Failure<unsigned&> f2{v2};
    auto constexpr FailureOverRefAccessedAsRValueRefIsLValueRef =
            std::is_same_v<decltype(std::move(f2).error()), unsigned&>;
    EXPECT_TRUE(FailureOverRefAccessedAsRValueRefIsLValueRef);
    auto& v2x = std::move(f2).error();
    EXPECT_EQ(&v2, &v2x);

    unsigned v3{25519u};
    Failure<unsigned&> const f3{v3};
    auto constexpr ConstFailureOverRefAccessedAsLValueRef =
            std::is_same_v<decltype(f3.error()), unsigned&>;
    EXPECT_TRUE(ConstFailureOverRefAccessedAsLValueRef);
    EXPECT_EQ(&f3.error(), &v3);

    unsigned v4{25519u};
    Failure<unsigned&> const f4{v4};
    auto constexpr ConstFailureOverRefAccessedAsRValueRefIsLValueRef =
            std::is_same_v<decltype(std::move(f4).error()), unsigned&>;
    EXPECT_TRUE(ConstFailureOverRefAccessedAsRValueRefIsLValueRef);
    auto& v4x = std::move(f4).error();
    EXPECT_EQ(&v4, &v4x);
}

TEST_F(FailureTests, FailureRefComparisonProperties) {
    unsigned const v{25519u};
    auto const f1l = Failure<unsigned>{v};
    auto const f1r = fail(std::ref(v));
    EXPECT_TRUE(f1l == f1r);

    auto const f2l = Failure<unsigned>{0u};
    auto const f2r = fail(std::ref(v));
    EXPECT_TRUE(f2l != f2r);
    EXPECT_TRUE(f2l <= f2r);
    EXPECT_TRUE(f2l < f2r);

    auto const f3l = Failure<unsigned>{91552u};
    auto const f3r = fail(std::ref(v));
    EXPECT_TRUE(f3l >= f3r);
    EXPECT_TRUE(f3l > f3r);
}

TEST_F(FailureTests, Misc) {
    std::string const s1{strValue1};
    auto f1 = fail(s1);
    auto constexpr DecayedTypeDeduction =
            std::is_same_v<decltype(f1), Failure<std::string>>;
    EXPECT_TRUE(DecayedTypeDeduction);

    auto const e2 = std::string{cstrValue1, 5ul};
    auto const f2 = fail<std::string>(cstrValue1, 5ul);
    EXPECT_EQ(f2.error(), e2);

    auto const e3 = std::string{
            {'E', 'D', '2', '5', '5', '1', '9'},
            std::allocator<char>{}
    };
    auto const f3 = fail<std::string>(
            {'E', 'D', '2', '5', '5', '1', '9'},
            std::allocator<char>{});
    EXPECT_EQ(f3.error(), e3);

    auto e4 = std::string{strValue1};
    auto f4 = fail(std::ref(e4));
    auto constexpr RefTypeDeduction =
            std::is_same_v<decltype(f4), Failure<std::string&>>;
    EXPECT_TRUE(RefTypeDeduction);
    EXPECT_EQ(f4.error(), e4);
    EXPECT_EQ(&f4.error(), &e4);
}

TEST_F(FailureTests, Swap) {
    std::string const e1{strValue1};
    std::string const e2{"RSA"s};
    auto f1 = fail(e1);
    auto f2 = fail(e2);

    swap(f1, f2);
    EXPECT_EQ(f1.error(), e2);
    EXPECT_EQ(f2.error(), e1);
}

}