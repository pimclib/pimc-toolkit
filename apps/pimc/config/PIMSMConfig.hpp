#pragma once

#include "pimc/net/IPAddress.hpp"

namespace pimc {

template <net::IPAddress A>
class PIMSMConfig final {
public:
    constexpr explicit PIMSMConfig(A neighbor)
    : neighbor_{neighbor} {}

    [[nodiscard]]
    A neighbor() const { return neighbor_; }

private:
    A neighbor_;
};

} // namespace pimc