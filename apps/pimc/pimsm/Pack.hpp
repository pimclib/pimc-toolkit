#pragma once

#include <deque>

#include "pimc/net/IP.hpp"
#include "pimc/system/Exceptions.hpp"

#include "config/JPConfig.hpp"
#include "PIMSMParams.hpp"
#include "Update.hpp"
#include "UBCursor.hpp"

namespace pimc {

template <IPVersion V>
std::vector<Update<V>> pack(JPConfig<V> const&);

namespace pimsm_detail {

template<IPVersion> class UpdatePacker;

template <IPVersion V>
class GroupEntryBuilderImpl final {
    using IPAddress = typename IP<V>::Address;

public:
    GroupEntryBuilderImpl(IPAddress group, size_t jcnt, size_t pcnt)
    : group_{group} {
        joins_.reserve(jcnt);
        prunes_.reserve(pcnt);
    }

    void join(IPAddress src, bool wildcard, bool rpt) {
        joins_.emplace_back(src, wildcard, rpt);
    }

    void prune(IPAddress src, bool wildcard, bool rpt) {
        prunes_.emplace_back(src, wildcard, rpt);
    }

    [[nodiscard]]
    size_t size() const {
        return pimsm::params<V>::GrpHdrSize +
                (joins_.size() + prunes_.size()) * pimsm::params<V>::SrcASize;
    }

    GroupEntry<V> build() {
        return {group_, std::move(joins_), std::move(prunes_)};
    }

private:
    IPAddress group_;
    std::vector<Source<V>> joins_;
    std::vector<Source<V>> prunes_;
};

template <IPVersion V>
class UpdateBuilderImpl final {
public:
    using IPAddress = typename IP<V>::Address;

    explicit UpdateBuilderImpl(): sz_{0ul} {}

    void add(GroupEntry<V> group, size_t sz) {
        chkSz(sz);
        groups_.emplace_back(std::move(group));
        sz_ += sz;
    }

    [[nodiscard]]
    size_t size() const { return sz_; }

    [[nodiscard]]
    size_t remaining() const {
        return static_cast<size_t>(pimsm::params<V>::JPCapacity - sz_);
    }

    Update<V> build() {
        return Update<V>{std::move(groups_)};
    }

    void chkSz(size_t sz) {
        if (sz_ + sz > pimsm::params<V>::JPCapacity)
            raise<std::logic_error>(
                    "pim-update capacity {}, current size {}, update size {}",
                    pimsm::params<V>::JPCapacity, sz_, sz);
    }

    [[nodiscard]]
    bool empty() const { return groups_.empty(); }

    [[nodiscard]]
    bool full() const { return remaining() < pimsm::params<V>::MinEntrySize; };
private:
    std::vector<GroupEntry<V>> groups_;
    size_t sz_;
};


template<IPVersion V>
class UpdatePacker final {
    using IPAddress = typename IP<V>::Address;

    template <IPVersion U>
    friend std::vector<Update<U>> pimc::pack(JPConfig<U> const&);

private:
    UpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    void pack(JPConfig<V> const& jpCfg) {
        for (auto const& ge: jpCfg.groups()) fitGroup(ge);
    }

    UpdateBuilderImpl<V>* findRptUb(GroupConfig<V> const& ge) {
        if (not ge.rpt()) return nullptr;
        auto const& rpt = ge.rpt().value();
        size_t rptSz{
            pimsm::params<V>::GrpHdrSize +
            pimsm::params<V>::SrcASize * (rpt.prunes().size() + 1)};
        UBCursor c{&ubq_, start_};
        while (c->remaining() < rptSz) ++c;
        return c.addr();
    }

    static size_t maxSources(size_t rem) {
        if (rem <= pimsm::params<V>::GrpHdrSize) return 0;
        return (rem - pimsm::params<V>::GrpHdrSize) / pimsm::params<V>::SrcASize;
    }

    void fitGroup(GroupConfig<V> const& ge) {
        auto* rptUb = findRptUb(ge);

        UBCursor c{&ubq_, start_};
        auto const& spt = ge.spt();
        size_t srci{0};

        while (srci < spt.size()) {
            if (c.addr() != rptUb) {
                auto cnt = std::min(
                        maxSources(c->remaining()),
                        spt.size() - srci);
                if (cnt > 0) {
                    GroupEntryBuilderImpl<V> geb{ge.group(), cnt, 0};
                    for (size_t i{srci}; i < srci + cnt; ++i)
                        geb.join(spt[i], false, false);
                    c.add(geb);
                    srci += cnt;
                }
            } else {
                auto const& rpt = ge.rpt().value();
                auto cnt = std::min(
                        maxSources(
                                c->remaining() -
                                (pimsm::params<V>::SrcASize * (rpt.prunes().size() + 1))),
                        spt.size() - srci);
                GroupEntryBuilderImpl<V> geb{
                    ge.group(), cnt + 1, rpt.prunes().size()};
                for (size_t i{srci}; i < srci + cnt; ++i)
                    geb.join(spt[i], false, false);
                geb.join(rpt.rp(), true, true);
                for (auto const& rptPruneSrc: rpt.prunes())
                    geb.prune(rptPruneSrc, false, true);
                c.add(geb);
                srci += cnt;
                rptUb = nullptr;
            }
            ++c;
        }

        if (rptUb != nullptr) {
            auto const& rpt = ge.rpt().value();
            GroupEntryBuilderImpl<V> geb{ge.group(), 1, rpt.prunes().size()};
            geb.join(rpt.rp(), true, true);
            for (auto const& rptPruneSrc: rpt.prunes())
                geb.prune(rptPruneSrc, false, true);
            auto geSz = geb.size();
            rptUb->add(geb.build(), geSz);
        }
    }

    std::vector<Update<V>> build() {
        size_t resSz = ubq_.size();
        if (ubq_[resSz - 1].empty())
            --resSz;
        std::vector<Update<V>> updates;
        updates.reserve(resSz);
        for (size_t i = 0; i < resSz; ++i)
            updates.emplace_back(ubq_[i].build());
        return updates;
    }

private:
    std::deque<UpdateBuilderImpl<V>> ubq_;
    std::size_t start_;
};

} // namespace pimc::pimsm_detail

template <IPVersion V>
inline std::vector<Update<V>> pack(JPConfig<V> const& jpCfg) {
    pimsm_detail::UpdatePacker<V> up{};
    up.pack(jpCfg);
    return up.build();
}

} // namespace pimc
