#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>

#include "pimc/core/Endian.hpp"

namespace pimc {

constexpr uint8_t IPv4_FAMILY_NUMBER{1u};
constexpr uint8_t IPv6_FAMILY_NUMBER{2u};

constexpr uint8_t PIMSMv2_NATIVE_ENCODING{0u};

// PIM Message types
constexpr uint8_t PIMSMv2_HELLO{0u};
constexpr uint8_t PIMSMv2_REGISTER{1u};
constexpr uint8_t PIMSMv2_REGISTER_STOP{2u};
constexpr uint8_t PIMSMv2_JOIN_PRUNE{3u};
constexpr uint8_t PIMSMv2_BOOTSTRAP{4u};
constexpr uint8_t PIMSMv2_ASSERT{5u};
constexpr uint8_t PIMSMv2_GRAFT{6u};
constexpr uint8_t PIMSMv2_GRAFT_ACK{7u};
constexpr uint8_t PIMSMv2_CANDIDATE_RP_ADVERTISEMENT{8u};

constexpr uint16_t PIMSMv2_OPT_HOLDTIME{1u};
constexpr uint16_t PIMSMv2_OPT_LAN_PRUNE_DELAY{2u};
constexpr uint16_t PIMSMv2_OPT_DR_PRIORITY{19u};
constexpr uint16_t PIMSMv2_OPT_GENERATION_ID{20u};
constexpr uint16_t PIMSMv2_OPT_ADDRESS_LIST{24u};

/*!
 * The PIM Sparse Mode v2 packet header.
 */
struct PIMSMv2Hdr final {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t Type:4;
    uint8_t Version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t Version:4,
    uint8_t Type:4;
#else
#error  "Please fix pimc/core/Endian.hpp"
#endif
    uint8_t Reserved;
    uint16_t Checksum;
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2Hdr) == 4u);

/*!
 * The PIM SM v2 "Encoded Unicast Address"
 *
 * Per <a href="https://datatracker.ietf.org/doc/html/rfc7761.html#section-4.9.1">
 * RFC7761 4.9.1.  Encoded Source and Group Address Formats</a>
 */
struct PIMSMv2EncUAddr final {
    uint8_t Family;
    uint8_t EncodingType;
    // + variable length unicast address
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2EncUAddr) == 2u);

constexpr size_t PIMSMv2EncUIPv4AddrSize{6u};

struct PIMSMv2EncGAddr final {
    uint8_t Family;
    uint8_t EncodingType;
    uint8_t BReservedZ;
    uint8_t MaskLen;
    // + variable length multicast address
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2EncGAddr) == 4u);

constexpr size_t PIMSMv2EncGIPv4AddrSize{8u};

struct PIMSMv2EncSrcAddr final {
    uint8_t Family;
    uint8_t EncodingType;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t R:1;
    uint8_t W:1;
    uint8_t S:1;
    uint8_t Reserved:5;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t Reserved:5;
    uint8_t S:1;
    uint8_t W:1;
    uint8_t R:1;
#else
#error  "Please fix pimc/core/Endian.hpp"
#endif
    uint8_t MaskLen;
    // + variable length source address
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2EncSrcAddr) == 4u);

constexpr size_t PIMSMv2EncSrcAddrSize{8u};

struct PIMSMv2HelloOption final {
    uint16_t Type;
    uint16_t Length;

} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2HelloOption) == 4u);

constexpr size_t PIMSMv2IPv4MaxPruneSGrptLen{180u};

} // namespace pimc