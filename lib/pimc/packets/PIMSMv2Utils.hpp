#pragma once

#include "pimc/net/IPv4Address.hpp"
#include "IPChecksum.hpp"
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
inline constexpr void writeHdr(PacketWriter& pw, uint8_t type) {
    auto* hdr = next<PIMSMv2Hdr>(pw);
    hdr->Version = 2u;
    hdr->Type = 0xf & type;
    hdr->Reserved = 0u;
}

/*!
 * \brief Computes and writes the IP checksum into the PIM checksum
 * field.
 *
 * The packet writer \p pw must be positioned at the start of the PIM
 * header and must have size 0.
 *
 * @param pw the packet writer positioned at the PIM header
 * @param sz the size of the PIM payload
 */
inline void writeChkSum(PacketWriter pw, size_t sz) {
    auto const* data = pw.data();
    auto* hdr = next<PIMSMv2Hdr>(pw);
    hdr->Checksum = ipChecksumNs(data, sz);
}

/*!
 * \brief Writes a PIM SM encoded unicast address corresponding to
 * \p uaddr to the current position of the packet writer \p pw.
 * @param pw the packet writer
 * @param uaddr the address to encode
 */
inline void writeIPv4Addr(PacketWriter& pw, IPv4Address uaddr) {
    auto* h = next<PIMSMv2EncUAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    auto* ap = next<uint32_t>(pw);
    *ap = uaddr.to_nl();
}

/*!
 * \brier Writes the Join/Prune header to the current position of
 * the packet writer \p pw.
 *
 * @param pw the packet writer
 * @param neighbor the IP address of the PIM neighbor
 * @param grpNum the number of groups in this update
 * @param holdtime the PIM hold time in the host byte order
 */
inline void writeIPv4JPHdr(
        PacketWriter& pw, IPv4Address neighbor, uint8_t grpNum, uint16_t holdtime) {
    writeIPv4Addr(pw, neighbor);
    auto* reserved = next<uint8_t>(pw);
    *reserved = 0u;
    auto* gnp = next<uint8_t>(pw);
    *gnp = grpNum;
    auto* htp = next<uint16_t>(pw);
    *htp = htons(holdtime);
}

/*!
 * \brief Writer a PIM SM encoded group address corresponding to
 * \p group to the current position of the packer writer \p pw.
 *
 * @param pw the packet writer
 * @param group the multicast group to encode
 */
inline void writeIPv4Grp(PacketWriter& pw, IPv4Address group) {
    auto* h = next<PIMSMv2EncGAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    h->BReservedZ = 0u;
    h->MaskLen = 32u;
    auto *gp = next<uint32_t>(pw);
    *gp = group.to_nl();
}

/*!
 * \brief Writes a PIM SM encoded multicast source address corresponding
 * to \p src.
 *
 * The parameter \p rpt indicates if the source entry is for the shared
 * tree, if \p rpt is true, or the shortest tree, if \p ptr is false.
 *
 * @param pw the packet writer
 * @param wc the wildcard bit (is set only for RP in Join/Prune(*,G))
 * @param src the multicast source address
 * @param rpt the flag specifying if the source is on the shared or
 * shortest path tree
 */
inline void writeIPv4Src(PacketWriter& pw, IPv4Address src, bool rpt, bool wc) {
    auto* h = next<PIMSMv2EncSrcAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    h->R = rpt ? 1u : 0u;
    h->W = wc ? 1u : 0u;
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
 * @param holdtime the holdtime in the host byte order
 */
inline void writeOptHoldtime(PacketWriter& pw, uint16_t holdtime) {
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
 * @param drPrio the DR priority in the host byte order
 */
inline void writeOptDrPriority(PacketWriter& pw, uint32_t drPrio) {
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
 * @param genId the generation ID in the host byte order
 */
inline void writeOptGenerationId(PacketWriter& pw, uint32_t genId) {
    auto* h = next<PIMSMv2HelloOption>(pw);
    h->Type = htons(PIMSMv2_OPT_GENERATION_ID);
    h->Length = htons(4u);
    auto* gia = next<uint32_t>(pw);
    *gia = htonl(genId);
}

} // namespace pimc::pimsmv2
