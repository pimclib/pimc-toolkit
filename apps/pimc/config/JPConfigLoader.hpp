#pragma once

#include <set>
#include <algorithm>

#include "MulticastConfigLoader.hpp"

namespace pimc {

template <IPVersion V>
class JPGroupConfigBuilder final:
        public GroupConfigBuilderBase<V, JPGroupConfigBuilder<V>> {
public:
    using IPAddress = typename IP<V>::Address;
    using GroupConfigBuilderBase<V, JPGroupConfigBuilder<V>>::GroupConfigBuilderBase;

    void acceptRP(IPAddress rp) { rp_ = rp; }

    void acceptPruneSGrpt(IPAddress sa) { rptPrunedSources_.emplace(sa); }

    void acceptJoinSG(IPAddress sa) { sgJoinedSources_.emplace(sa); }

    [[nodiscard]]
    GroupConfig<V> build(IPAddress group) const {
        std::optional<RPT<V>> rpt;
        if (rp_) {
            std::vector<IPAddress> rptPrunedSources;
            rptPrunedSources.reserve(rptPrunedSources_.size());
            std::copy(
                    rptPrunedSources_.begin(), rptPrunedSources_.end(),
                    std::back_inserter(rptPrunedSources));
            rpt.emplace(rp_.value(), std::move(rptPrunedSources));
        }
        std::vector<IPAddress> sgJoinedSources;
        sgJoinedSources.reserve(sgJoinedSources_.size());
        std::copy(
                sgJoinedSources_.begin(), sgJoinedSources_.end(),
                std::back_inserter(sgJoinedSources));
        return GroupConfig<V>{
            group, std::move(rpt), std::move(sgJoinedSources)};
    }
private:
    std::optional<IPAddress> rp_;
    std::set<IPAddress> rptPrunedSources_;
    std::set<IPAddress> sgJoinedSources_;
};

template <IPVersion V>
auto loadJPConfig(yaml::ValueContext const& jpCfgCtx)
-> Result<JPConfig<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    MulticastConfigBuilder<V, JPGroupConfigBuilder<V>> jpCfgBuilder{errors};
    jpCfgBuilder.loadMulticastConfig(jpCfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return jpCfgBuilder.build();
}


} // namespace pimc
