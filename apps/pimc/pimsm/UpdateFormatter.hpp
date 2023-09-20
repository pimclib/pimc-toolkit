#pragma once

#include <tuple>
#include <algorithm>

#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/MemoryBuffer.hpp"
#include "pimc/text/NumberLengths.hpp"
#include "pimc/text/SCLine.hpp"
#include "pimc/text/Plural.hpp"

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
        auto out = fmt::format_to(
                ctx.out(),
                "Update with {} group{}:\n",
                groups.size(), pimc::plural(groups));
        for (auto const& ge: groups)
            out = fmt::format_to(out, "{}", ge);

        return out;
    }
};

template <pimc::IPVersion V>
struct formatter<std::tuple<unsigned, pimc::Update<V> const&>>: formatter<string_view> {

    template <typename FormatContext>
    auto format(
            std::tuple<unsigned, pimc::Update<V> const&> const& eu, FormatContext& ctx) {
        unsigned n = std::get<unsigned>(eu);
        auto const& update = std::get<pimc::Update<V> const&>(eu);
        auto const& groups = update.groups();
        auto out = fmt::format_to(
                ctx.out(),
                "Update #{} with {} group{}:\n",
                n, groups.size(), pimc::plural(groups));
        for (auto const& ge: groups)
            out = fmt::format_to(out, "{}", ge);

        return out;
    }
};


template <pimc::IPVersion V>
struct formatter<pimc::UpdateSummary<V>>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::UpdateSummary<V> const& us, FormatContext& ctx) {
        char gbuf[32];
        size_t groupColW{5ul};
        size_t joinsColW{5ul};
        size_t prunesColW{6ul};
        size_t sizeColW{4ul};

        for (auto const& gs: us.groups()) {
            groupColW = std::max(groupColW, gs.group().charlen());
            joinsColW = std::max(joinsColW, pimc::decimalUIntLen(gs.joins()));
            prunesColW = std::max(prunesColW, pimc::decimalUIntLen(gs.prunes()));
            sizeColW = std::max(sizeColW, pimc::decimalUIntLen(gs.size()));
        }

        pimc::SCLine<'='> sep{std::max({groupColW, joinsColW, prunesColW, sizeColW})};
        pimc::SCLine<' '> ind{2};

        auto fmts = fmt::format(
                "{}{{:<{}}} {{:>{}}} {{:>{}}} {{:>{}}}\n",
                ind(), groupColW, joinsColW, prunesColW, sizeColW);

        auto out = fmt::format_to(
                ctx.out(),
                "Update #{}, size {}, remaining size {}\n",
                us.n(), us.size(), us.remaining());

        out = fmt::format_to(
                out, fmt::runtime(fmts),
                "group", "joins", "prunes", "size");
        out = fmt::format_to(
                out, fmt::runtime(fmts),
                sep(groupColW), sep(joinsColW), sep(prunesColW), sep(sizeColW));

        for (auto const& gs: us.groups()) {
            auto* p = fmt::format_to(gbuf, "{}", gs.group());
            size_t blanks = groupColW - static_cast<size_t>(p - gbuf);
            for (size_t i = 0; i < blanks; ++i) {
                *p++ = ' ';
            }
            *p = static_cast<char>(0);
            out = fmt::format_to(
                    out, fmt::runtime(fmts),
                    gbuf, gs.joins(), gs.prunes(), gs.size());
        }

        return out;
    }
};


} // namespace fmt
