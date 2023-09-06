#pragma once

#include "config/PIMSMConfig.hpp"
#include "net/IPv4PIMIntf.hpp"
#include "packets/IPv4PIMHelloPacket.hpp"
#include "Timer.hpp"

namespace pimc {

class IPv4HelloEvent final {
public:
    IPv4HelloEvent(
            IPv4PIMIntf& pimIntf,
            PIMSMConfig<IPv4> const& cfg)
            : pimIntf_{pimIntf}
            , pkt_{IPv4PIMHelloPacket::create(
                    cfg.intfAddr(),
                    cfg.helloHoldtime(),
                    cfg.drPriority(),
                    cfg.generationId())}
            , helloPeriod_{cfg.helloPeriod()} {}

private:
    IPv4PIMIntf& pimIntf_;
    IPv4PIMHelloPacket pkt_;
    unsigned helloPeriod_;
};

} // namespace pimc
