#pragma once

#include "pimc/net/IP.hpp"
#include "pimc/packets/PIMSMv2.hpp"

namespace pimc::pimsm {

template<IPVersion>
struct params {
};

template<>
struct params<IPv4> {
    /// The capacity of the IPv4 Join/Prune message for the group entries
    /// in bytes:
    ///
    /// Ethernet MTU: 1522 bytes
    /// MAC Header:
    ///   Dst MAC Addr      6 bytes
    ///   Src Mac Addr      6 bytes
    ///   802.1Q tag        4 bytes (optional)
    ///   Ethertype         2 bytes (0x0800 for IPv4)
    /// Trailer FCS         4 bytes
    ///                    22 bytes (subtotal)
    ///           1500 bytes remaining
    /// IPv4 Header        20 bytes
    ///           1480 bytes remaining
    /// PIM-SM v2 for IPv4 Join/Prune header:
    ///   PIM Header         4 bytes
    ///   Upstream Neighbor  6 bytes
    ///   Reserved           1 byte
    ///   Number of groups   1 byte
    ///   Hold time          2 bytes
    ///                     14 bytes (subtotal)
    ///           1466 bytes remaining for group entries
    static constexpr size_t capacity{1466ul};
    /// PIM-SM encoded IPv4 multicast address size [8 bytes]
    static constexpr size_t GrpASize{PIMSMv2EncGIPv4AddrSize};
    /// PIM-SM group "header" size [12 bytes]
    /// The "header" has the following structure:
    ///  +--------------------------------------------------
    ///  | Multicast Group Address (Encoded-Group format, for IPv4 8 bytes)
    ///  +-------------------------------------------------------------------+
    ///  | Number of Joined Srcs (2 bytes) | Number of Prunes Srcs (2 bytes) |
    ///  +-------------------------------------------------------------------+
    static constexpr size_t GrpHdrSize{GrpASize + 2ul + 2ul};
    /// PIM-SM encoded IPv4 source address size [8 bytes]
    static constexpr size_t SrcASize{PIMSMv2EncSrcAddrSize};
    /// The minimum size of the group entry, i.e. group header and just one
    /// join or prune entry [20 bytes]
    static constexpr size_t MinEntrySize{GrpHdrSize + SrcASize};

    static constexpr size_t MaxPruneSGrptLen{PIMSMv2IPv4MaxPruneSGrptLen};
};

}  // namespace pimc::pimsm
