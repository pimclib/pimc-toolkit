#pragma once

#include "pimc/formatters/Fmt.hpp"

#include "pimc/formatters/IPv4Formatters.hpp"

#include "JPConfig.hpp"
#include "PIMSMConfig.hpp"

namespace fmt {

template <>
struct formatter<pimc::RPTConfig>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::RPTConfig const& rptConfig, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "    Join(*,G): RP {}\n", rptConfig.rp());
        auto const& rptPrunes = rptConfig.prunes();
        if (not rptPrunes.empty()) {
            out = fmt::format_to(out, "    Prune(S,G,rpt):\n");
            for (auto const& src: rptPrunes)
                out = fmt::format_to(out, "      {}\n", src);
        }
        return out;
    }
};

template <>
struct formatter<pimc::GroupConfig>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::GroupConfig const& gCfg, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "  {}\n", gCfg.group());
        auto const& rpt = gCfg.rpt();
        if (rpt)
            out = fmt::format_to(out, "{}", rpt.value());
        auto const& spt = gCfg.spt();
        if (not spt.empty()) {
            out = fmt::format_to(out, "    Join(S,G):\n");
            for (auto const& src: spt)
                out = fmt::format_to(out, "      {}\n", src);
        }
        return out;
    }
};

template <>
struct formatter<pimc::JPConfig>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::JPConfig const& jpConfig, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "Join/Prune config:\n");
        for (auto const& gCfg: jpConfig.groups())
            out = fmt::format_to(out, "{}", gCfg);

        return out;
    }
};

template <>
struct formatter<pimc::PIMSMConfig>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::PIMSMConfig const& pimsmConfig, FormatContext& ctx) {
        return fmt::format_to(
                ctx.out(),
                "PIM sparse-mode:\n  neighbor: {}\n",
                pimsmConfig.neighbor());
    }
};

} // namespace fmt
