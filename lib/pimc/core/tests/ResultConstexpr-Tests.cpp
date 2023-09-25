#include <type_traits>

#include <gtest/gtest.h>

#include "pimc/core/Result.hpp"

using namespace std::string_literals;

namespace pimc::testing {

namespace {

template <typename T>
constexpr auto adhocMove(T&& v) -> T { return static_cast<T&&>(v); }

class ConstExpr {
public:
    constexpr ConstExpr() noexcept = default;
    constexpr ConstExpr(ConstExpr const&) noexcept = default;
    constexpr ConstExpr(ConstExpr&&) noexcept = default;
    constexpr ConstExpr(int value) noexcept: value_{value} {}

    auto operator= (ConstExpr const&) noexcept -> ConstExpr& = default;
    auto operator= (ConstExpr&&) noexcept -> ConstExpr& = default;
    auto operator= (int v) noexcept -> ConstExpr& {
        value_ = v;
        return *this;
    }

    [[nodiscard]]
    constexpr int value() const noexcept { return value_; }
private:
    int value_;
};

constexpr auto operator== (ConstExpr const& lhs, ConstExpr const& rhs) noexcept -> bool {
    return lhs.value() == rhs.value();
}

using CE2Result = Result<ConstExpr, ConstExpr>;

static_assert(std::is_trivially_copyable_v<CE2Result>);
static_assert(std::is_trivially_destructible_v<CE2Result>);

} // anon.namespace

class ResultConstexprTests: public ::testing::Test {
protected:
};

TEST_F(ResultConstexprTests, DefaultConstruction) {
    constexpr CE2Result r1{};

    EXPECT_TRUE(r1.hasValue());
    EXPECT_FALSE(r1.hasError());
}

TEST_F(ResultConstexprTests, CopyConstruction) {
    constexpr CE2Result r1{};
    constexpr CE2Result r2{r1};

    EXPECT_TRUE(r2.hasValue());
    EXPECT_FALSE(r2.hasError());
}

TEST_F(ResultConstexprTests, MoveConstruction) {
    constexpr CE2Result r1{};
    constexpr CE2Result r2{adhocMove(r1)};

    EXPECT_TRUE(r1.hasValue());
    EXPECT_TRUE(r2.hasValue());
}

TEST_F(ResultConstexprTests, InPlaceValueConstruction) {
    constexpr CE2Result r1{InPlaceValue, 25519};

    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(r1.value().value(), 25519);
}

TEST_F(ResultConstexprTests, InPlaceErrorConstruction) {
    constexpr CE2Result r1{InPlaceError, 25519};

    EXPECT_FALSE(r1.hasValue());
    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error().value(), 25519);
}

TEST_F(ResultConstexprTests, ImplicitInPlaceValueConstruction) {
    constexpr CE2Result r1 = 25519;

    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(r1.value().value(), 25519);
}

TEST_F(ResultConstexprTests, ExplicitInPlaceValueConstruction) {
    constexpr CE2Result r1{25519};

    EXPECT_TRUE(r1.hasValue());
    EXPECT_EQ(r1.value().value(), 25519);
}

TEST_F(ResultConstexprTests, FromFailureConstruction) {
    constexpr Failure<int> f{25519};
    constexpr CE2Result r1{f};

    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error().value(), 25519);
}

TEST_F(ResultConstexprTests, ResultVoidDefaultConstruction) {
    constexpr Result<void, int> r1{};

    EXPECT_TRUE(r1.hasValue());
    EXPECT_FALSE(r1.hasError());
}

TEST_F(ResultConstexprTests, ResultVoidCopyConstruction) {
    constexpr Result<void, int> r1{};
    constexpr Result<void, int> r2{r1};

    EXPECT_TRUE(r2.hasValue());
}

TEST_F(ResultConstexprTests, ResultVoidMoveConstruction) {
    constexpr Result<void, int> r1{};
    constexpr Result<void, int> r2{adhocMove(r1)};

    EXPECT_TRUE(r1.hasValue());
    EXPECT_TRUE(r2.hasValue());
}

TEST_F(ResultConstexprTests, ResultVoidInPlaceErrorConstruction) {
    constexpr Result<void, int> r1{InPlaceError, 25519};

    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), 25519);
}

TEST_F(ResultConstexprTests, ResultVoidFromFailureConstruction) {
    constexpr Failure<int> f{25519};
    constexpr Result<void, int> r1{f};

    EXPECT_TRUE(r1.hasError());
    EXPECT_EQ(r1.error(), 25519);
}

} // namespace pimc::testing