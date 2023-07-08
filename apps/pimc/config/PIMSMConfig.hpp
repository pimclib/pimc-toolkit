#pragma once

#include "pimc/net/IPv4Address.hpp"

namespace pimc {

class PIMSMConfig final {
public:
    constexpr explicit PIMSMConfig(net::IPv4Address neighbor)
    : neighbor_{neighbor} {}

    [[nodiscard]]
    net::IPv4Address neighbor() const { return neighbor_; }

private:
    net::IPv4Address neighbor_;
};

} // namespace pimc