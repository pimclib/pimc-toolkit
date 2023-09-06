#pragma once

#include <cstdint>

#include "pimc/core/CompilerUtils.hpp"
#include "pimc/time/TimeUtils.hpp"

namespace pimc {

class Timer final {
public:
    Timer(): ts_{gethostnanos()} {}

    PIMC_ALWAYS_INLINE
    void update() {
        ts_ = gethostnanos();
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint64_t inSec(uint64_t seconds) const {
        return ts_ + (NanosInSecond * seconds);
    }
private:
    uint64_t ts_;
};

} // namespace pimc
