#include <concepts>
#include <tuple>
#include <vector>
#include <optional>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include "pimc/core/Result.hpp"
#include "pimc/formatters/Fmt.hpp"

#include "config/JPConfig.hpp"
#include "pimsm/Update.hpp"

namespace pimc {

template<std::input_iterator I,
        std::sentinel_for<I> S,
        typename T>
std::string concat(I start, S end, T &&sep) {
    fmt::memory_buffer buf;
    auto bi = std::back_inserter(buf);
    bool separating = false;

    for (I ii{start}; ii != end; ++ii) {
        if (separating)
            fmt::format_to(bi, "{}", std::forward<T>(sep));
        else separating = true;
        fmt::format_to(bi, "{}", *(ii));
    }

    return fmt::to_string(buf);
}

template<std::ranges::input_range R, typename T>
std::string concat(R &&r, T &&sep) {
    return concat(r.begin(), r.end(), std::forward<T>(sep));
}

template<IPVersion V>
auto compareAddrSets(
        std::unordered_set<typename IP<V>::Address> const &a,
        std::unordered_set<typename IP<V>::Address> const &b)
-> std::tuple<std::set<typename IP<V>::Address>, std::set<typename IP<V>::Address>> {
    using IPAddress = typename IP<V>::Address;
    std::set<IPAddress> missing;
    std::set<IPAddress> extraneous;

    for (auto const &addr: a) {
        if (not b.contains(addr)) missing.emplace(addr);
    }

    for (auto const &addr: b) {
        if (not a.contains(addr)) extraneous.emplace(addr);
    }

    return {missing, extraneous};
}

template<IPVersion V, typename T>
auto keySet(std::unordered_map<typename IP<V>::Address, T> const &m)
-> std::unordered_set<typename IP<V>::Address> {
    std::unordered_set<typename IP<V>::Address> keys;
    for (auto const &ii: m) keys.emplace(ii.first);
    return keys;
}

template<IPVersion V>
auto vecSet(std::vector<typename IP<V>::Address> const &v)
-> std::unordered_set<typename IP<V>::Address> {
    std::unordered_set<typename IP<V>::Address> items;
    for (auto const &a: v) items.emplace(a);
    return items;
}

namespace pimsm_detail {

template<IPVersion V>
class GroupBase {
protected:
    using IPAddress = typename IP<V>::Address;

protected:
    explicit GroupBase(IPAddress group) : group_{group}, failed_{false} {}

    template<typename ... Ts>
    void error(fmt::format_string<Ts...> const &fs, Ts &&... args) {
        auto bi = std::back_inserter(buf_);
        if (not failed_) {
            fmt::format_to(bi, "Group {}:\n", group_);
            failed_ = true;
        }

        fmt::format_to(bi, "  ");
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        fmt::format_to(bi, "\n");
    }

    [[nodiscard]]
    bool failed() const { return failed_; }

    [[nodiscard]]
    std::string msg() {
        buf_.push_back('\n');
        return fmt::to_string(buf_);
    }

protected:
    IPAddress group_;

private:
    bool failed_;
    fmt::memory_buffer buf_;
};

template<IPVersion V>
class GroupEntryConverter final : private GroupBase<V> {
    using IPAddress = typename IP<V>::Address;
    using GroupBase<V>::error;
public:
    explicit GroupEntryConverter(IPAddress group)
            : GroupBase<V>{group} {}

