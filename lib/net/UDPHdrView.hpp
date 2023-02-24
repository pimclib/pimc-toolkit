#pragma once

#include <cstdint>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

constexpr uint8_t UDPProto{17u};

#ifdef __linux__
#include <linux/udp.h>

class UDPHdrView final {
public:
    inline static constexpr size_t HdrSize{sizeof(udphdr)};

    PIMC_ALWAYS_INLINE
    constexpr explicit UDPHdrView() : udpHdr_{nullptr} {}

    PIMC_ALWAYS_INLINE
    constexpr explicit UDPHdrView(void const *p)
            : udpHdr_{static_cast<udphdr const *>(p)} {}

    PIMC_ALWAYS_INLINE
    constexpr auto operator=(void const *p) -> UDPHdrView & {
        udpHdr_ = static_cast<udphdr const *>(p);
        return *this;
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t sport() const { return udpHdr_->source; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t dport() const { return udpHdr_->dest; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t len() const { return udpHdr_->len; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t sum() const { return udpHdr_->check; }

private:
    udphdr const* udpHdr_;
};

#endif

#ifdef __APPLE__

#include <netinet/udp.h>

class UDPHdrView final {
public:
    inline static constexpr size_t HdrSize{sizeof(udphdr)};

    PIMC_ALWAYS_INLINE
    constexpr explicit UDPHdrView() : udpHdr_{nullptr} {}

    PIMC_ALWAYS_INLINE
    constexpr explicit UDPHdrView(void const *p)
            : udpHdr_{static_cast<udphdr const *>(p)} {}

    PIMC_ALWAYS_INLINE
    constexpr auto operator=(void const *p) -> UDPHdrView & {
        udpHdr_ = static_cast<udphdr const *>(p);
        return *this;
    }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t sport() const { return udpHdr_->uh_sport; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t dport() const { return udpHdr_->uh_dport; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t len() const { return udpHdr_->uh_ulen; }

    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t sum() const { return udpHdr_->uh_sum; }

private:
    udphdr const* udpHdr_;
};

#endif

} // namespace pimc