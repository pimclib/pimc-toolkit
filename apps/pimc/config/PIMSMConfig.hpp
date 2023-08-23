#pragma once

#include "pimc/net/IP.hpp"

namespace pimc {

template <IPVersion V>
class PIMSMConfig final {
public:
    using IPAddress = typename IP<V>::Address;

    constexpr explicit PIMSMConfig(
            IPAddress neighbor,
            IPAddress intfAddr,
            std::string intfName)
    : neighbor_{neighbor}
    , intfAddr_{intfAddr}
    , intfName_{std::move(intfName)} {}

    [[nodiscard]]
    IPAddress neighbor() const { return neighbor_; }

    [[nodiscard]]
    IPAddress intfAddr() const { return intfAddr_; }

    [[nodiscard]]
    std::string const& intfName() const { return intfName_; }

private:
    IPAddress neighbor_;
    IPAddress intfAddr_;
    std::string intfName_;
};

} // namespace pimc