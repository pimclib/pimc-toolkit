#pragma once

#include <algorithm>

#include "pimc/text/MemoryBuffer.hpp"

#include "pimsm/Update.hpp"

namespace fmt {

template <pimc::IPVersion V>
struct formatter<pimc::Source<V>>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::Source<V> const& src, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "{}", src.addr());
        if (src.wildcard())
            out = fmt::format_to(out, ", WC");
        if (src.rpt())
            out = fmt::format_to(out, ", rpt");
        return out;
    }
};

template <pimc::IPVersion V>
struct formatter<pimc::GroupEntry<V>>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::GroupEntry<V> const& ge, FormatContext& ctx) {
        auto out = fmt::format_to(ctx.out(), "Group {}\n", ge.group());
        auto const& joins = ge.joins();
        auto const& prunes = ge.prunes();
        out = fmt::format_to(out, " {} joins, {} prunes\n", joins.size(), prunes.size());
        out = fmt::format_to(out, " Joins:\n");
        for (auto const& join: joins)
            out = fmt::format_to(out, "   {}\n", join);
        out = fmt::format_to(out, " Prunes:\n");
        for (auto const& prune: prunes)
            out = fmt::format_to(out, "   {}\n", prune);
        return out;
    }
};

template <pimc::IPVersion V>
struct formatter<pimc::Update<V>>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::Update<V> const& update, FormatContext& ctx) {
        auto const& groups = update.groups();
        auto out = fmt::format_to(ctx.out(), "Update with {} groups:\n", groups.size());
        for (auto const& ge: groups)
            out = fmt::format_to(out, ge);

        return out;
    }
};

} // namespace fmt
