#pragma once

#include <vector>

#include "config/PIMCConfig.hpp"
#include "net/IPv4PIMIntf.hpp"
#include "packets/IPv4PIMUpdatePacket.hpp"

namespace pimc {

class IPv4GoodbyeJPUpdateEvent final {
private:

    static auto inverseUpdatePackets(PIMCConfig<IPv4> const& cfg)
    -> std::vector<IPv4PIMUpdatePacket> {
        std::vector<IPv4PIMUpdatePacket> pkts;
        pkts.reserve(cfg.inverseUpdates().size());
        for (auto const& update: cfg.inverseUpdates())
            pkts.emplace_back(
                    update,
                    cfg.pimsmConfig().intfAddr(),
                    cfg.pimsmConfig().neighbor(),
                    cfg.pimsmConfig().jpHoldtime());
        return pkts;
    }

public:
    explicit IPv4GoodbyeJPUpdateEvent(
            IPv4PIMIntf& pimIntf,
            PIMCConfig<IPv4> const& cfg)
            : pimIntf_{pimIntf}
            , inverseUpdatePackets_{inverseUpdatePackets(cfg)}
            , pktName_{"Goodbye Join/Prune Update"} {}


    [[nodiscard]]
    Result<void, std::string> send() {
        for (auto const& pkt: inverseUpdatePackets_) {
            auto r = pimIntf_.send(pkt.data(), pkt.size(), pktName_);
            if (not r)
                return r;
        }

        return {};
    }

private:
    IPv4PIMIntf& pimIntf_;
    std::vector<IPv4PIMUpdatePacket> inverseUpdatePackets_;
    std::string pktName_;
};

} // namespace pimc
