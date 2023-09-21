#pragma once

#include "pimc/unix/CapState.hpp"

#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Result.hpp"
#include "pimc/system/SysError.hpp"

#include "pimc/packets/IPv4HdrView.hpp"
#include "pimc/packets/UDPHdrView.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"


#include "ReceiverBase.hpp"

namespace pimc {

template <Limiter Limit>
class IPRawReceiver: public ReceiverBase<IPRawReceiver<Limit>, Limit> {
    using Base = ReceiverBase<IPRawReceiver<Limit>, Limit>;

    inline static char const* LastResortMsg =
#ifdef WITH_LIBCAP
        "permission to receive multicast on all UDP ports denied "
        "even though the process now has the effective CAP_NET_RAW; "
        "as a last resort try running under sudo";
#else
        "permission to receive multicast on all UDP ports denied, "
        "try running under sudo";
#endif

public:

    IPRawReceiver(Config const& cfg, OutputHandler& oh, bool& stopped)
    : Base{cfg, oh, stopped}, groupNl_{cfg.group().to_nl()} {}

protected:
    using Base::cfg_;
    using Base::oh_;
    using Base::dissectMclstBeaconPayload;

public:
    auto openSocket(char const* progname) -> int {
        auto r = CapState::raiseFor(progname, CAP_(NET_RAW));
        if (not r)
            throw std::runtime_error{r.error()};

        int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

        if (s == -1) {
            if (errno == EPERM)
                throw std::runtime_error{LastResortMsg};
            else raise<std::runtime_error>(
                    "unable to open raw IP socket: {}", SysError{});
        }

        return s;
    }

    auto processPacket(
            sockaddr_in const&, PacketInfo& pktInfo) -> PacketStatus {
        PacketView pv{pktInfo.receivedData, pktInfo.receivedSize};

        IPv4HdrView ipHdr;
        if (PIMC_UNLIKELY(not pv.take(IPv4HdrView::HdrSize, [&ipHdr] (auto const* p) {
            ipHdr = p;
        }))) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "recvmsg() returned size {} which is smaller than the minimum "
                    "IPv4 header size {}",
                    pktInfo.receivedSize, IPv4HdrView::HdrSize);
            return PacketStatus::Filtered;
        }

        if (PIMC_UNLIKELY(ipHdr.daddr() != groupNl_)) return PacketStatus::Filtered;
        if (PIMC_UNLIKELY(ipHdr.protocol() != UDPProto)) return PacketStatus::Filtered;

        auto effIPHdrSize = ipHdr.headerSizeBytes();
        if (PIMC_UNLIKELY(effIPHdrSize < IPv4HdrView::HdrSize)) {
            // This will really never happen
            oh_.warningTs(
                    pktInfo.timestamp,
                    "corrupted IPv4 header: header size in header is {} "
                    "whereas the minimum header size is {}",
                    effIPHdrSize, IPv4HdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }

        if (PIMC_UNLIKELY(not pv.skip(effIPHdrSize - IPv4HdrView::HdrSize))) {
            // This can also not really happen...
            oh_.warningTs(
                    pktInfo.timestamp,
                    "recvmsg() returned size {} which is smaller than the actual "
                    "IPv4 header size {}",
                    pktInfo.receivedSize, IPv4HdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }

        UDPHdrView udpHdr;
        if (PIMC_UNLIKELY(not pv.take(UDPHdrView::HdrSize, [&udpHdr] (auto const* p) {
            udpHdr = p;
        }))) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "recvmsg() returned size {} which is insufficient for IPv4 "
                    "and UDP headers ({} + {} = {})",
                    pktInfo.receivedSize, effIPHdrSize, UDPHdrView::HdrSize,
                    effIPHdrSize + UDPHdrView::HdrSize);
            return PacketStatus::AcceptedNoShow;
        }

        pktInfo.source = IPv4Address::from_nl(ipHdr.saddr());
        pktInfo.sport = ntohs(udpHdr.sport());
        pktInfo.dport = ntohs(udpHdr.dport());

        auto ipTTL = static_cast<int16_t>(ipHdr.ttl());
        if (PIMC_UNLIKELY(pktInfo.ttl != ipTTL)) {
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

        if (PIMC_UNLIKELY(udpSize < UDPHdrView::HdrSize)) {
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

        if (PIMC_UNLIKELY(udpSize > remSize)) {
            oh_.warningTs(
                    pktInfo.timestamp,
                    "in packet {}:{}->{}:{} UDP size {} is larger than the "
                    "size of the data after the IPv4 and UDP headers, which is {}",
                    pktInfo.source, pktInfo.sport, pktInfo.group, pktInfo.dport,
                    udpSize, remSize);
            return PacketStatus::AcceptedNoShow;
        }

        if (PIMC_UNLIKELY(udpSize < remSize)) {
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