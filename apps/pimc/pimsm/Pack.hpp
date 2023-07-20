#include <deque>
#include <algorithm>

#include "pimc/net/IP.hpp"

#include "config/PIMSMParams.hpp"
#include "config/JPConfig.hpp"
#include "Update.hpp"

namespace pimc {

template <IPVersion V>
std::vector<Update<V>> pack(JPConfig<typename IP<V>::Address> const&);

namespace pimsm_detail {

template<IPVersion> class UpdatePacker;

template <IPVersion V>
class GroupEntryBuilder final {
    using IPAddress = typename IP<V>::Address;

    template<IPVersion> friend class UpdatePacker;
private:
    GroupEntryBuilder(IPAddress group, size_t jcnt, size_t pcnt)
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
    std::vector<IPAddress> joins_;
    std::vector<IPAddress> prunes_;
};

template <IPVersion V>
class UpdateBuilder final {
    template<IPVersion> friend class UpdatePacker;
private:
    using IPAddress = typename IP<V>::Address;

    explicit UpdateBuilder(): sz_{0ul} {}

    void add(GroupEntry<V> group, size_t sz) {
        chkSz(sz);
        groups_.emplace_back(std::move(group));
        sz_ += sz;
    }

    [[nodiscard]]
    size_t remaining() const {
        return static_cast<size_t>(pimsm::params<V>::capacity - sz_);
    }

    Update<V> build() {
        return {std::move(groups_)};
    }

    void chkSz(size_t sz) {
        if (sz_ + sz > pimsm::params<V>::capacity)
            raise<std::logic_error>(
                    "pim-update capacity {}, current size {}, update size {}",
                    pimsm::params<V>::capacity, sz_, sz);
    }

    [[nodiscard]]
    bool empty() const { return groups_.empty(); }
private:
    std::vector<GroupEntry<V>> groups_;
    size_t sz_;
};


template<IPVersion V>
class UpdatePacker final {
    using IPAddress = typename IP<V>::Address;

    template <IPVersion U>
    friend std::vector<Update<U>> pimc::pack(JPConfig<U> const&);

    class UBCursor final {
        template <IPVersion> friend class UpdatePacker;

        constexpr UBCursor(std::deque<UpdateBuilder<V>>* ubq, size_t& start)
        : ubq_{ubq}, start_{start}, i_{start} {}

        auto operator++ () -> UBCursor& {
            if (++i_ > ubq_->size())
                ubq_->emplace_back();
            return *this;
        }

        auto operator* () -> UpdateBuilder<V>& {
            return (*ubq_)[i_];
        }

        auto operator-> () -> UpdateBuilder<V>* {
            return addr();
        }

        auto addr() -> UpdateBuilder<V>* {
            return &((*ubq_)[i_]);
        }

        void add(GroupEntryBuilder<V> geb) {
            auto geSz = geb.size();
            (*ubq_)[i_].add(geb.build(), geSz);
            updateStart();
        }

        void updateStart() {
            for (size_t j = start_; j <= i_; ++j) {
                if ((*ubq_)[j].remaining() < pimsm::params<V>::MinEntrySize)
                    start_ = j;
                else return;
            }

            if (start_ == ubq_->size())
                ubq_->emplace_back();
        }

        std::deque<UpdateBuilder<V>>* ubq_;
        size_t& start_;
        size_t i_;
    };
private:
    UpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    void pack(JPConfig<V> const& jpCfg) {
        for (auto const& ge: jpCfg.groups()) fitGroupEntry(ge);
    }

    UpdateBuilder<V>* findRptUb(GroupEntry<V> const& ge) {
        if (not ge.rpt()) return nullptr;
        size_t rptSz{
            pimsm::params<V>::GrpHdrSize +
            pimsm::params<V>::SrcASize * (ge.rpt().prunes() + 1)};
        UBCursor c{&ubq_, start_};
        while (c->remaining() < rptSz) ++c;
        return c.addr();
    }

    static size_t maxSources(size_t rem) {
        return (rem - pimsm::params<V>::GrpHdrSize) / pimsm::params<V>::SrcASize;
    }

    void fitGroupEntry(GroupEntry<V> const& ge) {
        auto* rptUb = findRptUb(ge);

        UBCursor c{&ubq_, start_};
        auto const& spt = ge.spt();
        size_t srci{0};

        while (srci < spt.size()) {
            if (c.addr() != rptUb) {
                auto cnt = std::max(
                        maxSources(c->remaining()),
                        spt.size() - srci);
                GroupEntryBuilder<V> geb{ge.group(), cnt, 0};
                for (size_t i{srci}; i < cnt; ++i)
                    geb.join(spt[i], false, false);
                c.add(geb);
                srci += cnt;
            } else {
                auto cnt = std::max(
                        maxSources(
                                c->remaining() -
                                (pimsm::params<V>::GrpHdrSize +
                                 pimsm::params<V>::SrcASize * (ge.rpt().prunes() + 1))),
                        spt.size() - srci);
                GroupEntryBuilder<V> geb{ge.group(), cnt + 1, ge.rpt().prunes()};
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
            GroupEntryBuilder<V> geb{ge.group(), 1, ge.rpt().prunes()};
            geb.join(ge.rpt().rp(), true, true);
            auto const& rptPrunes = ge.rpt().prunes();
            for (auto const& rptPruneSrc: rptPrunes)
                geb.prune(rptPruneSrc, false, true);
            auto geSz = geb.size();
            rptUb->add(geb.build(), geSz);
        }
    }

    std::vector<Update<V>> build() {
        // TODO is it possible there there are multiple empty builders at
        //      the end? Unlikely, but need to make sure
        size_t resSz = ubq_.size();
        if (ubq_[ubq_.size() - 1].empty())
            --resSz;
        std::vector<Update<V>> updates;
        updates.reserve(resSz);
        std::transform(
                ubq_.begin(), ubq_.end(),
                std::back_inserter(updates),
                [] (auto& ub) { return ub.build(); });
        return updates;
    }

private:
    std::deque<UpdateBuilder<V>> ubq_;
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
