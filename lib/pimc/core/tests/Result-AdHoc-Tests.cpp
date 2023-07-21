#include <cstdint>

#include <gtest/gtest.h>
#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Result.hpp"

namespace pimc::testing {

namespace {
class MoveOnlyTracker {
public:

    explicit MoveOnlyTracker(uint64_t& v) : v_{&v} {}

    MoveOnlyTracker(MoveOnlyTracker const &) = delete;

    MoveOnlyTracker(MoveOnlyTracker&& rhs) noexcept
    : v_{rhs.v_} {
        rhs.v_ = nullptr;
        ++*v_;
    }

    auto operator=(MoveOnlyTracker const &)
            -> MoveOnlyTracker & = delete;

    auto operator=(MoveOnlyTracker&& rhs)
            noexcept -> MoveOnlyTracker & {
        if (&rhs == this) return *this;
        v_ = rhs.v_;

        *v_ += (1ul << 16u);

        rhs.v_ = nullptr;
        return *this;
    }

    ~MoveOnlyTracker() {
        if (v_ != nullptr) *v_ += (1ul << 32u);

    }

private:
    uint64_t *v_;
};

uint64_t countMovesCtors(u_int64_t v) {
    return v & 0xFFFF;
}

uint64_t countMoveAssigns(uint64_t v) {
    return (v >> 16u) & 0xFFFF;
}

uint64_t countDestrs(uint64_t v) {
    return (v >> 32u) & 0xFFFF;
}

} // anon.namespace

class ResultAdhocTests: public ::testing::Test {
protected:

    static Result<void, MoveOnlyTracker> checkValue(int x, uint64_t& v) {
        if (x == 1) {
            return fail(MoveOnlyTracker{v});
        }

        return {};
    }

    static void enasureResult(Result<void, MoveOnlyTracker>&& r) {
        auto rr = std::move(r);
        if (not rr)
            throw std::runtime_error{""};
    }
};

TEST_F(ResultAdhocTests, CountMovesAssignsDestrs) {
    uint64_t v{0};
    bool excpt{false};

    auto r = checkValue(1, v);
    if (not r) {
        try {
            enasureResult(std::move(r));
        } catch (std::runtime_error const&){
            excpt = true;
        }
    }

    EXPECT_TRUE(excpt);
    EXPECT_EQ(countMovesCtors(v), 3);
    EXPECT_EQ(countMoveAssigns(v), 0);
    EXPECT_EQ(countDestrs(v), 1);
}

} // namespace pimc::testing
