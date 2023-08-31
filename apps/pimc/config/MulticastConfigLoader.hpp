#include <concepts>
#include <unordered_map>
#include <stdexcept>

#include "pimc/net/IP.hpp"
#include "pimc/parsers/IPParsers.hpp"
#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/IPFormatters.hpp"

#include "pimc/yaml/BuilderBase.hpp"
#include "ConfigUtils.hpp"
#include "PIMSMParams.hpp"
#include "JPConfig.hpp"

namespace pimc {

template <IPVersion V>
Result<typename IP<V>::Address, std::string> grpAddr(std::string const &g) {
    auto oga = parse<V>::address(g);
    if (not oga)
        return fail(fmt::format("invalid multicast {} group address '{}'", V{}, g));

    auto ga = oga.value();
    if (not ga.isMcast())
        return fail(fmt::format(
                "invalid multicast {} group address {}: not multicast", V{}, ga));

    return ga;
}

template <typename GCB, typename V>
concept GroupConfigBuilder =
IPVersion<V> and requires(
        GCB gcb,
        std::vector<yaml::ErrorContext> &errors,
        typename IP<V>::Address ga,
        int line,
        typename IP<V>::Address sa) {
    GCB{errors, ga, line};
    gcb.acceptRP(sa);
    gcb.acceptPruneSGrpt(sa);
    gcb.acceptJoinSG(sa);
    { gcb.build(ga) } -> std::same_as<GroupConfig<V>>;
};

template <IPVersion V, typename GCB>
class GroupConfigBuilderBase: BuilderBase {
public:
    using IPAddress = typename IP<V>::Address;

    GroupConfigBuilderBase(
            std::vector<yaml::ErrorContext> &errors,
            IPAddress group,
            int line)
            : BuilderBase{errors}
            , group_{group}
            , line_{line}
            , rptPrunedSources_{0ul} {}

    void loadGroupConfig(yaml::ValueContext const &grpCtx) {
        auto rGrpCfg = chk(
                grpCtx.getMapping(fmt::format("{} group {} config", V{}, group_)));

        if (rGrpCfg) {
            bool rptCfg{false}, sptCfg{false};

            auto oRptCfg = rGrpCfg->optional("Join*");
            if (oRptCfg) {
                rptCfg = true;
                loadRPTConfig(oRptCfg.value());
            }

            auto oSptCfg = rGrpCfg->optional("Join");
            if (oSptCfg) {
                sptCfg = true;
                loadSPTConfig(oSptCfg.value());
            }

            chkExtraneous(rGrpCfg.value());

            if ((not rptCfg) and (not sptCfg))
                errors_.emplace_back(
                        grpCtx.error(
                                "{} group {} config may not be empty", V{}, group_));
        }
    }

    [[nodiscard]]
    int line() const { return line_; }

    [[nodiscard]]
    GroupConfig<V> build() { return impl()->build(group_); }

private:
    template <typename Self = GCB>
    Self* impl() requires GroupConfigBuilder<Self, V> {
        return static_cast<Self*>(this);
    }

    // The handler for the "Join*" entry:
    void loadRPTConfig(yaml::ValueContext const &rptCtx) {
        auto rRptCfg = chk(rptCtx.getMapping(fmt::format("{} RPT config", V{})));

        if (rRptCfg) {
            // Rendezvous Point
            auto rRP = chk(rRptCfg->required("RP")
                    .flatMap(yaml::scalar(fmt::format("{} RP address", V{}))));

            if (rRP) {
                auto rRPA = chk(
                        ucAddr<V>(rRP->value(), UCAddrType::RP).mapError(
                                [&rRP](auto msg) { return rRP->error(std::move(msg)); }));

                if (rRPA) {
                    if (chkSrc(rRP.value(), rRPA.value(), JPSourceType::RP))
                        impl()->acceptRP(rRPA.value());
                }
            }

            // Optional RPT-pruned sources
            auto oPruneSGRptList = rRptCfg->optional("Prune");
            if (oPruneSGRptList) {
                auto rPruneSGRptList =
                        chk(oPruneSGRptList->getSequence("RPT-pruned sources"));

                if (rPruneSGRptList) {
                    for (auto const &vCtx: rPruneSGRptList->list()) {
                        auto rSrc = chk(
                                vCtx.getScalar(fmt::format("{} source address", V{})));

                        if (rSrc) {
                            auto rSrcA = chk(
                                    ucAddr<V>(rSrc->value(), UCAddrType::Source)
                                            .mapError(
                                                    [&rSrc](auto msg) {
                                                        return rSrc->error(std::move(msg));
                                                    }));

                            if (rSrcA) {
                                auto src = rSrcA.value();
                                if (not chkSrc(
                                        rSrc.value(),
                                        src,
                                        JPSourceType::RptPruned))
                                    continue;

                                if (rptPrunedSources_ >= pimsm::params<V>::MaxPruneSGrptLen) {
                                    errors_.emplace_back(
                                            rSrc->error(
                                                    "unable to add source {} "
                                                    "to the RPT-prune list for group {} "
                                                    "as it exceeds the maximum number of "
                                                    "entries {}",
                                                    src, group_, rptPrunedSources_));
                                    continue;
                                }

                                ++rptPrunedSources_;
                                impl()->acceptPruneSGrpt(src);
                            }
                        }
                    } // loop over Prune(S,G,rpt)
                }
            } // Prune entry exists

            chkExtraneous(rRptCfg.value());
        }
    }

