#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>

#include <fmt/format.h>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"
#include "pimc/formatters/IPv4Formatters.hpp"
#include "pimc/packets/PIMSMv2.hpp"
#include "JPConfigLoader.hpp"
#include "pimc/yaml/BuilderBase.hpp"

namespace pimc {
namespace {
enum class JPSourceType: unsigned {
    RP = 0,
    RptPruned = 1,
    SptJoined = 2
};
} // anon.namespace
} // namespace pimc

namespace fmt {

template <>
struct formatter<pimc::JPSourceType>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::JPSourceType const& jpst, FormatContext& ctx) {
        switch (jpst) {

        case pimc::JPSourceType::RP:
            return fmt::format_to(ctx.out(), "RP");
        case pimc::JPSourceType::RptPruned:
            return fmt::format_to(ctx.out(), "RPT-pruned source");
        case pimc::JPSourceType::SptJoined:
            return fmt::format_to(ctx.out(), "SPT-joined source");
        }

        return fmt::format_to(
                ctx.out(), "unknown source type {}", static_cast<unsigned>(jpst));
    }
};

} // namespace fmt

namespace pimc {

namespace {

struct JPSourceInfo {
    JPSourceType type_;
    int line_;
};

std::vector<net::IPv4Address> set2vec(std::set<net::IPv4Address> const& s) {
    if (s.empty()) return {};
    std::vector<net::IPv4Address> vs;
    vs.reserve(s.size());
    for (auto const& a: s) vs.emplace_back(a);
    return vs;
}


Result<net::IPv4Address, std::string> grpAddr(std::string const& g) {
    auto oga = parseIPv4Address(g);
    if (not oga)
        return fail(fmt::format("invalid multicast IPv4 group address '{}'", g));

    net::IPv4Address ga = oga.value();
    if (not ga.isMcast())
        return fail(fmt::format(
                "invalid multicast IPv4 group address {}: not multicast", ga));

    return ga;
}

struct BuilderBase: yaml::BuilderBase<BuilderBase> {
    constexpr explicit BuilderBase(std::vector<yaml::ErrorContext>& errors)
    : errors_{errors} {}

    void chkExtraneous(yaml::MappingContext const& mCtx) {
        auto extraneous = mCtx.extraneous();
        if (not extraneous.empty()) {
            errors_.reserve(errors_.size() + extraneous.size());
            for (auto& e: extraneous)
                errors_.emplace_back(std::move(e));
        }
    }

    void consume(yaml::ErrorContext ectx) {
        errors_.emplace_back(std::move(ectx));
    }

    std::vector<yaml::ErrorContext>& errors_;
};

struct IPv4JPGroupConfigBuilder final: BuilderBase {
    IPv4JPGroupConfigBuilder(
            std::vector<yaml::ErrorContext>& errors,
            net::IPv4Address group,
            int line)
    : BuilderBase{errors}, group_{group}, line_{line} {}

    // The handler for the "Join*" entry:
    void loadRPTConfig(yaml::ValueContext const& rptCtx) {
        auto rRptCfg = chk(rptCtx.getMapping("RPT IPv4 config"));

        if (rRptCfg) {
            // Rendezvous Point
            auto rRP = chk(rRptCfg->required("RP")
                    .flatMap(yaml::scalar("RP IPv4 address")));

            if (rRP) {
                auto rRPA = chk(
                        srcAddr(rRP->value()).mapError(
                                [&rRP] (auto msg) { return rRP->error(msg); }));

                if (rRPA) {
                    chkSrc(rRP.value(), rRPA.value(), JPSourceType::RP);
                    rp_ = rRPA.value();
                }
            }

            // Optional RPT-pruned sources
            auto oPruneSGRptList = rRptCfg->optional("Prune");
            if (oPruneSGRptList) {
                auto rPruneSGRptList =
                        chk(oPruneSGRptList->getSequence("RPT-pruned sources"));

                if (rPruneSGRptList) {
                    for (auto const& vCtx: rPruneSGRptList->list()) {
                        auto rSrc = chk(vCtx.getScalar("source IPv4 address"));

                        if (rSrc) {
                            auto rSrcA = chk(
                                    srcAddr(rSrc->value()).mapError(
                                            [&rSrc] (auto msg) {
                                                return rSrc->error(msg);
                                            }));

                            if (rSrcA) {
                                auto src = rSrcA.value();
                                if (not chkSrc(
                                        rSrc.value(),
                                        src,
                                        JPSourceType::RptPruned)) continue;

                                if (pruneSGrptList_.size() > PIMSMv2IPv4MaxPruneSGrptLen) {
                                    errors_.emplace_back(
                                            rSrc->error(
                                                    "unable to add source  {} "
                                                    "to the RPT-prune list for group {} "
                                                    "as it exceeds maximum number of "
                                                    "entries {}",
                                                    src, group_, pruneSGrptList_.size()));
                                    continue;
                                }
                                pruneSGrptList_.emplace(src);
                            }
                        }
                    } // loop over Prune(S,G,rpt)
                }
            } // Prune entry exists

            chkExtraneous(rRptCfg.value());
        }
    }