    void add(int updateNo, GroupEntry<V> const &ge) {
        for (auto const &se: ge.joins()) {
            auto addr = se.addr();
            if (se.wildcard()) {
                if (not se.rpt())
                    error("update #{}: RP {}: rpt bit is not set", updateNo, addr);

                if (rp_) {
                    if (rp_.value() == addr)
                        error("update #{}: RP {}: duplicate insert", updateNo, addr);
                    else
                        error("update #{}: previously set RP {}: "
                              "attempt to set another RP {}",
                              updateNo, rp_.value(), addr);
                } else rp_ = addr;
            } else {
                if (se.rpt())
                    error("update #{}: source {}: rpt bit set", updateNo, addr);

                auto insop = joins_.emplace(addr);
                if (not insop.second)
                    error("update #{}: duplicate joined source {}", updateNo, addr);

                if (prunes_.contains(addr))
                    error("update #{}: joined source {} "
                          "also appears in the RPT pruned sources",
                          updateNo, addr);
            }
        }

        if (not rp_ and not ge.prunes().empty())
            error("update #{}: no RP is defined, "
                  "ignoring {} RPT pruned sources",
                  updateNo, ge.prunes().size());
        else {
            for (auto const &se: ge.prunes()) {
                auto addr = se.addr();

                if (se.wildcard())
                    error("update #{}: pruned source {}: wildcard bit set",
                          updateNo, addr);

                if (not se.rpt())
                    error("update #{}: pruned source {}: rpt bit not set",
                          updateNo, addr);

                auto insop = prunes_.emplace(addr);
                if (not insop.second)
                    error("update #{}: duplicate pruned source {}", updateNo, addr);

                if (joins_.contains(addr))
                    error("update #{}: pruned source {} "
                          "also appears in the SPT joined sources",
                          updateNo, addr);
            }
        }
    }

    [[nodiscard]]
    Result<GroupConfig<V>, std::string> convert() {
        if (this->failed())
            return fail(this->msg());

        std::optional<RPT<V>> rpt;
        if (rp_) {
            std::vector<IPAddress> prunes;
            prunes.reserve(prunes_.size());
            std::ranges::copy(prunes_, std::back_inserter(prunes));
            rpt = RPT<V>{rp_.value(), std::move(prunes)};
        }

        std::vector<IPAddress> joins;
        joins.reserve(joins_.size());
        std::ranges::copy(joins_, std::back_inserter(joins));
        return GroupConfig<V>{group_, std::move(rpt), std::move(joins)};
    }

private:
    IPAddress group_;
    std::set<IPAddress> joins_;
    std::optional<IPAddress> rp_;
    std::set<IPAddress> prunes_;

};

template<IPVersion V>
auto convertUpdatesToJPConfig(std::vector<Update<V>> const &updates)
-> Result<
        std::unordered_map<typename IP<V>::Address, GroupConfig<V>>,
        std::vector<std::string>> {
    using IPAddress = typename IP<V>::Address;
    std::unordered_map<IPAddress, GroupEntryConverter<V>> cvmap;

    int i{1};
    for (auto const &update: updates) {
        for (auto const &ge: update.groups()) {
            auto maddr = ge.group();
            auto ii = cvmap.try_emplace(maddr, maddr);
            ii.first->second.add(i++, ge);
        }
    }

    std::vector<std::string> errors;
    std::unordered_map<IPAddress, GroupConfig<V>> jpCfg;
    for (auto &ii: cvmap) {
        auto r = ii.second.convert();

        if (r)
            jpCfg.try_emplace(ii.first, std::move(r).value());
        else
            errors.emplace_back(std::move(r).error());
    }

    if (not errors.empty())
        return fail(std::move(errors));

    return std::move(jpCfg);
}

template<IPVersion V>
class GroupConfigComparator : private GroupBase<V> {
    using IPAddress = typename IP<V>::Address;
    using GroupBase<V>::error;
public:
    explicit GroupConfigComparator(IPAddress group)
            : GroupBase<V>{group} {}

