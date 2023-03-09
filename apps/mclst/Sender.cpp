#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

#include "pimc/core/Endian.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/time/TimeUtils.hpp"
#include "pimc/formatters/IPv4Formatters.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "Sender.hpp"

using namespace std::chrono_literals;

namespace pimc {

void Sender::init() {
    if (gethostname(pkt_.message, sizeof(pkt_.message)) == -1)
        raise<std::runtime_error>("unable to get local host name: {}", SysError{});

    auto msgLen = strlen(pkt_.message);
    pkt_.message[sizeof(pkt_.message)-1] = '\0';
    pkt_.hdr.magic = htobe64(MclstMagic);
    pkt_.hdr.dataLen = htobe16(static_cast<uint16_t>(msgLen));
    pktSize_ = sizeof(MclstBeaconHdr) + msgLen;

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == -1)
        raise<std::runtime_error>("unable to create socket: {}", SysError{});

    auto ttl = static_cast<u_char>(cfg_.ttl());
    if (setsockopt(socket_, IPPROTO_IP,
                   IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1)
        raise<std::runtime_error>("unable to set multicast TTL: {}", SysError{});

    // this is required to allow this host to receive its own packets
    u_char loopback{1};
    if (setsockopt(socket_, IPPROTO_IP,
                   IP_MULTICAST_LOOP, &loopback, sizeof(loopback)) == -1)
        raise<std::runtime_error>(
                "unable to set loopback mode on socket: {}", SysError{});

    in_addr intfAddr { .s_addr = cfg_.intfAddr().to_nl() };
    if (setsockopt(socket_, IPPROTO_IP,
                   IP_MULTICAST_IF, &intfAddr, sizeof(intfAddr)) == -1)
        raise<std::runtime_error>(
                "unable to make {} ({}) multicast output interface: {}",
                cfg_.intf(), cfg_.intfAddr(), SysError{});
}

void Sender::sendLoop() {
    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(cfg_.dport());
    dst.sin_addr.s_addr = cfg_.group().to_nl();

    while (not stopped_) {
        pkt_.hdr.timeNs = htobe64(gethostnanos());
        pkt_.hdr.seq = htobe64(seq_);
        if (sendto(socket_, &pkt_, pktSize_, 0,
                   reinterpret_cast<sockaddr*>(&dst), sizeof(dst)) == -1)
            raise<std::runtime_error>(
                    "failed to send packet to {}:{}: {}",
                    cfg_.group(), cfg_.dport(), SysError{});

        oh_.showSentPacket(gethostnanos(), seq_);
        ++seq_;

        if (cfg_.count() != 0 && seq_ >= cfg_.count()) return;

        std::this_thread::sleep_for(1s);
    }
}

} // namespace pimc
