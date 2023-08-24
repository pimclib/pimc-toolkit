#pragma once

#include "pimc/net/IP.hpp"

#include "config/PIMSMParams.hpp"
#include "config/ConfigUtils.hpp"
#include "pimsm/Update.hpp"

namespace pimc {

template <IPVersion V>
class GroupEntryBuilder final: BuilderBase {
    using IPAddress = typename IP<V>::Address;
public:
    GroupEntryBuilder(
            std::vector<yaml::ErrorContext>& errors,
            IPAddress group,
            int line)
            : BuilderBase{errors}
            , group_{group}
            , line_{line}
            , rpJoined_{false}
            , joining_{true}
            , rptPrunedSources_{0} {}

    void loadGroupEntryConfig(yaml::ValueContext const& geCfgCtx) {
        auto rGECfg =
                chk(geCfgCtx.getMapping(fmt::format("{} group entry config", V{})));

        if (rGECfg) {
            for (auto const& ii: rGECfg->items()) {
                auto ost = sourceListType(ii.first);
                if (ost) {
                    auto st = ost.value();
                    auto rlst = chk(ii.second.getSequence(fmt::format("{} list", st)));

                    if (rlst)
                        processList(st, rlst.value());
                }
            }
        }
    }

    [[nodiscard]]
    int line() const { return line_; }

    GroupEntry<V> build() {
        return {group_, std::move(joins_), std::move(prunes_)};
    }

private:
    Optional<JPSourceType> sourceListType(yaml::ValueContext const& vCtx) {
        auto rfn = chk(vCtx.getScalar("source list name"));
        if (rfn) {
            auto const& fn = rfn->value();
            if (fn == "Join(*,G)")
                return JPSourceType::RP;
            if (fn == "Join(S,G)")
                return JPSourceType::SptJoined;
            if (fn == "Prune(S,G,rpt)")
                return JPSourceType::RptPruned;

            errors_.emplace_back(rfn->error(
                    "unrecognized source list '{}', allowed fields are "
                    "Join(*,G), Join(S,G) and Prune(S,G,rpt)"));
        }
        return {};
    }

    void processList(JPSourceType st, yaml::SequenceContext const& lst) {
        UCAddrType uat;
        if (st == JPSourceType::RP) {
            uat = UCAddrType::RP;
            if (lst.size() != 1) {
                consume(lst.error(
                        "Join(*,G) list must contain exactly 1 entry, not {}",
                        lst.size()));
                return;
            }
        } else uat = UCAddrType::Source;

        for (auto const& vCtx: lst.list()) {
            auto re = chk(vCtx.getScalar(fmt::format("{} address", st)));

            if (re) {
                auto rs = chk(
                        ucAddr<V>(re->value(), uat)
                                .mapError(
                                        [&re] (auto msg) {
                                            return re->error(std::move(msg));
                                        }));

                if (rs) {
                    auto src = rs.value();

                    if (not chkSrc(re.value(), src, st))
                        continue;

                    if (st == JPSourceType::RptPruned) {
                        joining_ = false;

                        if (not rpJoined_)
                            errors_.emplace_back(re->error(
                                    "Prune(S,G,rpt) for source {} requires a valid Join(*,G)",
                                    src));

                        // Check if not too many
                        if (rptPrunedSources_ >= pimsm::params<V>::MaxPruneSGrptLen) {
                            errors_.emplace_back(
                                    re->error(
                                            "unable to add source {} "
                                            "to the RPT-prune list for group {} "
                                            "as it exceeds the maximum number of "
                                            "entries {}",
                                            src, group_, rptPrunedSources_));
                            continue;
                        }

                        ++rptPrunedSources_;
                        prunes_.emplace_back(src, false, true);
                    } else {
                        // Check if we're still joining
                        if (not joining_)
                            errors_.emplace_back(
                                    re->error(
                                            "Join({},G) may not appear after Prune(S,G,rpt)",
                                            st == JPSourceType::RP ? '*' : 'S'));

                        if (st == JPSourceType::RP) {
                            if (rpJoined_)
                                re->error("Duplicate Join(*,G), with RP {}", src);
                            else rpJoined_ = true;
                        }

                        auto wcrpt = st == JPSourceType::RP;
                        joins_.emplace_back(src, wcrpt, wcrpt);
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

private:
    IPAddress group_;
    int line_;
    bool rpJoined_;
    bool joining_;
    size_t rptPrunedSources_;
    std::unordered_map<IPAddress, JPSourceInfo> sources_;
    std::vector<Source<V>> joins_;
    std::vector<Source<V>> prunes_;
};

template <IPVersion V>
class UpdateConfigBuilder final: BuilderBase {
    using IPAddress = typename IP<V>::Address;
public:
    using BuilderBase::BuilderBase;

    void loadUpdateConfig(yaml::ValueContext const &jpCfgCtx) {
        auto rUpdateCfg =
                chk(jpCfgCtx.getMapping(fmt::format("{} update config", V{})));

        if (rUpdateCfg) {
            size_t sz = rUpdateCfg->size();

            if (sz > 0) {
                for (auto const &ii: rUpdateCfg->items()) {
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
                                bldi.first->second.loadGroupEntryConfig(ii.second);
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
                        jpCfgCtx.error("IPv4 J/P config contains no groups"));
        }
    }

    Update<V> build() {
        std::vector<GroupEntry<V>> groups;
        groups.reserve(groups_.size());
        std::transform(groups_.begin(), groups_.end(),
                       std::back_inserter(groups),
                       [this](auto const &ga) {
                           if (auto ii = groupBldMap_.find(ga);
                                   ii != groupBldMap_.end())
                               return ii->second.build();
                           throw std::logic_error{"indexed group not in map"};
                       });
        return Update<V>{std::move(groups)};
    }

private:
    std::vector<IPAddress> groups_;
    std::unordered_map<IPAddress, GroupEntryBuilder<V>> groupBldMap_;
};

template <IPVersion V>
auto loadUpdateConfig(yaml::ValueContext const& vCtx)
-> Result<Update<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    UpdateConfigBuilder<V> ucb{errors};
    ucb.loadUpdateConfig(vCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return ucb.build();
}

template<IPVersion V>
class UpdatesBuilder final: BuilderBase {
public:
    using BuilderBase::BuilderBase;

    void loadPackingVerifierConfig(yaml::ValueContext const& vCtx) {
        auto rPktCfgList = chk(vCtx.getSequence("update config"));

        if (rPktCfgList) {
            updates_.reserve(rPktCfgList->size());

            for (auto const& vPktCfgCtx: rPktCfgList->list()) {
                auto rPktCfg = chkErrors(loadUpdateConfig<V>(vPktCfgCtx));
                if (rPktCfg)
                    updates_.emplace_back(std::move(rPktCfg).value());
            }
        }
    }

    std::vector<Update<V>> build() {
        return std::move(updates_);
    }
private:
    std::vector<Update<V>> updates_;
};

template<IPVersion V>
auto loadUpdates(yaml::ValueContext const &pvCfgCtx)
-> Result<std::vector<Update<V>>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    UpdatesBuilder<V> pvfb{errors};
    pvfb.loadPackingVerifierConfig(pvCfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return pvfb.build();
}

} // namespace pimc
