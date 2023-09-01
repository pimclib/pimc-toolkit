#include <cstdint>
#include <vector>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/packets/PacketWriter.hpp"
#include "pimc/packets/IPv4HdrWriter.hpp"
#include "pimc/packets/PIMSMv2Utils.hpp"
#include "pimc/system/Exceptions.hpp"

#include "pimsm/Update.hpp"
#include "IPv4PIMUpdatePacket.hpp"

namespace pimc {

namespace {

void writeGroupEntry(PacketWriter& pw, GroupEntry<IPv4> const& ge) {
    pimsmv2::writeIPv4Grp(pw, ge.group());
    auto* jnp = next<uint16_t>(pw);
    *jnp = htons(static_cast<uint16_t>(ge.joins().size()));
    auto* pnp = next<uint16_t>(pw);
    *pnp = htons(static_cast<uint16_t>(ge.prunes().size()));
    for (auto const& s: ge.joins())
        pimsmv2::writeIPv4Src(pw, s.addr(), s.rpt(), s.wildcard());
}

} // anon.namespace

IPv4PIMUpdatePacket IPv4PIMUpdatePacket::create(
        Update<IPv4> const& update,
        IPv4Address source, IPv4Address neighbor, uint16_t holdtime) {
    UpdateSummary<IPv4> us{0, update};

    size_t pimSz = pimsm::params<IPv4>::PIMJPHdrSize + us.size();
    size_t sz = IPv4HdrWriter::HdrSize + pimSz;
    std::vector<uint8_t> data;
    data.reserve(sz);
    data.resize(sz);

    PacketWriter pw{static_cast<void*>(data.data())};

    next<IPv4HdrWriter>(pw, IPv4HdrWriter::HdrSize)
            .tos(192)
            .totalLen(htons(static_cast<uint16_t >(sz)))
            .id(0)
            .flagsAndFragOff(htons(IP_DF))
            .ttl(1)
            .saddr(source.to_nl())
            .daddr(pimsm::params<IPv4>::AllPIMRouters.to_nl());

    // Take a snapshot at the current position, so we could compute the
    // PIM data checksum at the end
    auto pimPw = pw.mark();

    // Write the Join/Prune header
    pimsmv2::writeHdr(pw, PIMSMv2_JOIN_PRUNE);
    pimsmv2::writeIPv4JPHdr(
            pw, neighbor, static_cast<uint8_t>(update.groups().size()), holdtime);

    // Write J/P state for each group in the update
    for (auto const& ge: update.groups())
        writeGroupEntry(pw, ge);

    // Sanity check
    if (pw.size() != sz)
        raise<std::logic_error>(
                "expecting PIM Join/Prune update packet of size {},"
                " whereas the encoded size is {} "
                "(including the IPv4 header)",
                sz, pw.size());

    pimsmv2::writeChkSum(pimPw, pimSz);

    return IPv4PIMUpdatePacket{std::move(data)};
}

} // namespace pimc
