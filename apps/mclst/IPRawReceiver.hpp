#pragma once

#include "pimc/net/IPv4HdrView.hpp"
#include "pimc/net/UDPHdrView.hpp"

#include "ReceiverBase.hpp"

namespace pimc {

template <Limiter Limit>
class IPRawReceiver: public ReceiverBase<IPRawReceiver<Limit>, Limit> {
    using Base = ReceiverBase<IPRawReceiver<Limit>, Limit>;
public:

    IPRawReceiver(Config const& cfg, OutputHandler& oh, bool& stopped)
    : Base{cfg, oh, stopped}, groupNl_{cfg.group().to_nl()} {}

protected:
    using Base::cfg_;
    using Base::oh_;
    using Base::dissectMclstBeaconPayload;

public:
    auto openSocket() -> int {
        int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

        if (s == -1) {
            if (errno == EPERM)
                raise<std::runtime_error>(
                        "permission to receive multicast on "
                        "all UDP ports denied; try running under sudo or "
                        "granting the binary the CAP_NET_RAW capability");
            else raise<std::runtime_error>(
                    "unable to open raw IP socket: {}", sysError());
        }

        return s;
    }

    auto processPacket(
            sockaddr_in const&, PacketInfo& pktInfo) -> PacketStatus {
        PacketView pv{pktInfo.receivedData, pktInfo.receivedSize};

        IPv4HdrView ipHdr;
        if (not pv.take(IPv4HdrView::HdrSize, [&ipHdr] (auto const* p) {
            ipHdr = p;
        })) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "recvmsg() returned size {} which is smaller than the minimum "
                    "IPv4 header size {}",
                    pktInfo.receivedSize, IPv4HdrView::HdrSize);
            return PacketStatus::Filtered;
        }

        if (ipHdr.daddr() != groupNl_) return PacketStatus::Filtered;
        if (ipHdr.protocol() != UDPProto) return PacketStatus::Filtered;

        auto effIPHdrSize = ipHdr.headerSizeBytes();
        if (effIPHdrSize < IPv4HdrView::HdrSize) {
            // This will really never happen
            oh_.warningTs(
                    pktInfo.timestamp,
                    "corrupted IPv4 header: header size in header is {} "
                    "whereas the minimum header size is {}",
                    effIPHdrSize, IPv4HdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }

        if (not pv.skip(effIPHdrSize - IPv4HdrView::HdrSize)) {
            // This can also not really happen...
            oh_.warningTs(
                    pktInfo.timestamp,
                    "recvmsg() returned size {} which is smaller than the actual "
                    "IPv4 header size {}",
                    pktInfo.receivedSize, IPv4HdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }

        UDPHdrView udpHdr;
        if (not pv.take(UDPHdrView::HdrSize, [&udpHdr] (auto const* p) {
            udpHdr = p;
        })) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "recvmsg() returned size {} which is insufficient for IPv4 "
                    "and UDP headers ({} + {} = {})",
                    pktInfo.receivedSize, effIPHdrSize, UDPHdrView::HdrSize,
                    effIPHdrSize + UDPHdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }

        pktInfo.source = net::IPv4Address::from_nl(ipHdr.saddr());
        pktInfo.sport = ntohs(udpHdr.sport());
        pktInfo.dport = ntohs(udpHdr.dport());

        auto ipTTL = static_cast<int16_t>(ipHdr.ttl());
        if (pktInfo.ttl != ipTTL) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "in packet {}:{}->{}:{} TTL received from recvmsg() is {} whereas "
                    "the TTL in the IPv4 header is {}, overriding",
                    pktInfo.source, pktInfo.sport, pktInfo.group, pktInfo.dport,
                    pktInfo.ttl, ipTTL);
            pktInfo.ttl = ipTTL;
        }

        auto remSize = pv.remaining();
        uint16_t udpSize = ntohs(udpHdr.len());

        if (udpSize < UDPHdrView::HdrSize) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "in packet {}:{}->{}:{} UDP size {} is less than the UDP header size {}",
                    pktInfo.source, pktInfo.sport, pktInfo.group, pktInfo.dport,
                    udpSize, UDPHdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }
        // The length in the UDP header includes the size of the UDP headers, thus
        // to get the UDP payload length we need to subtract the UDP header size
        // from the original value
        udpSize -= static_cast<uint16_t>(UDPHdrView::HdrSize);

        if (udpSize > remSize) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "in packet {}:{}->{}:{} UDP size {} is larger than the "
                    "size of the data after the IPv4 and UDP headers, which is {}",
                    pktInfo.source, pktInfo.sport, pktInfo.group, pktInfo.dport,
                    udpSize, remSize);
            return PacketStatus::AcceptedNoShow;
        }

        if (udpSize < remSize) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "in packet {}:{}->{}:{} UDP size {} is less than the "
                    "size of the data after the IPv4 and UDP headers, which is {}",
                    pktInfo.source, pktInfo.sport, pktInfo.group, pktInfo.dport,
                    udpSize, remSize);
        }

        pktInfo.payload = pktInfo.receivedData + pv.taken();
        pktInfo.payloadSize = udpSize;
        dissectMclstBeaconPayload();

        return PacketStatus::AcceptedShow;
    };

private:
    uint32_t groupNl_;
};

} // namespace pimc