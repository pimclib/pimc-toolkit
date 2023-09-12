#pragma once

#include "config/PIMSMConfig.hpp"
#include "net/IPv4PIMIntf.hpp"
#include "packets/IPv4PIMHelloPacket.hpp"

namespace pimc {

class IPv4GoodbyeEvent final {
public:
    IPv4GoodbyeEvent(
            IPv4PIMIntf& pimIntf,
            PIMSMConfig<IPv4> const& cfg)
            : pimIntf_{pimIntf}
            , pkt_{cfg.intfAddr(),
                   0u,
                   cfg.drPriority(),
                   cfg.generationId()}
            , pktName_{"Goodbye"}{}

    [[nodiscard]]
    Result<void, std::string> send() {
        return pimIntf_.send(pkt_.data(), pkt_.size(), pktName_);
    }

private:
    IPv4PIMIntf& pimIntf_;
    IPv4PIMHelloPacket pkt_;
    std::string pktName_;
};

} // namespace pimc
