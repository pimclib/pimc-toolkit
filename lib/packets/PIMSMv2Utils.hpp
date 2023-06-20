#pragma once

#include "pimc/packets/PacketWriter.hpp"
#include "IPv4Address.hpp"
#include "PIMSMv2.hpp"

namespace pimc::pimsmv2 {

constexpr void writeHdr(PacketWriter& pw, uint8_t type) {
    auto* hdr = next<PIMSMv2Hdr>(pw);
    hdr->Version = 2u;
    hdr->Type = type;
    hdr->Reserved = 0u;
}


void writeIPv4Addr(PacketWriter& pw, net::IPv4Address uaddr) {
    auto* h = next<PIMSMv2EncUAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    auto* ap = next<uint32_t>(pw);
    *ap = uaddr.to_nl();
}


void writeIPv4Grp(PacketWriter& pw, net::IPv4Address group) {
    auto* h = next<PIMSMv2EncGAddr>(pw);
    h->Family = IPv4_FAMILY_NUMBER;
    h->EncodingType = PIMSMv2_NATIVE_ENCODING;
    h->BReservedZ = 0u;
    h->MaskLen = 32u;
    auto *gp = next<uint32_t>(pw);
    *gp = group.to_nl();
}


static void writeIPv4RP(PacketWriter& pw, net::IPv4Address rp) {
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

static void writeIPv4Src(PacketWriter& pw, net::IPv4Address src, bool rpt) {
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


} // namespace pimc