    auto compare(GroupConfig<V> const &orig, GroupConfig<V> rslt)
    -> Result<void, std::string> {
        std::set<IPAddress> mjs, ejs;
        std::tie(mjs, ejs) =
                compareAddrSets<V>(vecSet<V>(orig.spt()), vecSet<V>(rslt.spt()));

        if (not mjs.empty())
            error("missing SPT joined sources:\n"
                  "    {}",
                  concat(mjs, "\n    "));

        if (not ejs.empty())
            error("extraneous SPT joined sources:\n"
                  "    {}",
                  concat(ejs, "\n    "));

        auto const &origRpt = orig.rpt();
        auto const &rsltRpt = rslt.rpt();

        if (origRpt and not rsltRpt) {
            error("the original has RPT with RP {} and {} pruned sources, "
                  "while the result has no RPT",
                  origRpt->rp(), origRpt->prunes().size());
        } else if (not origRpt and rsltRpt) {
            error("the original has no RPT, "
                  "while the result has RPT with RP {} and {} pruned sources",
                  rsltRpt->rp(), rsltRpt->prunes().size());
        } else if (origRpt and rsltRpt) {
            if (origRpt->rp() != rsltRpt->rp())
                error("original RP {} != result RP {}", origRpt->rp(), rsltRpt->rp());

            std::set<IPAddress> mps, eps;
            std::tie(mps, eps) =
                    compareAddrSets<V>(
                            vecSet<V>(origRpt->prunes()), vecSet<V>(rsltRpt->prunes()));

            if (not mps.empty())
                error("missing RPT pruned sources:\n"
                      "    {}",
                      concat(mps, "\n    "));

            if (not eps.empty())
                error("extraneous RPT pruned sources:\n"
                      "    {}",
                      concat(eps, "\n    "));
        }

        if (this->failed())
            return fail(this->msg());

        return {};
    }

private:
};

class ErrorTracker {
public:
    ErrorTracker() : failed_{false} {}

    template<typename ... Ts>
    void error(fmt::format_string<Ts...> const &fs, Ts &&...args) {
        auto bi = std::back_inserter(buf_);
        failed_ = true;
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        fmt::format_to(bi, "\n\n");
    }

    void append(std::string const &msg) {
        failed_ = true;
        buf_.append(msg);
    }

    [[nodiscard]]
    bool failed() const { return failed_; }

    [[nodiscard]]
    std::string msg() {
        return fmt::to_string(buf_);
    }

private:
    bool failed_;
    fmt::memory_buffer buf_;
};


template<IPVersion V>
auto compareJPConfigs(
        std::unordered_map<typename IP<V>::Address, GroupConfig<V> const *> const &orig,
        std::unordered_map<typename IP<V>::Address, GroupConfig<V>> const &rslt)
-> Result<void, std::string> {
    using IPAddress = typename IP<V>::Address;

    ErrorTracker et;

    std::set<IPAddress> mgs, egs;
    std::tie(mgs, egs) = compareAddrSets<V>(keySet<V>(orig), keySet<V>(rslt));

    if (not mgs.empty()) {
        et.error(
                "missing groups:\n"
                "  {}",
                concat(mgs, "\n  "));
    }

    if (not egs.empty())
        et.error(
                "extraneous groups:\n"
                "  {}",
                concat(egs, "\n  "));

    for (auto ii: orig) {
        auto ri = rslt.find(ii.first);

        if (ri != rslt.end()) {
            GroupConfigComparator<V> gcc{ii.first};
            auto r = gcc.compare(*ii.second, ri->second);

            if (not r)
                et.append(r.error());
        }
    }

    if (et.failed())
        return fail(et.msg());

    return {};
}

} // namespace pimsm_detail

template <IPVersion V>
auto verifyUpdates(
        JPConfig<V> const& jpCfg, std::vector<Update<V>> const& updates)
-> Result<void, std::string> {
    using IPAddress = typename IP<V>::Address;

    auto r = pimsm_detail::convertUpdatesToJPConfig(updates);
    if (not r) {
        fmt::memory_buffer buf;
        for (auto const& msg: r.error())
            buf.append(msg);
        return fail(fmt::to_string(buf));
    }

    std::unordered_map<IPAddress, GroupConfig<V> const*> origCfg;

    for (auto const& ge: jpCfg.groups())
        origCfg.try_emplace(ge.group(), &ge);

    return pimsm_detail::compareJPConfigs(origCfg, r.value());
}

} // namespace pimc

