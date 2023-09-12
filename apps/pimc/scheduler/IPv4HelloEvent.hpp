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
            Timer& timer,
            PIMSMConfig<IPv4> const& cfg)
            : pimIntf_{pimIntf}
            , timer_{timer}
            , pkt_{cfg.intfAddr(),
                   cfg.helloHoldtime(),
                   cfg.drPriority(),
                   cfg.generationId()}
            , helloPeriod_{cfg.helloPeriod()}
            , nextEventTime_{timer.cts()}
            , pktName_{"Hello"}{}

    [[nodiscard]]
    bool ready() {
        if (timer_.cts() >= nextEventTime_) {
            nextEventTime_ = timer_.inSec(helloPeriod_);
            return true;
        }

        return false;
    }

    [[nodiscard]]
    Result<void, std::string> fire() {
        return pimIntf_.send(pkt_.data(), pkt_.size(), pktName_);
    }

private:
    IPv4PIMIntf& pimIntf_;
    Timer& timer_;
    IPv4PIMHelloPacket pkt_;
    unsigned helloPeriod_;
    uint64_t nextEventTime_;
    std::string pktName_;
};

} // namespace pimc
