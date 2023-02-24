#pragma once

#include <cstdint>

namespace pimc {

constexpr uint64_t MclstMagic{11899030981529723792ul};

struct MclstBeaconHdr final {
    uint64_t magic;
    uint64_t seq;
    uint64_t timeNs;
    uint16_t dataLen;
} __attribute__((__aligned__(1), __packed__));

} // namespace pimc
