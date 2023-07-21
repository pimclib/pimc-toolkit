#include <stdexcept>

#include <gtest/gtest.h>

#include "pimc/core/Deferred.hpp"

namespace pimc::testing {

namespace {

enum class ExStatus: int {
    Init = 0,
    Thrown = 1,
    NotThrown = -1,
};

} // anon.namespace

class DeferredTests: public ::testing::Test {
protected:
    bool called;
    ExStatus thrown;

    void reset() {
        called = false;
        thrown = ExStatus::Init;
    }

    void onEx() { thrown = ExStatus::Thrown; }

    void onNoEx() { thrown = ExStatus::NotThrown; }

    void condThrow(int a, int b, int c) {
        auto d = defer([this] { called = true; });

        if (a > 0) {
            if (b > c)
                throw std::runtime_error{"a > 0 and b > c"};

            if (c < 0)
                throw std::runtime_error("a > 0 and c < 0");
        } else {
            if (b > c)
                d.cancel();
        }
    }

    int condReturn(int a, int b, int c) {
        auto d = defer([this] { called = true; });

        switch (a) {
        case 10:
            switch (b) {
            case 1:
                if (c > 0)
                    d.cancel();
                else
                    return 1;
                break;
            case 2:
                if (c > 0)
                    d.cancel();
                else
                    return 2;
                break;
            case 3:
                if (c > 0)
                    d.cancel();
                else return 3;
                break;
            default:
                return -c;
            }
            break;

        case 20:
            switch (b) {
            case 1:
                if (c < 0)
                    d.cancel();
                else
                    return 1;
                break;
            case 2:
                if (c < 0)
                    d.cancel();
                else
                    return 2;
                break;
            case 3:
                if (c < 0)
                    d.cancel();
                else return 3;
                break;
            default:
                return c;
            }
            break;

        default:
            return 0;
        }

        return a;
    }
};

TEST_F(DeferredTests, OnException) {
    reset();

    EXPECT_FALSE(called);
    try {
        condThrow(1, 10, 5);
        onNoEx();
    } catch (std::runtime_error&) {
        onEx();
    }
    EXPECT_EQ(thrown, ExStatus::Thrown);
    EXPECT_TRUE(called);

    reset();
    EXPECT_FALSE(called);
    try {
        condThrow(-1, 0, 1);
        onNoEx();
    } catch (std::runtime_error&) {
        onEx();
    }
    EXPECT_EQ(thrown, ExStatus::NotThrown);
    EXPECT_TRUE(called);

    reset();
    EXPECT_FALSE(called);
    try {
        condThrow(-1, 1, 0);
        onNoEx();
    } catch (std::runtime_error&) {
        onEx();
    }
    EXPECT_EQ(thrown, ExStatus::NotThrown);
    EXPECT_FALSE(called);
}

TEST_F(DeferredTests, OnReturn) {
    reset();
    condReturn(10, 1, -1);
    EXPECT_TRUE(called);

    reset();
    condReturn(10, 3, 1);
    EXPECT_FALSE(called);

    reset();
    condReturn(20, 3, -1);
    EXPECT_FALSE(called);

    reset();
    condReturn(20, 3, 1);
    EXPECT_TRUE(called);
}

} // namespace pimc::testing