    // The handler for the "Join" entry:
    void loadSPTConfig(yaml::ValueContext const &sptCtx) {
        auto rSptCfg = chk(sptCtx.getSequence(fmt::format("{} SPT config", V{})));

        if (rSptCfg) {
            for (auto const &vCtx: rSptCfg->list()) {
                auto rSrc = chk(vCtx.getScalar(fmt::format("{} source address", V{})));

                if (rSrc) {
                    auto rSrcA = chk(
                            ucAddr<V>(rSrc->value(), UCAddrType::Source)
                                    .mapError(
                                            [&rSrc](auto msg) {
                                                return rSrc->error(msg);
                                            }));

                    if (rSrcA) {
                        auto src = rSrcA.value();
                        if (not chkSrc(
                                rSrc.value(),
                                src,
                                JPSourceType::SptJoined))
                            continue;

                        impl()->acceptJoinSG(src);
                    }
                }
            }
        }
    }

    bool chkSrc(
            yaml::NodeContext &nctx, IPAddress src, JPSourceType jpst) {
        auto ii = sources_.try_emplace(
                src, JPSourceInfo{.type_ = jpst, .line_ = nctx.line()});

        if (not ii.second) {
            JPSourceInfo eJpsi = ii.first->second;

            errors_.emplace_back(
                    nctx.error(
                            "duplicate {} {}: declared as {} on line {}",
                            src, jpst, eJpsi.type_, eJpsi.line_));
            return false;
        }

        return true;
    }

    IPAddress group_;
    int line_;
    size_t rptPrunedSources_;
    std::unordered_map<IPAddress, JPSourceInfo> sources_;
};

template <IPVersion V, GroupConfigBuilder<V> GCB>
class MulticastConfigBuilder final : BuilderBase {
    using IPAddress = typename IP<V>::Address;
public:
    using BuilderBase::BuilderBase;

    void loadMulticastConfig(yaml::ValueContext const &jpCfgCtx) {
        auto rJPCfg = chk(jpCfgCtx.getMapping(fmt::format("{} multicast config", V{})));

        if (rJPCfg) {
            size_t sz = rJPCfg->size();

            if (sz > 0) {
                for (auto const &ii: rJPCfg->items()) {
                    auto rGrp = chk(ii.first.getScalar());

                    if (rGrp) {
                        auto rGrpA = chk(grpAddr<V>(
                                rGrp->value()).mapError(
                                [&rGrp](auto msg) {
                                    return rGrp->error(msg);
                                }));

                        if (rGrpA) {
                            IPAddress ga = rGrpA.value();

                            auto bldi = groupBldMap_.try_emplace(
                                    ga, errors_, ga, rGrp->line());
                            if (bldi.second) {
                                groups_.emplace_back(ga);
                                bldi.first->second.loadGroupConfig(ii.second);
                            } else {
                                auto const &eBld = bldi.first->second;
                                errors_.emplace_back(
                                        rGrp->error(
                                                "duplicate group {}, "
                                                "previously declared on line {}",
                                                ga, eBld.line()));
                            }
                        }
                    }
                }
            } else
                errors_.emplace_back(
                        jpCfgCtx.error(
                                fmt::format("{} J/P config contains no groups", V{})));
        }
    }

    JPConfig<V> build() {
        std::vector<GroupConfig<V>> groups;
        groups.reserve(groups_.size());
        std::transform(groups_.begin(), groups_.end(),
                       std::back_inserter(groups),
                       [this](auto const &ga) {
                           if (auto ii = groupBldMap_.find(ga);
                               ii != groupBldMap_.end())
                               return ii->second.build(ga);
                           throw std::logic_error{"indexed group not in map"};
                       });
        return JPConfig<V>{std::move(groups)};
    }

private:
    std::vector<IPAddress> groups_;
    std::unordered_map<IPAddress, GCB> groupBldMap_;
};

} // namespace pimc
