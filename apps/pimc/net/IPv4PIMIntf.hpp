#pragma once

#include "pimc/core/Result.hpp"
#include "config/PIMCConfig.hpp"

namespace pimc {

class IPv4PIMIntf final {
public:
    static auto create(char const* progname, PIMCConfig<IPv4> const& cfg,bool& stopped)
    -> Result<IPv4PIMIntf, std::string>;

private:
    constexpr IPv4PIMIntf(PIMCConfig<IPv4> const& cfg, int socket, bool& stopped)
    : cfg_{cfg}, socket_{socket}, stopped_{stopped} {}

private:
    PIMCConfig<IPv4> const& cfg_;
    int socket_;
    bool& stopped_;
};

} // namespace pimc
