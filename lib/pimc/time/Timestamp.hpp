#pragma once

#include <cstdint>

namespace pimc {

struct Timestamp {
    uint64_t value;
};

template <typename T>
struct TimeValue {
    uint64_t value;
};

} // namespace pimc
