#pragma once

#include "MclstBase.hpp"

namespace pimc {

class Sender final: private MclstBase {
public:
    constexpr Sender(Config const& cfg, OutputHandler& oh, bool& stopped)
    : MclstBase{cfg, oh, stopped}, seq_{0} {}

    void run() {
        init();
        sendLoop();
        oh_.showTxStats(seq_+1, stopped_);
    }

private:
    void init();

    void sendLoop();
private:
    struct MclstBeaconPacket  {
        MclstBeaconHdr hdr;
        char message[1024];
    } __attribute__((__aligned__(1), __packed__));

private:
    MclstBeaconPacket pkt_;
    uint64_t seq_;
    size_t pktSize_;
};


} // namespace pimc
