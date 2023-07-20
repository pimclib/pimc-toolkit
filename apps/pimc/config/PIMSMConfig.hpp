#pragma once

#include "pimc/net/IP.hpp"

namespace pimc {

template <IPVersion V>
class PIMSMConfig final {
public:
    using IPAddress = typename IP<V>::Address;

    constexpr explicit PIMSMConfig(IPAddress neighbor)
    : neighbor_{neighbor} {}

    [[nodiscard]]
    IPAddress neighbor() const { return neighbor_; }

private:
    IPAddress neighbor_;
};

} // namespace pimc