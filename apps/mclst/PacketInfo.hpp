#pragma once

#include <cstdint>

#include "pimc/net/IPv4Address.hpp"

#include "MclstBeacon.hpp"

namespace pimc {

/*!
 * The buffer size to contain the payload of a received packets. This
 * buffer should be big enough to accommodate the payload of the
 * largest completely reassembled IP/UDP datagram.
 */
constexpr std::size_t BufferSize{67584};

struct PacketInfo final {
    uint64_t timestamp;
    IPv4Address source;
    uint16_t sport;
    IPv4Address group;
    uint16_t dport;
    // the index of the interface on which the packet was received
    unsigned ifIndex;
    // If ttl field is -1, it means the receiver was unable
    // to get the TTL value
    int16_t ttl;
    // The packet data
    uint8_t receivedData[BufferSize];
    unsigned receivedSize;
    // The pointer to the start of the payload in the field packet.
    // Unless the socket is raw, this pointer will point to the first
    // byte of the packet field
    uint8_t const* payload;
    unsigned payloadSize;

    // If the mclstBeacon is true, the rest of the fields are populated
    // from the dissected Mclst beacon payload
    bool mclstBeacon;
    uint64_t remoteSeq;
    uint64_t remoteTimestamp;
    size_t remoteMsgLen;
    char const* remoteMsg;

    void reset() {
        timestamp = 0ul;
        ifIndex = 0;
        ttl = -1;
        mclstBeacon = false;
    }
};


} // namespace pimc