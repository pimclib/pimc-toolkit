#include <deque>
#include <algorithm>

#include "pimc/packets/PIMSMv2.hpp"

#include "config/JPConfig.hpp"
#include "Update.hpp"

namespace pimc {

template <net::IPAddress A>
std::vector<Update<A>> pack(JPConfig<A> const&);

namespace pimsm_detail {

template <net::IPAddress A>
struct params {};

template <>
struct params<net::IPv4Address> {
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
    static constexpr size_t GrpASize{PIMSMv2EncUIPv4AddrSize};
    /// PIM-SM group "header" size. The "header" has the following structure:
    ///  +--------------------------------------------------
    ///  | Multicast Group Address (Encoded-Group format) [12 bytes]
    ///  +-------------------------------------------------------------------+
    ///  | Number of Joined Srcs (2 bytes) | Number of Prunes Srcs (2 bytes) |
    ///  +-------------------------------------------------------------------+
    static constexpr size_t GrpHdrSize{GrpASize + 2ul + 2ul};
    /// PIM-SM encoded IPv4 source address size [8 bytes]
    static constexpr size_t SrcASize{PIMSMv2EncSrcAddrSize};
    /// The minimum size of the group entry, i.e. group header and just one
    /// join or prune entry
    static constexpr size_t MinEntrySize{GrpHdrSize + SrcASize};
};

template <net::IPAddress A>
class UpdateBuilder final {
public:
    UpdateBuilder(): sz_{0ul} {}

    void join(GroupEntry<A> group, size_t sz) {
        chkSz(sz);
        joins_.emplace_back(std::move(group));
        sz_ += sz;
    }

    void prune(GroupEntry<A> group, size_t sz) {
        chkSz(sz);
        prunes_.emplace_back(std::move(group));
        sz_ += sz;
    }

    [[nodiscard]]
    size_t remaining() const {
        return static_cast<size_t>(params<A>::capacity - sz_);
    }

    Update<A> build() {
        std::vector<GroupEntry<A>> joins;
        std::vector<GroupEntry<A>> prunes;

        joins.reserve(joins_.size());
        std::ranges::copy(joins_, std::back_inserter(joins));
        prunes.reserve(prunes_.size());
        std::ranges::copy(joins_, std::back_inserter(joins));
        return {std::move(joins), std::move(prunes)};
    }
private:
    void chkSz(size_t sz) {
        if (sz_ + sz > params<A>::capacity)
            raise<std::logic_error>(
                    "pim-update capacity {}, current size {}, update size {}",
                    params<A>::capacity, sz_, sz);
    }
private:
    std::deque<GroupEntry<A>> joins_;
    std::deque<GroupEntry<A>> prunes_;
    size_t sz_;
};


template<net::IPAddress A>
class UpdatePacker final {
    template <net::IPAddress Addr>
    friend std::vector<Update<Addr>> pimc::pack(JPConfig<A> const&);

private:
    UpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    static size_t rptSize(GroupEntry<A> const& ge) {
        if (not ge.rpt()) return 0;
        return params<A>::GrpHdrSize + params<A>::SrcASize * (ge.rpt().prunes() + 1);
    }

    void pack(JPConfig<A> const& jpCfg) {
        for (auto const& ge: jpCfg.groups()) fitGroupEntry(ge);
    }

    void fitGroupEntry(GroupEntry<A> const& ge) {
        auto rptSz = rptSize(ge);

        // Find an entry for RPT, e.g. entry j
        // Starting at the first builder and checking all builders until all
        // RPT and SPT entries can be placed, do the following
        // Assuming the current entry is i
        // if i != j: insert as many SPT entries as possible to builder i
        // if i == j: subtract the rpt size from the remaining bytes in builder
        //            and check how many SPT entries can be inserted, and then
        //            insert RPT and SPT entries
        // if i never reaches j, insert the RPT entries by themselves
    }

    std::vector<Update<A>> build() {

    }

private:
    std::deque<UpdateBuilder<A>> ubq_;
    std::size_t start_;
};


} // namespace pimc::pimsm_detail

template <net::IPAddress A>
inline std::vector<Update<A>> pack(JPConfig<A> const& jpCfg) {
    pimsm_detail::UpdatePacker<A> up{};
    up.pack(jpCfg);
    return up.build();
}

} // namespace pimc