    // The handler for the "Join" entry:
    void loadSPTConfig(yaml::ValueContext const& sptCtx) {
        auto rSptCfg = chk(sptCtx.getSequence("SPT IPv4 config"));

        if (rSptCfg) {
            for (auto const& vCtx: rSptCfg->list()) {
                auto rSrc = chk(vCtx.getScalar("source IPv4 address"));

                if (rSrc) {
                    auto rSrcA = chk(
                            srcAddr(rSrc->value()).mapError(
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

                        joinSGList_.emplace(src);
                    }
                }
            }
        }
    }

    void loadGroupConfig(yaml::ValueContext const& grpCtx) {
        auto rGrpCfg = chk(
                grpCtx.getMapping(fmt::format("Group {} J/P config", group_)));

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
                        grpCtx.error("Group {} J/P config may not be empty", group_));
        }
    }

    [[nodiscard]]
    GroupConfig build() const {
        std::optional<RPTConfig> rptCfg;
        if (not rp_.isDefault())
            rptCfg = RPTConfig{rp_, set2vec(pruneSGrptList_)};

        auto sptJoins = set2vec(joinSGList_);
        return GroupConfig{group_, std::move(rptCfg), std::move(sptJoins)};
    }

    bool chkSrc(
            yaml::NodeContext& nctx, net::IPv4Address src, JPSourceType jpst) {
        auto ii = sources_.try_emplace(
                src, JPSourceInfo{ .type_ = jpst, .line_ = nctx.line()});

        if (not ii.second) {
            JPSourceInfo eJpsi = ii.first->second;

            errors_.emplace_back(
                    nctx.error(
                            "duplicate {} {}: declared as {} on line {}",
                            src, jpst, eJpsi.type_, eJpsi.type_));
            return false;
        }

        return true;
    }

    net::IPv4Address group_;
    int line_;

    // If this IP address is not default, we have Join(*,G)
    net::IPv4Address rp_;
    std::set<net::IPv4Address> pruneSGrptList_;
    std::set<net::IPv4Address> joinSGList_;

    std::unordered_map<net::IPv4Address, JPSourceInfo> sources_;
};

struct IPv4JPConfigBuilder final: BuilderBase {
    explicit IPv4JPConfigBuilder(std::vector<yaml::ErrorContext>& errors)
    : BuilderBase{errors} {}

    void loadJPConfig(yaml::ValueContext const& jpCfgCtx) {
        auto rJPCfg = chk(jpCfgCtx.getMapping("IPv4 J/P config"));

        if (rJPCfg) {
            size_t sz = rJPCfg->size();

            if (sz > 0) {
                for (auto const& ii: rJPCfg->items()) {
                    auto rGrp = chk(ii.first.getScalar());

                    if (rGrp) {
                        auto rGrpA = chk(grpAddr(
                                rGrp->value()).mapError(
                                        [&rGrp] (auto msg) {
                                            return rGrp->error(msg);
                                        }));

                        if (rGrpA) {
                            net::IPv4Address ga = rGrpA.value();

                            auto bldi = groups_.try_emplace(
                                    ga, errors_, ga, rGrp->line());
                            if (bldi.second) {
                                bldi.first->second.loadGroupConfig(ii.second);
                            } else {
                                auto const& eBld = bldi.first->second;
                                errors_.emplace_back(
                                        rGrp->error(
                                                "duplicate group {}, "
                                                "previously declared on line {}",
                                                ga, eBld.line_));
                            }
                        }
                    }
                }
            } else
                errors_.emplace_back(
                        jpCfgCtx.error("IPv4 J/P config contains no groups"));
        }
    }

    JPConfig build() {
        std::vector<GroupConfig> groups;
        groups.reserve(groups_.size());
        std::transform(groups_.begin(), groups_.end(),
                       std::back_inserter(groups),
                       [] (auto const& ii) {
                            return ii.second.build();
                        });
        return JPConfig{std::move(groups)};
    }

    std::map<net::IPv4Address, IPv4JPGroupConfigBuilder> groups_;
};

} // anon.namespace

auto loadJPConfig(yaml::ValueContext const& jpCfgCtx)
-> Result<JPConfig, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    IPv4JPConfigBuilder jpCfgBuilder{errors};
    jpCfgBuilder.loadJPConfig(jpCfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return jpCfgBuilder.build();
}

} // namespace pimc