#include <deque>

#include "pimc/net/IP.hpp"
#include "pimc/system/Exceptions.hpp"

#include "config/PIMSMParams.hpp"
#include "config/JPConfig.hpp"
#include "Update.hpp"

namespace pimc {

template <IPVersion V>
std::vector<Update<V>> inversePack(JPConfig<V> const&);

namespace pimsm_detail {

template<IPVersion> class UpdatePacker;

template <IPVersion V>
class InverseGroupEntryBuilder final {
    using IPAddress = typename IP<V>::Address;

    template<IPVersion> friend class UpdatePacker;
private:
    InverseGroupEntryBuilder(IPAddress group, size_t pcnt)
    : group_{group} {
        prunes_.reserve(pcnt);
    }

    void prune(IPAddress src, bool wildcard, bool rpt) {
        prunes_.emplace_back(src, wildcard, rpt);
    }

    [[nodiscard]]
    size_t size() const {
        return pimsm::params<V>::GrpHdrSize +
                (prunes_.size()) * pimsm::params<V>::SrcASize;
    }

    GroupEntry<V> build() {
        return {group_, {}, std::move(prunes_)};
    }

private:
    IPAddress group_;
    std::vector<Source<V>> prunes_;
};

template <IPVersion V>
class InverseUpdateBuilder final {
    template<IPVersion> friend class UpdatePacker;

public:
    using IPAddress = typename IP<V>::Address;

    explicit InverseUpdateBuilder(): sz_{0ul} {}

private:
    void add(GroupEntry<V> group, size_t sz) {
        chkSz(sz);
        groups_.emplace_back(std::move(group));
        sz_ += sz;
    }

    [[nodiscard]]
    size_t size() const { return sz_; }

    [[nodiscard]]
    size_t remaining() const {
        return static_cast<size_t>(pimsm::params<V>::capacity - sz_);
    }

    Update<V> build() {
        return Update<V>{std::move(groups_)};
    }

    void chkSz(size_t sz) {
        if (sz_ + sz > pimsm::params<V>::capacity)
            raise<std::logic_error>(
                    "pim-update capacity {}, current size {}, update size {}",
                    pimsm::params<V>::capacity, sz_, sz);
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
class InverseUpdatePacker final {
    using IPAddress = typename IP<V>::Address;

    template <IPVersion U>
    friend std::vector<Update<U>> pimc::inversePack(JPConfig<U> const&);

    class IUBCursor final {
        template <IPVersion> friend class InverseUpdatePacker;

        constexpr IUBCursor(std::deque<InverseUpdateBuilder<V>>* ubq, size_t& start)
        : ubq_{ubq}, start_{start}, i_{start} {}

        auto operator++ () -> IUBCursor& {
            if (++i_ >= ubq_->size())
                ubq_->emplace_back();

            return *this;
        }

        auto operator* () -> InverseUpdateBuilder<V>& {
            return (*ubq_)[i_];
        }

        auto operator-> () -> InverseUpdateBuilder<V>* {
            return addr();
        }

        auto addr() -> InverseUpdateBuilder<V>* {
            return &((*ubq_)[i_]);
        }

        void add(InverseGroupEntryBuilder<V> geb) {
            auto geSz = geb.size();
            (*ubq_)[i_].add(geb.build(), geSz);
            updateStart();
        }

        void updateStart() {
            for (size_t j = start_; j <= i_; ++j) {
                if ((*ubq_)[j].full())
                    start_ = j;
                else return;
            }

            if (start_ == ubq_->size())
                ubq_->emplace_back();
        }

        std::deque<InverseUpdateBuilder<V>>* ubq_;
        size_t& start_;
        size_t i_;
    };
private:
    InverseUpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    void pack(JPConfig<V> const& jpCfg) {
        for (auto const& ge: jpCfg.groups()) fitGroup(ge);
    }

    InverseUpdateBuilder<V>* findRptUb(GroupConfig<V> const& ge) {
        if (not ge.rpt()) return nullptr;
        auto const& rpt = ge.rpt().value();
        size_t rptSz{
            pimsm::params<V>::GrpHdrSize +
            pimsm::params<V>::SrcASize * (rpt.prunes().size() + 1)};
        IUBCursor c{&ubq_, start_};
        while (c->remaining() < rptSz) ++c;
        return c.addr();
    }

    static size_t maxSources(size_t rem) {
        if (rem <= pimsm::params<V>::GrpHdrSize) return 0;
        return (rem - pimsm::params<V>::GrpHdrSize) / pimsm::params<V>::SrcASize;
    }

    void fitGroup(GroupConfig<V> const& ge) {
        auto* rptUb = findRptUb(ge);

        IUBCursor c{&ubq_, start_};
        auto const& spt = ge.spt();
        size_t srci{0};

        while (srci < spt.size()) {
            if (c.addr() != rptUb) {
                auto cnt = std::min(
                        maxSources(c->remaining()),
                        spt.size() - srci);
                if (cnt > 0) {
                    InverseGroupEntryBuilder<V> geb{ge.group(), cnt, 0};
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
                InverseGroupEntryBuilder<V> geb{
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
            InverseGroupEntryBuilder<V> geb{ge.group(), 1, rpt.prunes().size()};
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
    std::deque<InverseUpdateBuilder<V>> ubq_;
    std::size_t start_;
};


} // namespace pimc::pimsm_detail

template <IPVersion V>
inline std::vector<Update<V>> inversePack(JPConfig<V> const& jpCfg) {
    pimsm_detail::InverseUpdatePacker<V> up{};
    up.pack(jpCfg);
    return up.build();
}

} // namespace pimc
