#pragma once

#include "pimc/net/IP.hpp"

#include "pimsm/PIMSMParams.hpp"
#include "config/ConfigUtils.hpp"
#include "pimsm/Update.hpp"

namespace pimc {

template <IPVersion V>
class InverseGroupEntryBuilder final: BuilderBase {
    using IPAddress = typename IP<V>::Address;
public:
    InverseGroupEntryBuilder(
            std::vector<yaml::ErrorContext>& errors,
            IPAddress group,
            int line)
            : BuilderBase{errors}
            , group_{group}
            , line_{line}
            , rpPruned_{false} {}

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
        return {group_, {}, std::move(prunes_)};
    }

private:
    Optional<JPSourceType> sourceListType(yaml::ValueContext const& vCtx) {
        auto rfn = chk(vCtx.getScalar("source list name"));
        if (rfn) {
            auto const& fn = rfn->value();
            if (fn == "Prune(*,G)")
                return JPSourceType::RP;
            if (fn == "Prune(S,G)")
                return JPSourceType::SptJoined;

            errors_.emplace_back(rfn->error(
                    "unrecognized source list '{}', allowed fields are "
                    "Prune(*,G) and Prune(S,G)"));
        }
        return {};
    }

    void processList(JPSourceType st, yaml::SequenceContext const& lst) {
        UCAddrType uat;
        if (st == JPSourceType::RP) {
            uat = UCAddrType::RP;
            if (lst.size() != 1) {
                consume(lst.error(
                        "Prune(*,G) list must contain exactly 1 entry, not {}",
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

                    if (st == JPSourceType::RP) {
                        if (rpPruned_)
                            re->error("Duplicate Prune(*,G), with RP {}", src);
                        else rpPruned_ = true;
                    }

                    auto wcrpt = st == JPSourceType::RP;
                    prunes_.emplace_back(src, wcrpt, wcrpt);
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
    bool rpPruned_;
    std::unordered_map<IPAddress, JPSourceInfo> sources_;
    std::vector<Source<V>> prunes_;
};

template <IPVersion V>
class InverseUpdateConfigBuilder final: BuilderBase {
    using IPAddress = typename IP<V>::Address;
public:
    using BuilderBase::BuilderBase;

    void loadInverseUpdateConfig(yaml::ValueContext const &jpCfgCtx) {
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
                        jpCfgCtx.error(
                                fmt::format("{} J/P config contains no groups", V{})));
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
    std::unordered_map<IPAddress, InverseGroupEntryBuilder<V>> groupBldMap_;
};

template <IPVersion V>
auto loadInverseUpdateConfig(yaml::ValueContext const& vCtx)
-> Result<Update<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    InverseUpdateConfigBuilder<V> ucb{errors};
    ucb.loadInverseUpdateConfig(vCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return ucb.build();
}

template<IPVersion V>
class InverseUpdatesBuilder final: BuilderBase {
public:
    using BuilderBase::BuilderBase;

    void loadPackingVerifierConfig(yaml::ValueContext const& vCtx) {
        auto rPktCfgList = chk(vCtx.getSequence("inverse update config"));

        if (rPktCfgList) {
            updates_.reserve(rPktCfgList->size());

            for (auto const& vPktCfgCtx: rPktCfgList->list()) {
                auto rPktCfg = chkErrors(loadInverseUpdateConfig<V>(vPktCfgCtx));
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
auto loadInverseUpdates(yaml::ValueContext const &pvCfgCtx)
-> Result<std::vector<Update<V>>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    InverseUpdatesBuilder<V> pvfb{errors};
    pvfb.loadPackingVerifierConfig(pvCfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return pvfb.build();
}

} // namespace pimc
