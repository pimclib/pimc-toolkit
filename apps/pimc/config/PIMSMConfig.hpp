#pragma once

#include "pimc/net/IP.hpp"

namespace pimc {

template <IPVersion V>
class PIMSMConfig final {
public:
    using IPAddress = typename IP<V>::Address;

    constexpr explicit PIMSMConfig(
            IPAddress neighbor,
            unsigned intfIndex,
            IPAddress intfAddr,
            std::string intfName)
    : neighbor_{neighbor}
    , intfIndex_{intfIndex}
    , intfAddr_{intfAddr}
    , intfName_{std::move(intfName)} {}

    [[nodiscard]]
    IPAddress neighbor() const { return neighbor_; }

    [[nodiscard]]
    unsigned intfIndex() const { return intfIndex_; }

    [[nodiscard]]
    IPAddress intfAddr() const { return intfAddr_; }

    [[nodiscard]]
    std::string const& intfName() const { return intfName_; }

    [[nodiscard]]
    uint16_t helloPeriod() const { return helloPeriod_; }

    [[nodiscard]]
    uint16_t helloHoldtime() const { return helloHoldtime_; }

    [[nodiscard]]
    uint16_t jpHoldtime() const { return jpHoldtime_; }

private:
    IPAddress neighbor_;
    unsigned intfIndex_;
    IPAddress intfAddr_;
    std::string intfName_;
    uint16_t helloPeriod_;
    uint16_t helloHoldtime_;
    uint16_t jpHoldtime_;
};

} // namespace pimc