#pragma once

#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>

#include <concepts>

#include "pimc/core/CompilerUtils.hpp"
#include "pimc/core/Endian.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/net/IPv4PktInfo.hpp"
#include "pimc/packets/PacketView.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "MclstBeacon.hpp"
#include "MclstBase.hpp"
#include "PacketInfo.hpp"
#include "RxStats.hpp"
#include "Timer.hpp"

namespace pimc {

template <typename T>
concept Limiter = requires(T limit, Config const& cfg) {
    { T{cfg} };
    { limit.reached() } -> std::same_as<bool>;
};

struct UnlimitedPackets {
    constexpr explicit UnlimitedPackets(Config const&) {}
    constexpr bool reached() { return false; }
};

class LimitedPackets {
public:
    explicit LimitedPackets(Config const& cfg)
    : limit_{cfg.count()}, count_{0} {}

    bool reached() { return ++count_ >= limit_; }
private:
    uint64_t limit_;
    uint64_t count_;
};

enum class PacketStatus: unsigned {
    /*!
     * \brief The packet is not accepted and should not be cause the timer reset.
     *
     * This should only be used by the raw received provider if it observes a
     * packet that is not destined for the configured multicast group.
     */
    Filtered = 0,

    /*!
     * \brief The packet is accepted but should not be shown as there was a
     * problem with its dissection. This is *very* unlikely to happen and if
     * it happens it only happens in the raw receiver provider.
     */
    AcceptedNoShow = 1,

    /*!
     * \brief The packet is accepted and should be shown.
     */
    AcceptedShow = 3,
};

template <typename T>
concept ReceiverProvider = requires(
        T rcvp, sockaddr_in const& sender, PacketInfo& pktInfo) {
    // The provider must throw an exception instead of returning -1
    { rcvp.openSocket() } -> std::same_as<int>;
    { rcvp.processPacket(sender, pktInfo) } -> std::same_as<PacketStatus>;
};

template <typename RP, Limiter Limit>
class ReceiverBase: private MclstBase {
protected:
    using MclstBase::cfg_;
    using MclstBase::oh_;

    ReceiverBase(Config const& cfg, OutputHandler& oh, bool& stopped)
    : MclstBase{cfg, oh, stopped}, limit_{cfg} { pktInfo_.group = cfg_.group(); }

    void dissectMclstBeaconPayload() {
        PacketView pv{pktInfo_.payload, pktInfo_.payloadSize};

        if (PIMC_UNLIKELY(not pv.take(sizeof(MclstBeaconHdr), [this] (auto const* p) {
            auto const& hdr = *static_cast<MclstBeaconHdr const*>(p);
            if (be64toh(hdr.magic) == MclstMagic) {
                pktInfo_.mclstBeacon = true;
                pktInfo_.remoteSeq = be64toh(hdr.seq);
                pktInfo_.remoteTimestamp = be64toh(hdr.timeNs);
                pktInfo_.remoteMsgLen = be16toh(hdr.dataLen);
            }
        }))) return;

        if (PIMC_UNLIKELY(not pv.take(pktInfo_.remoteMsgLen, [this] (auto const* p) {
            pktInfo_.remoteMsg = static_cast<char const*>(p);
        }))) {
            pktInfo_.mclstBeacon = false;
            oh_.warningTs(
                    pktInfo_.timestamp,
                    "{}:{}->{}:{}: in message #{} "
                    "length is {}, but the remaining length is {}",
                    pktInfo_.source, pktInfo_.sport, pktInfo_.group, pktInfo_.dport,
                    pktInfo_.remoteSeq, pktInfo_.remoteMsgLen, pv.remaining());
        }
    }

private:
    static constexpr unsigned Accepted{1};
    static constexpr unsigned Show{2};

private:
    template <typename Self = RP>
    Self& impl() noexcept requires ReceiverProvider<Self> {
        return static_cast<Self&>(*this);
    }

    void configure() {
        socket_ = impl().openSocket();

        // Make socket non-blocking
        int flags = fcntl(socket_, F_GETFL);
        if (flags == -1)
            raise<std::runtime_error>(
                    "fcntl() failed to get socket flags: {}", SysError{});

        flags |= O_NONBLOCK;
        fcntl(socket_, F_SETFL, flags);
        if (flags == -1)
            raise<std::runtime_error>(
                    "fcntl() failed to make socket non-blocking: {}", SysError{});

        // allow multiple sockets use the same UDP ports
        int allowReuse = 1;
        if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
                       &allowReuse, sizeof(allowReuse)) == -1)
            raise<std::runtime_error>("cannot enable UDP port reuse: {}", SysError{});

        int bufSize{BufferSize};
        if (setsockopt(socket_, SOL_SOCKET,
                       SO_RCVBUF, &bufSize, sizeof(bufSize)) == -1) {
            oh_.warning(
                    "failed to set receive buffer size to {} bytes: {}",
                    bufSize, SysError{});
        }

        int ttl = 1;

        if (setsockopt(socket_, IPPROTO_IP, IP_RECVTTL, &ttl, sizeof(ttl)) == -1)
            raise<std::runtime_error>("cannot enable receiving TTL: {}", SysError{});

        int pktinfo{1};
        if (setsockopt(socket_, IPPROTO_IP, IP_PKTINFO, &pktinfo, sizeof(pktinfo)) == -1)
            raise<std::runtime_error>(
                    "cannot enable receiving the interface on which packet is received{}",
                    SysError{});

        // Bind the socket to any interface. The join will be sent
        // from the interface specified on the command line
        sockaddr_in src;
        memset(&src, 0, sizeof(src));
        src.sin_family = AF_INET;
        src.sin_port = htons(cfg_.dport());
        src.sin_addr.s_addr = INADDR_ANY;

