#pragma once

#include "ReceiverBase.hpp"

namespace pimc {

template <Limiter Limit>
class Receiver final: public ReceiverBase<Receiver<Limit>, Limit> {
    using Base = ReceiverBase<Receiver<Limit>, Limit>;
public:
    Receiver(Config const& cfg, OutputHandler& oh, bool& stopped)
    : Base{cfg, oh, stopped} {}

protected:
    using Base::cfg_;
    using Base::dissectMclstBeaconPayload;

public:
    auto openSocket() -> int {
        int s = socket(AF_INET, SOCK_DGRAM, 0);

        if (s == -1)
            raise<std::runtime_error>("unable to create socket: {}", SysError{});

        return s;
    }

    auto processPacket(
            sockaddr_in const& sender, PacketInfo& pktInfo) -> PacketStatus {
        pktInfo.dport = cfg_.dport();
        pktInfo.source = net::IPv4Address::from_nl(sender.sin_addr.s_addr);
        pktInfo.sport = ntohs(sender.sin_port);
        pktInfo.payload = pktInfo.receivedData;
        pktInfo.payloadSize = pktInfo.receivedSize;
        dissectMclstBeaconPayload();
        return PacketStatus::AcceptedShow;
    }
};

}