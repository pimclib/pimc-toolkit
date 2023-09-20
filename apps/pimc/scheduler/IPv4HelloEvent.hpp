#pragma once

#include "config/PIMSMConfig.hpp"
#include "logging/Logging.hpp"
#include "net/IPv4PIMIntf.hpp"
#include "packets/IPv4PIMHelloPacket.hpp"
#include "Timer.hpp"

namespace pimc {

class IPv4HelloEvent final {
public:
    IPv4HelloEvent(
            Logger& log,
            IPv4PIMIntf& pimIntf,
            Timer& timer,
            PIMSMConfig<IPv4> const& cfg)
            : log_{log}
            , pimIntf_{pimIntf}
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
        auto r = pimIntf_.send(pkt_.data(), pkt_.size(), pktName_);
        if (r)
            log_.debug("Successfully sent {}", pkt_.descr());

        return r;
    }

private:
    Logger& log_;
    IPv4PIMIntf& pimIntf_;
    Timer& timer_;
    IPv4PIMHelloPacket pkt_;
    unsigned helloPeriod_;
    uint64_t nextEventTime_;
    std::string pktName_;
};

} // namespace pimc
