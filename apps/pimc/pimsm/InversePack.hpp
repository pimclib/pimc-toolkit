#include <deque>

#include "pimc/net/IP.hpp"
#include "pimc/system/Exceptions.hpp"

#include "config/PIMSMParams.hpp"
#include "config/JPConfig.hpp"
#include "Update.hpp"
#include "UBCursor.hpp"

namespace pimc {

template <IPVersion V>
std::vector<Update<V>> inversePack(JPConfig<V> const&);

namespace pimsm_detail {

template<IPVersion> class UpdatePacker;

template <IPVersion V>
class InverseGroupEntryBuilderImpl final {
    using IPAddress = typename IP<V>::Address;

public:

    InverseGroupEntryBuilderImpl(IPAddress group, size_t pcnt)
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
class InverseUpdateBuilderImpl final {
public:
    using IPAddress = typename IP<V>::Address;

    explicit InverseUpdateBuilderImpl(): sz_{0ul} {}

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
    friend std::vector<Update<U>> pimc::inversePack(const JPConfig<V> &);

private:
    InverseUpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    void pack(JPConfig<V> const& jpCfg) {
        for (auto const& ge: jpCfg.groups()) fitGroup(ge);
    }

    static size_t maxSources(size_t rem) {
        if (rem <= pimsm::params<V>::GrpHdrSize) return 0;
        return (rem - pimsm::params<V>::GrpHdrSize) / pimsm::params<V>::SrcASize;
    }

    void fitGroup(GroupConfig<V> const& ge) {
        UBCursor c{&ubq_, start_};
        unsigned pruneRP{ge.rpt().has_value() ? 1 : 0};
        auto const& spt = ge.spt();
        size_t srci{0};

        while (srci < spt.size()) {
            auto cnt = std::min(
                    maxSources(c->remaining()),
                    pruneRP + spt.size() - srci);
            if (cnt > 0) {
                InverseGroupEntryBuilderImpl<V> geb{ge.group(), cnt, 0};
                if (pruneRP == 1) {
                    geb.prune(ge.rpt().value().rp(), true, true);
                    pruneRP = 0;
                    if (--cnt == 0)
                        continue;
                }

                for (size_t i{srci}; i < srci + cnt; ++i)
                    geb.prune(spt[i], false, false);
                c.add(geb);
                srci += cnt;
            }
            ++c;
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
    std::deque<InverseUpdateBuilderImpl<V>> ubq_;
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
