#pragma once

#include <cstdint>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

#ifdef __linux__
#include <linux/ip.h>

class IPv4HdrView final {
public:
    inline static constexpr size_t HdrSize{sizeof(iphdr)};

    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrView(): ipHdr_{nullptr} {}

    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrView(void const* p)
    : ipHdr_{static_cast<iphdr const*>(p)} {}

    PIMC_ALWAYS_INLINE
    constexpr auto operator= (void const* p) -> IPv4HdrView& {
        ipHdr_ = static_cast<iphdr const*>(p);
        return *this;
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ihl() const { return ipHdr_->ihl; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t headerSizeBytes() const {
        return static_cast<uint16_t>(static_cast<uint16_t>(ihl()) << 2u);
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t version() const { return ipHdr_->version; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t tos() const { return ipHdr_->tos; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t totalLen() const { return ipHdr_->tot_len; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t id() const { return ipHdr_->id; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t flagsAndFragOff() const { return ipHdr_->frag_off; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ttl() const { return ipHdr_->ttl; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t protocol() const { return ipHdr_->protocol; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t hdrChecksum() const { return ipHdr_->check; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t saddr() const { return ipHdr_->saddr; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t daddr() const { return ipHdr_->daddr; }

private:
    iphdr const* ipHdr_;
};


#endif

#ifdef __APPLE__

#include <netinet/ip.h>

class IPv4HdrView final {
public:
    inline static constexpr size_t HdrSize{sizeof(ip)};

    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrView() : ipHdr_{nullptr} {}

    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrView(void const *p)
            : ipHdr_{static_cast<ip const *>(p)} {}

    PIMC_ALWAYS_INLINE
    constexpr auto operator=(void const *p) -> IPv4HdrView & {
        ipHdr_ = static_cast<ip const *>(p);
        return *this;
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ihl() const { return ipHdr_->ip_hl; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t headerSizeBytes() const {
        return static_cast<uint16_t>(static_cast<uint16_t>(ihl()) << 2u);
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t version() const { return ipHdr_->ip_v; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t tos() const { return ipHdr_->ip_tos; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t totalLen() const { return ipHdr_->ip_len; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t id() const { return ipHdr_->ip_id; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t flagsAndFragOff() const { return ipHdr_->ip_off; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ttl() const { return ipHdr_->ip_ttl; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t protocol() const { return ipHdr_->ip_p; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t hdrChecksum() const { return ipHdr_->ip_sum; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t saddr() const { return ipHdr_->ip_src.s_addr; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t daddr() const { return ipHdr_->ip_dst.s_addr; }

private:
    ip const *ipHdr_;
};

#endif

} // namespace pimc