        if (bind(socket_, reinterpret_cast<sockaddr*>(&src), sizeof(src)) == -1)
            raise<std::runtime_error>(
                    "cannot bind socket to UDP port {}: {}",
                    cfg_.dport(), SysError{});

        // Activate poller
        FD_ZERO(&rfds_);
        FD_SET(socket_, &rfds_);
    }

    void join() {
        if (cfg_.source() != IPv4Address{}) {
            ip_mreq_source mreq_source{};
            mreq_source.imr_interface.s_addr = cfg_.intfAddr().to_nl();
            mreq_source.imr_multiaddr.s_addr = cfg_.group().to_nl();
            mreq_source.imr_sourceaddr.s_addr = cfg_.source().to_nl();

            if (setsockopt(socket_, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
                           &mreq_source, sizeof(mreq_source)) == -1)
                raise<std::runtime_error>(
                        "failed to join ({}, {}) on {}: {}",
                        cfg_.source(), cfg_.group(), cfg_.intf(), SysError{});
        } else {
            ip_mreq mreq{};
            mreq.imr_interface.s_addr = cfg_.intfAddr().to_nl();
            mreq.imr_multiaddr.s_addr = cfg_.group().to_nl();

            if (setsockopt(socket_, IPPROTO_IP,
                           IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
                raise<std::runtime_error>(
                        "failed to join (*, {}) on {}: {}",
                        cfg_.group(), cfg_.intf(), SysError{});
        }
    }

    PacketStatus receive(uint64_t recvTime) {
        pktInfo_.reset();

        iovec iov;
        iov.iov_base = pktInfo_.receivedData;
        iov.iov_len = sizeof(pktInfo_.receivedData);
        constexpr size_t cmsgSize =
                sizeof(cmsghdr) + sizeof(int16_t) + sizeof(in_pktinfo) + 64ul;
        uint8_t cmsgBuf[CMSG_SPACE(cmsgSize)];
        sockaddr_in sender;
        memset(&sender, 0, sizeof(sender));
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = &sender;
        msg.msg_namelen = sizeof(sender);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgBuf;
        msg.msg_controllen = sizeof(cmsgBuf);

        ssize_t rsz = recvmsg(socket_, &msg, 0);

        if (rsz < 0)
            raise<std::runtime_error>("recvmsg() failed: {}", SysError{});

        pktInfo_.timestamp = recvTime;
        pktInfo_.receivedSize = static_cast<unsigned>(rsz);

        for (auto cmsgp = CMSG_FIRSTHDR(&msg);
             cmsgp != nullptr;
             cmsgp = CMSG_NXTHDR(&msg, cmsgp)) {

            // Technically, we should only be looking for IP_TTL, but in macOS
            // this doesn't work, instead what we get is IP_RECVTTL
            if (cmsgp->cmsg_level == IPPROTO_IP and
                (cmsgp->cmsg_type == IP_TTL or cmsgp->cmsg_type == IP_RECVTTL) and
                cmsgp->cmsg_len > 0) {
                auto ttl = *reinterpret_cast<uint8_t const*>(CMSG_DATA(cmsgp));
                pktInfo_.ttl = static_cast<int16_t>(ttl);
                continue;
            }

            if (cmsgp->cmsg_level == IPPROTO_IP and
                cmsgp->cmsg_type == IP_PKTINFO and
                cmsgp->cmsg_len > 0) {
                auto const* p = static_cast<void const*>(CMSG_DATA(cmsgp));
                auto const* pi = reinterpret_cast<in_pktinfo const*>(p);
                pktInfo_.ifIndex = IF_INDEX(pi->ipi_ifindex);
            }
        }

        return impl().processPacket(sender, pktInfo_);
    }

    void receiveLoop() {
        fd_set rfds;
        RxStats::Timer rxStatsTimer{rxStats_};
        Timer timer{cfg_};

        while (not stopped_) {
            memcpy(&rfds, &rfds_, sizeof(rfds));
            tout_.tv_sec = cfg_.timeoutSec();
            tout_.tv_usec = 0;
            int rc = select(socket_+1, &rfds, nullptr, nullptr, &tout_);
            timer.save();

            if (rc < 0) {
                if (errno == EINTR) continue;

                raise<std::runtime_error>("select() failed: {}", SysError{});
            }

            if (rc == 0) {
                if (timer.timeout()) {
                    oh_.showTimeout(timer.timestamp());
                    timer.reset();
                }

                continue;
            }

            if (PIMC_LIKELY(FD_ISSET(socket_, &rfds_))) {
                auto ps = static_cast<unsigned>(receive(timer.timestamp()));

                if (PIMC_LIKELY(ps & Accepted)) {
                    timer.reset();

                    if (PIMC_LIKELY(ps & Show)) {
                        oh_.showReceivedPacket(pktInfo_);

                        // NoShow means pktInfo_ is incomplete and instead the
                        // receive() call produced a warning. Therefore, we only
                        // count packets which are shown.
                        rxStats_.update(
                                pktInfo_.source, pktInfo_.sport, pktInfo_.dport,
                                pktInfo_.payloadSize);
                    }

                    if (limit_.reached()) return;
                    continue;
                }
            } else {
                // This should never happen
                oh_.warningTs(
                        timer.timestamp(),
                        "select returned {} but multicast socket has no data",
                        rc);
            }
        }
    }

public:
    void run() {
        configure();
        join();
        receiveLoop();
        oh_.showRxStats(rxStats_, stopped_);
    }

private:
    PacketInfo pktInfo_;
    fd_set rfds_;
    timeval tout_;
    Limit limit_;
    RxStats rxStats_;
};

} // namespace pimc
