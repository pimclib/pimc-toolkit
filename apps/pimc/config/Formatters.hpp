#pragma once

#include "pimc/formatters/Fmt.hpp"

#include "pimc/formatters/IPv4Formatters.hpp"

#include "JPConfig.hpp"
#include "PIMSMConfig.hpp"

namespace fmt {

template <pimc::IPVersion V>
struct formatter<pimc::RPT<V>>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::RPT<V> const& rptConfig, FormatContext& ctx) {
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

template <pimc::IPVersion V>
struct formatter<pimc::GroupConfig<V>>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::GroupConfig<V> const& gCfg, FormatContext& ctx) {
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

template <pimc::IPVersion V>
struct formatter<pimc::JPConfig<V>>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::JPConfig<V> const& jpConfig, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "Join/Prune config:\n");
        for (auto const& gCfg: jpConfig.groups())
            out = fmt::format_to(out, "{}", gCfg);

        return out;
    }
};

template <pimc::IPVersion V>
struct formatter<pimc::PIMSMConfig<V>>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::PIMSMConfig<V> const& pimsmConfig, FormatContext& ctx) {
        return fmt::format_to(
                ctx.out(),
                "PIM sparse-mode:\n"
                "  neighbor: {}\n"
                "  interface: {}, #{}, addr {}\n"
                "  hello period: {}s\n"
                "  hello hold time: {}s\n"
                "  join/prune period: {}s\n"
                "  join/prune hold time: {}s\n"
                "  generation ID: {:08x}\n",
                pimsmConfig.neighbor(),
                pimsmConfig.intfName(),
                pimsmConfig.intfIndex(),
                pimsmConfig.intfAddr(),
                pimsmConfig.helloPeriod(),
                pimsmConfig.helloHoldtime(),
                pimsmConfig.jpPeriod(),
                pimsmConfig.jpHoldtime(),
                pimsmConfig.generationId());
    }
};

} // namespace fmt
