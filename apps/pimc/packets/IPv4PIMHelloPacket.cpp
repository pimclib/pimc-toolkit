#include <cstdint>
#include <vector>
#include <netinet/ip.h>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/packets/PacketWriter.hpp"
#include "pimc/packets/IPv4HdrWriter.hpp"
#include "pimc/packets/PIMSMv2Utils.hpp"
#include "pimc/system/Exceptions.hpp"

#include "pimsm/PIMSMParams.hpp"
#include "IPv4PIMHelloPacket.hpp"

namespace pimc {

IPv4PIMHelloPacket::IPv4PIMHelloPacket(
        IPv4Address source,
        uint16_t helloHoldtime, uint32_t drPriority, uint32_t generationId) {
    constexpr size_t pimSz =
            pimsm::params<IPv4>::PIMHdrSize +
            // Option 1: Hello Hold Time
            pimsm::params<IPv4>::HelloOptionHdrSize + 2ul +
            // Option 18: DR Priority
            pimsm::params<IPv4>::HelloOptionHdrSize + 4ul +
            // Option 20: Generation ID
            pimsm::params<IPv4>::HelloOptionHdrSize + 4ul;
    constexpr size_t sz = IPv4HdrWriter::HdrSize + pimSz;
    data_.reserve(sz);
    data_.resize(sz);

    PacketWriter pw{static_cast<void*>(data_.data())};

    next<IPv4HdrWriter>(pw, IPv4HdrWriter::HdrSize)
            .tos(192)
            .totalLen(htons(static_cast<uint16_t >(sz)))
            .id(0)
            .flagsAndFragOff(htons(IP_DF))
            .ttl(1)
            .protocol(IPPROTO_PIM)
            .saddr(source.to_nl())
            .daddr(pimsm::params<IPv4>::AllPIMRouters.to_nl());


    // Take a snapshot at the current position, so we could compute the
    // PIM data checksum at the end
    auto pimPw = pw.mark();

    // Write the Hello header
    pimsmv2::writeHdr(pw, PIMSMv2_HELLO);

    // Write the options
    pimsmv2::writeOptHoldtime(pw, helloHoldtime);
    pimsmv2::writeOptDrPriority(pw, drPriority);
    pimsmv2::writeOptGenerationId(pw, generationId);

    // Sanity check
    if (pw.size() != sz)
        raise<std::logic_error>(
                "expecting PIM Hello packet of size {},"
                " whereas the encoded size is {} "
                "(including the IPv4 header)",
                sz, pw.size());

    pimsmv2::writeChkSum(pimPw, pimSz);

    if (helloHoldtime > 0)
        descr_ = fmt::format(
                "IPv4 Hello [holdtime {}s, DR priority {}, generation ID {:08x}]",
                helloHoldtime, drPriority, generationId);
    else descr_ = fmt::format(
                "IPv4 Goodbye [DR priority {}, generation ID {:08x}]",
                drPriority, generationId);
}

}