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

template<net::IPAddress> class UpdatePacker;

template <net::IPAddress A>
class GroupEntryBuilder final {
    template<net::IPAddress> friend class UpdatePacker;
private:
    GroupEntryBuilder(A group, size_t jcnt, size_t pcnt)
    : group_{group} {
        joins_.reserve(jcnt);
        prunes_.reserve(pcnt);
    }

    void join(A src, bool wildcard, bool rpt) {
        joins_.emplace_back(src, wildcard, rpt);
    }

    void prune(A src, bool wildcard, bool rpt) {
        prunes_.emplace_back(src, wildcard, rpt);
    }

    [[nodiscard]]
    size_t size() const {
        return params<A>::GrpHdrSize +
                (joins_.size() + prunes_.size()) * params<A>::SrcASize;
    }

    GroupEntry<A> build() {
        return {group_, std::move(joins_), std::move(prunes_)};
    }

private:
    A group_;
    std::vector<Source<A>> joins_;
    std::vector<Source<A>> prunes_;
};

template <net::IPAddress A>
class UpdateBuilder final {
    template<net::IPAddress> friend class UpdatePacker;
private:
    UpdateBuilder(): sz_{0ul} {}

    void add(GroupEntry<A> group, size_t sz) {
        chkSz(sz);
        groups_.emplace_back(std::move(group));
        sz_ += sz;
    }

    [[nodiscard]]
    size_t remaining() const {
        return static_cast<size_t>(params<A>::capacity - sz_);
    }

    Update<A> build() {
        return {std::move(groups_)};
    }

    void chkSz(size_t sz) {
        if (sz_ + sz > params<A>::capacity)
            raise<std::logic_error>(
                    "pim-update capacity {}, current size {}, update size {}",
                    params<A>::capacity, sz_, sz);
    }

    [[nodiscard]]
    bool empty() const { return groups_.empty(); }
private:
    std::vector<GroupEntry<A>> groups_;
    size_t sz_;
};


template<net::IPAddress A>
class UpdatePacker final {
    template <net::IPAddress Addr>
    friend std::vector<Update<Addr>> pimc::pack(JPConfig<A> const&);

    class UBCursor final {
        template <net::IPAddress>
        friend class UpdatePacker;

        constexpr UBCursor(std::deque<UpdateBuilder<A>>* ubq, size_t& start)
        : ubq_{ubq}, start_{start}, i_{start} {}

        auto operator++ () -> UBCursor& {
            if (++i_ > ubq_->size())
                ubq_->emplace_back();
            return *this;
        }

        auto operator* () -> UpdateBuilder<A>& {
            return (*ubq_)[i_];
        }

        auto operator-> () -> UpdateBuilder<A>* {
            return addr();
        }

        auto addr() -> UpdateBuilder<A>* {
            return &((*ubq_)[i_]);
        }

        void add(GroupEntryBuilder<A> geb) {
            auto geSz = geb.size();
            (*ubq_)[i_].add(geb.build(), geSz);
            updateStart();
        }

        void updateStart() {
            for (size_t j = start_; j <= i_; ++j) {
                if ((*ubq_)[j].remaining() < params<A>::MinEntrySize)
                    start_ = j;
                else return;
            }

            if (start_ == ubq_->size())
                ubq_->emplace_back();
        }

        std::deque<UpdateBuilder<A>>* ubq_;
        size_t& start_;
        size_t i_;
    };
private:
    UpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    void pack(JPConfig<A> const& jpCfg) {
        for (auto const& ge: jpCfg.groups()) fitGroupEntry(ge);
    }

    UpdateBuilder<A>* findRptUb(GroupEntry<A> const& ge) {
        if (not ge.rpt()) return nullptr;
        size_t rptSz{
            params<A>::GrpHdrSize +
            params<A>::SrcASize * (ge.rpt().prunes() + 1)};
        UBCursor c{&ubq_, start_};
        while (c->remaining() < rptSz) ++c;
        return c.addr();
    }

    static size_t maxSources(size_t rem) {
        return (rem - params<A>::GrpHdrSize) / params<A>::SrcASize;
    }

    void fitGroupEntry(GroupEntry<A> const& ge) {
        auto* rptUb = findRptUb(ge);

        UBCursor c{&ubq_, start_};
        auto const& spt = ge.spt();
        size_t srci{0};

        while (srci < spt.size()) {
            if (c.addr() != rptUb) {
                auto cnt = std::max(
                        maxSources(c->remaining()),
                        spt.size() - srci);
                GroupEntryBuilder<A> geb{ge.group(), cnt, 0};
                for (size_t i{srci}; i < cnt; ++i)
                    geb.join(spt[i], false, false);
                c.add(geb);
                srci += cnt;
            } else {
                auto cnt = std::max(
                        maxSources(
                                c->remaining() -
                                (params<A>::GrpHdrSize +
                                 params<A>::SrcASize * (ge.rpt().prunes() + 1))),
                        spt.size() - srci);
                GroupEntryBuilder<A> geb{ge.group(), cnt + 1, ge.rpt().prunes()};
                for (size_t i{srci}; i < cnt; ++i)
                    geb.join(spt[i], false, false);
                geb.join(ge.rpt().rp(), true, true);
                auto const& rptPrunes = ge.rpt().prunes();
                for (auto const& rptPruneSrc: rptPrunes)
                    geb.prune(rptPruneSrc, false, true);
                c.add(geb);
                srci += cnt;
                rptUb = nullptr;
            }
            ++c;
        }

        if (rptUb != nullptr) {
            GroupEntryBuilder<A> geb{ge.group(), 1, ge.rpt().prunes()};
            geb.join(ge.rpt().rp(), true, true);
            auto const& rptPrunes = ge.rpt().prunes();
            for (auto const& rptPruneSrc: rptPrunes)
                geb.prune(rptPruneSrc, false, true);
            auto geSz = geb.size();
            rptUb->add(geb.build(), geSz);
        }
    }

    std::vector<Update<A>> build() {
        // TODO is it possible there there are multiple empty builders at
        //      the end? Unlikely, but need to make sure
        size_t resSz = ubq_.size();
        if (ubq_[ubq_.size() - 1].empty())
            --resSz;
        std::vector<Update<A>> updates;
        updates.reserve(resSz);
        std::transform(
                ubq_.begin(), ubq_.end(),
                std::back_inserter(updates),
                [] (auto& ub) { return ub.build(); });
        return updates;
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
