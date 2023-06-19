#include <cstdint>
#include <cstring>
#include <netinet/in.h>

#include "pimc/core/Endian.hpp"
#include "IPv4Address.hpp"

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

namespace detail {

class PktWriter final {
public:
    constexpr explicit PktWriter(void* m)
    : m_{static_cast<uint8_t*>(m)}, offs_{0} {}

    template <typename T>
    [[nodiscard]]
    constexpr auto next() -> T* {
        auto* p = static_cast<void*>(m_ + offs_);
        offs_ += sizeof(T);
        return static_cast<T*>(p);
    }

    [[nodiscard]]
    constexpr size_t offset() const { return offs_; }
private:
    uint8_t * m_;
    size_t offs_;
};

} // namespace detail

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

    constexpr static auto write(void* m, uint8_t type) -> size_t {
        detail::PktWriter pw{m};
        auto* hdr = pw.next<PIMSMv2Hdr>();
        hdr->Version = 2u;
        hdr->Type = type;
        hdr->Reserved = 0u;
        return pw.offset();
    }
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2Hdr) == 4u);

struct PIMSMv2EncUAddr final {
    uint8_t Family;
    uint8_t EncodingType;

    static auto writeIPv4Addr(void* m, net::IPv4Address uaddr) -> size_t {
        detail::PktWriter pw{m};
        auto* h = pw.next<PIMSMv2EncUAddr>();
        h->Family = IPv4_FAMILY_NUMBER;
        h->EncodingType = PIMSMv2_NATIVE_ENCODING;
        auto* ap = pw.next<uint32_t>();
        *ap = uaddr.to_nl();
        return pw.offset();
    }
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2EncUAddr) == 2u);

constexpr size_t PIMSMv2EncUIPv4AddrSize{6u};

struct PIMSMv2EncGAddr final {
    uint8_t Family;
    uint8_t EncodingType;
    uint8_t BReservedZ;
    uint8_t MaskLen;

    static auto writeIPv4Grp(void* m, net::IPv4Address group) -> size_t {
        detail::PktWriter pw{m};
        auto* h = pw.next<PIMSMv2EncGAddr>();
        h->Family = IPv4_FAMILY_NUMBER;
        h->EncodingType = PIMSMv2_NATIVE_ENCODING;
        h->BReservedZ = 0u;
        h->MaskLen = 32u;
        auto *gp = pw.next<uint32_t>();
        *gp = group.to_nl();
        return pw.offset();
    }
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

    static auto writeIPv4RP(void* m, net::IPv4Address rp) -> size_t {
        detail::PktWriter pw{m};
        auto* h = pw.next<PIMSMv2EncSrcAddr>();
        h->Family = IPv4_FAMILY_NUMBER;
        h->EncodingType = PIMSMv2_NATIVE_ENCODING;
        h->R = 1u;
        h->W = 1u;
        h->S = 1u;
        h->Reserved = 0;
        h->MaskLen = 32;
        auto* rpa = pw.next<uint32_t>();
        *rpa = rp.to_nl();
        return pw.offset();
    }

    static auto writeIPv4Src(void* m, net::IPv4Address src, bool rpt) {
        detail::PktWriter pw{m};
        auto* h = pw.next<PIMSMv2EncSrcAddr>();
        h->Family = IPv4_FAMILY_NUMBER;
        h->EncodingType = PIMSMv2_NATIVE_ENCODING;
        h->R = rpt ? 1u : 0u;
        h->W = 0u;
        h->S = 1u;
        h->Reserved = 0;
        h->MaskLen = 32;
        auto* srca = pw.next<uint32_t>();
        *srca = src.to_nl();
        return pw.offset();
    }
} __attribute__((packed, aligned(1)));

static_assert(sizeof(PIMSMv2EncSrcAddr) == 4u);

constexpr size_t PIMSMv2EncSrcAddrSize{8u};

} // namespace pimc