#pragma once

#include "config/ConfigUtils.hpp"
#include "config/MulticastConfigLoader.hpp"

namespace pimc::pimsm_config {

template<IPVersion V>
class PacketGroupConfigBuilder final :
        public GroupConfigBuilderBase<V, PacketGroupConfigBuilder<V>> {
public:
    using IPAddress = typename IP<V>::Address;
    using GroupConfigBuilderBase<V, PacketGroupConfigBuilder<V>>::GroupConfigBuilderBase;

    void acceptRP(IPAddress rp) { rp_ = rp; }

    void acceptPruneSGrpt(IPAddress sa) { rptPrunedSources_.emplace(sa); }

    void acceptJoinSG(IPAddress sa) { sgJoinedSources_.emplace(sa); }

    [[nodiscard]]
    GroupConfig<V> build(IPAddress group) const {
        std::optional<RPT<V>> rpt;
        if (rp_)
            rpt.emplace(rp_.value(), std::move(rptPrunedSources_));

        return GroupConfig<V>{
                group, std::move(rpt), std::move(sgJoinedSources_)};
    }

private:
    std::optional<IPAddress> rp_;
    std::vector<IPAddress> rptPrunedSources_;
    std::vector<IPAddress> sgJoinedSources_;
};

template<IPVersion V>
auto loadPacketConfig(yaml::ValueContext const &jpCfgCtx)
-> Result<JPConfig<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    MulticastConfigBuilder<V, PacketGroupConfigBuilder<V>> pktCfgBuilder{errors};
    pktCfgBuilder.loadMulticastConfig(jpCfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return pktCfgBuilder.build();
}

template<IPVersion V>
class PackingVerifierConfigBuilder final: BuilderBase {
public:
    using BuilderBase::BuilderBase;

    void loadPackingVerifierConfig(yaml::ValueContext const& vCtx) {
        auto rPktCfgList = chk(vCtx.getSequence("packet config"));

        if (rPktCfgList) {
            packetConfigs_.reserve(rPktCfgList->size());

            for (auto const& vPktCfgCtx: rPktCfgList->list()) {
                auto rPktCfg = chkErrors(loadPacketConfig<V>(vPktCfgCtx));
                if (rPktCfg)
                    packetConfigs_.emplace_back(rPktCfg->build());
            }
        }
    }

    std::vector<JPConfig<V>> build() {
        return std::move(packetConfigs_);
    }
private:
    std::vector<JPConfig<V>> packetConfigs_;
};

template<IPVersion V>
auto loadPackingVerifierConfig(yaml::ValueContext const &pvCfgCtx)
-> Result<std::vector<JPConfig<V>>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PacketGroupConfigBuilder<V> pvfb{errors};
    pvfb.loadPackingVerifierConfig(pvCfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return pvfb.build();
}

} // namespace pimc::pimsm_config
