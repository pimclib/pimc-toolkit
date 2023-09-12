#pragma once

#include <vector>

#include "config/PIMCConfig.hpp"
#include "net/IPv4PIMIntf.hpp"
#include "packets/IPv4PIMUpdatePacket.hpp"
#include "Timer.hpp"

namespace pimc {

class IPv4JPUpdateEvent final {
private:

    static auto updatePackets(PIMCConfig<IPv4> const& cfg)
    -> std::vector<IPv4PIMUpdatePacket> {
        std::vector<IPv4PIMUpdatePacket> pkts;
        pkts.reserve(cfg.updates().size());
        for (auto const& update: cfg.updates())
            pkts.emplace_back(
                    update,
                    cfg.pimsmConfig().intfAddr(),
                    cfg.pimsmConfig().neighbor(),
                    cfg.pimsmConfig().jpHoldtime());
        return pkts;
    }

public:
    explicit IPv4JPUpdateEvent(
            IPv4PIMIntf& pimIntf,
            Timer& timer,
            PIMCConfig<IPv4> const& cfg)
            : pimIntf_{pimIntf}
            , timer_{timer}
            , updatePackets_{updatePackets(cfg)}
            , jpPeriod_{cfg.pimsmConfig().jpPeriod()}
            , nextEventTime_{timer.inSec(jpPeriod_)}
            , pktName_{"Join/Prune Update"} {}

    [[nodiscard]]
    bool ready() {
        if (timer_.cts() >= nextEventTime_) {
            nextEventTime_ = timer_.inSec(jpPeriod_);
            return true;
        }

        return false;
    }

    [[nodiscard]]
    Result<void, std::string> fire() {
        for (auto const& pkt: updatePackets_) {
            auto r = pimIntf_.send(pkt.data(), pkt.size(), pktName_);
            if (not r)
                return r;
        }

        return {};
    }

private:
    IPv4PIMIntf& pimIntf_;
    Timer& timer_;
    std::vector<IPv4PIMUpdatePacket> updatePackets_;
    unsigned jpPeriod_;
    uint64_t nextEventTime_;
    std::string pktName_;
};

} // namespace pimc
