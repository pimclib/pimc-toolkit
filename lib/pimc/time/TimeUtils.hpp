#pragma once

#include <cstdint>
#include <ctime>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

/*!
 * Nanoseconds in a second
 */
constexpr uint64_t NanosInSecond{1'000'000'000};

/*!
 * Returns the host time in nanoseconds since the epoch, i.e.
 * 1970-01-01 00:00.0 UTC
 *
 * @return the nanoseconds since the epoch
 */
PIMC_ALWAYS_INLINE
uint64_t gethostnanos() {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * NanosInSecond
         + static_cast<uint64_t>(ts.tv_nsec);
}

} // namespace pimc
