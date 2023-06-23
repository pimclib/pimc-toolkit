#pragma once

#include "pimc/net/IPv4Address.hpp"
#include "PacketWriter.hpp"
#include "PIMSMv2.hpp"

namespace pimc::pimsmv2 {

/*!
 * \brief Writes a PIM SM v2 headers at the current position in the
 * packet writer \p pw.
 *
 * Sets the `type` field to the \p type.
 *
 * @param pw the packet writer to which to write the header
 * @param type the type of the PIM SM message
 */
constexpr void writeHdr(PacketWriter& pw, uint8_t type) {
    auto* hdr = next<PIMSMv2Hdr>(pw);
    hdr->Version = 2u;
    hdr->Type = type;
    hdr->Reserved = 0u;
}

/*!
 * \brief Writes a PIM SM encoded unicast address corresponding to
 * \p uaddr to the current position of the packet writer \p pw.
 * @param pw the packet writer
 * @param uaddr the address to encode
 */
void writeIPv4Addr(PacketWriter& pw, net::IPv4Address uaddr) {
    auto* h = next<PIMSMv2EncUAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    auto* ap = next<uint32_t>(pw);
    *ap = uaddr.to_nl();
}

/*!
 * \brief Writer a PIM SM encoded group address corresponding to
 * \p group to the current position of the packer writer \p pw.
 *
 * @param pw the packet writer
 * @param group the multicast group to encode
 */
void writeIPv4Grp(PacketWriter& pw, net::IPv4Address group) {
    auto* h = next<PIMSMv2EncGAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    h->BReservedZ = 0u;
    h->MaskLen = 32u;
    auto *gp = next<uint32_t>(pw);
    *gp = group.to_nl();
}

/*!
 * \brief Writes a PIM SM encoded RP address corresponding to
 * \p rp to the current position of the packe writer \p pw.
 *
 * @param pw the packet writer
 * @param rp the PIM SM v2 rendezvous point (RP)
 */
void writeIPv4RP(PacketWriter& pw, net::IPv4Address rp) {
    auto* h = next<PIMSMv2EncSrcAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    h->R = 1u;
    h->W = 1u;
    h->S = 1u;
    h->Reserved = 0;
    h->MaskLen = 32;
    auto* rpa = next<uint32_t>(pw);
    *rpa = rp.to_nl();
}

/*!
 * \brief Writes a PIM SM encoded multicast source address corresponding
 * to \p src.
 *
 * The parameter \p rpt indicates if the source entry is for the shared
 * tree, if \p rpt is true, or the shortest tree, if \p ptr is false.
 *
 * @param pw the packet writer
 * @param src the multicast source address
 * @param rpt the flag specifying if the source is on the shared or
 * shortest path tree
 */
void writeIPv4Src(PacketWriter& pw, net::IPv4Address src, bool rpt) {
    auto* h = next<PIMSMv2EncSrcAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    h->R = rpt ? 1u : 0u;
    h->W = 0u;
    h->S = 1u;
    h->Reserved = 0;
    h->MaskLen = 32;
    auto* srca = next<uint32_t>(pw);
    *srca = src.to_nl();
}

/*!
 * \brief Writes a PIM SM holdtime hellow packet option with the specified
 * \p holdtime.
 *
 * If \p holdtime is 0, this indicates that the PIM router is about to go
 * down. This message is otherwise known as a `goodbye`.
 *
 * @param pw the packet writer
 * @param holdtime the holdtime
 */
void writeOptHoldtime(PacketWriter& pw, uint16_t holdtime) {
    auto* h = next<PIMSMv2HelloOption>(pw);
    h->Type = htons(PIMSMv2_OPT_HOLDTIME);
    h->Length = htons(2u);
    auto* hta = next<uint16_t>(pw);
    *hta = htons(holdtime);
}

/*!
 * \brief Writes a PIM SM DR priority option with the specified DR
 * priority \p drPrio.
 *
 * The numerically larger DP priority indicates a higher preference
 * for the router to become a DR.
 *
 * @param pw the packet writer
 * @param drPrio the DR priority
 */
void writeOptDrPriority(PacketWriter& pw, uint32_t drPrio) {
    auto* h = next<PIMSMv2HelloOption>(pw);
    h->Type = htons(PIMSMv2_OPT_DR_PRIORITY);
    h->Length = htons(4u);
    auto* dpa = next<uint32_t>(pw);
    *dpa = htonl(drPrio);
}

/*!
 * \brief Writes a PIM SM generation ID with the specified value \p genId.
 *
 * @param pw the packet writer
 * @param genId the generation ID
 */
void writeOptGenerationId(PacketWriter& pw, uint32_t genId) {
    auto* h = next<PIMSMv2HelloOption>(pw);
    h->Type = htons(PIMSMv2_OPT_GENERATION_ID);
    h->Length = htons(4u);
    auto* gia = next<uint32_t>(pw);
    *gia = htonl(genId);
}

} // namespace pimc::pimsmv2
