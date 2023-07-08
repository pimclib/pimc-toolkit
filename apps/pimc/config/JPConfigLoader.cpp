#include <set>
#include <map>
#include <unordered_map>

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

Result<net::IPv4Address, std::string> srcAddr(std::string const& s) {
    auto osa = parseIPv4Address(s);
    if (not osa)
        return fail(fmt::format("invalid source IPv4 address '{}'", s));

    net::IPv4Address sa = osa.value();
    if (sa.isDefault() or sa.isLocalBroadcast())
        return fail(fmt::format("invalid source IPv4 address {}", sa));

    if (sa.isLoopback())
        return fail(fmt::format(
                "invalid source IPv4 address {}: address may not be loopback", sa));

    if (sa.isMcast())
        return fail(fmt::format(
                "invalid source IPv4 address {}: address may not be multicast", sa));

    return sa;
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

struct IPv4JPGroupConfigBuilder final: yaml::BuilderBase<IPv4JPGroupConfigBuilder> {
    IPv4JPGroupConfigBuilder(
            net::IPv4Address group, std::vector<yaml::ErrorContext>& errors)
    : group_{group}, errors_{errors} {}

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
                            "duplicate {} {}: declared as {} in line {}",
                            src, jpst, eJpsi.type_, eJpsi.type_));
            return false;
        }

        return true;
    }

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

    net::IPv4Address group_;
    // If this IP address is not default, we have Join(*,G)
    net::IPv4Address rp_;
    std::set<net::IPv4Address> pruneSGrptList_;
    std::set<net::IPv4Address> joinSGList_;

    std::unordered_map<net::IPv4Address, JPSourceInfo> sources_;
    std::vector<yaml::ErrorContext>& errors_;
};

struct JPConfigBuilder final: yaml::BuilderBase<JPConfigBuilder> {

    void loadJPConfig(yaml::ValueContext const& jpCfgCtx) {

    }

    void consume(yaml::ErrorContext ectx) {
        errors_.emplace_back(std::move(ectx));
    }

    std::unordered_map<net::IPv4Address, int> groups_;
    std::vector<yaml::ErrorContext> errors_;
};

} // anon.namespace


auto loadJPConfig(yaml::ValueContext const& jpCfgCtx)
-> Result<JPConfig, std::vector<yaml::ErrorContext>> {
    JPConfigBuilder jpCfgB{};

}

} // namespace pimc