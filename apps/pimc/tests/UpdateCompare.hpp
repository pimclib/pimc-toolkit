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

namespace pimc {

template <IPVersion V>
class UpdateCompare final {
    using IPAddress = typename IP<V>::Address;
public:
    class CompareResult final {
        template <IPVersion>
        friend class UpdateCompare;
    public:
        [[nodiscard]]
        constexpr explicit operator bool() const { return msg_.empty(); }

        [[nodiscard]]
        std::string const& msg() const { return msg_; }
    private:
        explicit CompareResult(std::string msg): msg_{std::move(msg)} {}

    private:
        std::string msg_;
    };
public:
    UpdateCompare(): buf_{getMemoryBuffer()} {}

    CompareResult operator() (Update<V> const& exp, Update<V> const& eff) {
        buf_.clear();
        auto bi = std::back_inserter(buf_);

        auto expSz = exp.groups().size();
        auto effSz = eff.groups().size();
        if (expSz != effSz)
            fmt::format_to("exp size {} != eff size {}\n", expSz, effSz);

        auto const& expGroups = exp.groups();
        auto const& effGroups = exp.groups();
        auto sz = std::min(expSz, effSz);
        for (size_t i = 0; i < sz; ++i)
            cmpge(expGroups[i], effGroups[i]);

        if (expSz > sz) {
            fmt::format_to(bi, "expecting the following group entries:\n");
            for (size_t i = sz; i < expSz; ++i)
                fmt::format_to(bi, "{}", expGroups[i]);
        } else if (effSz > sz) {
            fmt::format_to(bi, "not expecting the following group entries:\n");
            for (size_t i = sz; i < effSz; ++i)
                fmt::format_to(bi, "{}", effGroups[i]);
        }
    }

private:

    struct GroupCompareWriter final {
        GroupCompareWriter(IPAddress group, fmt::memory_buffer& buf)
        : bi_{std::back_inserter(buf)}, group_{group}, groupWritten_{false} {}

        template <typename ... Ts>
        void error(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
            if (not groupWritten_) {
                fmt::format_to(bi_, "Group {}\n", group_);
                groupWritten_ = true;
            }

            fmt::format_to(bi_, fs, std::forward<Ts>(args)...);
        }

        std::back_insert_iterator<fmt::memory_buffer> bi_;
        IPAddress group_;
        bool groupWritten_;
    };

    void cmpge(GroupEntry<V> expGe, GroupEntry<V> effGe) {
        auto expg = expGe.group();
        auto effg = effGe.group();
        if (expg != effg) {
            auto bi = std::back_inserter(buf_);
            fmt::format_to("Expecting group {}, received group {}\n", expg, effg);
            return;
        }

        GroupCompareWriter gcw{effg, buf_};

    }
private:
    fmt::memory_buffer& buf_;
};

} // namespace pimc
