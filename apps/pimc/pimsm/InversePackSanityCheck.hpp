#pragma once

#include <vector>
#include <optional>
#include <set>
#include <unordered_map>
#include <algorithm>

#include "pimc/core/Result.hpp"
#include "pimc/formatters/Fmt.hpp"

#include "config/JPConfig.hpp"
#include "Update.hpp"
#include "SanityCheckUtils.hpp"

namespace pimc {


namespace pimsm_detail {

template<IPVersion V>
class InverseGroupEntryConverter final : private GroupBase<V> {
    using IPAddress = typename IP<V>::Address;
    using GroupBase<V>::error;
public:
    explicit InverseGroupEntryConverter(IPAddress group)
            : GroupBase<V>{group} {}

    void add(int updateNo, GroupEntry<V> const &ge) {
        if (not ge.joins().empty())
            error("inverse update #{}: ignoring {} joins", updateNo, ge.joins().size());

        for (auto const &se: ge.prunes()) {
            auto addr = se.addr();
            if (se.wildcard()) {
                if (not se.rpt())
                    error("inverse update #{}: RP {}: rpt bit is not set", updateNo, addr);

                if (rp_) {
                    if (rp_.value() == addr)
                        error("inverse update #{}: RP {}: duplicate insert", updateNo, addr);
                    else
                        error("inverse update #{}: previously set RP {}: "
                              "attempt to set another RP {}",
                              updateNo, rp_.value(), addr);
                } else rp_ = addr;
            } else {
                if (se.rpt())
                    error("inverse update #{}: source {}: rpt bit set", updateNo, addr);

                auto insop = joins_.emplace(addr);
                if (not insop.second)
                    error("inverse update #{}: duplicate pruned source {}", updateNo, addr);
            }
        }
    }

    [[nodiscard]]
    Result<GroupConfig<V>, std::string> convert() {
        if (this->failed())
            return fail(this->msg());

        std::optional<RPT<V>> rpt;
        if (rp_)
            rpt = RPT<V>{rp_.value(), {}};

        std::vector<IPAddress> joins;
        joins.reserve(joins_.size());
        std::ranges::copy(joins_, std::back_inserter(joins));
        return GroupConfig<V>{group_, std::move(rpt), std::move(joins)};
    }

private:
    IPAddress group_;
    std::set<IPAddress> joins_;
    std::optional<IPAddress> rp_;
};

template<IPVersion V>
auto convertInverseUpdatesToJPConfig(std::vector<Update<V>> const &updates)
-> Result<
        std::unordered_map<typename IP<V>::Address, GroupConfig<V>>,
        std::vector<std::string>> {
    using IPAddress = typename IP<V>::Address;
    std::unordered_map<IPAddress, InverseGroupEntryConverter<V>> cvmap;

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
class InverseGroupConfigComparator : private GroupBase<V> {
    using IPAddress = typename IP<V>::Address;
    using GroupBase<V>::error;
public:
    explicit InverseGroupConfigComparator(IPAddress group)
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
            error("the original has RPT with RP {} and ?? pruned sources, "
                  "while the result has no RPT",
                  origRpt->rp());
        } else if (not origRpt and rsltRpt) {
            error("the original has no RPT, "
                  "while the result has RPT with RP {} and ?? pruned sources",
                  rsltRpt->rp());
        } else if (origRpt and rsltRpt) {
            if (origRpt->rp() != rsltRpt->rp())
                error("original RP {} != result RP {}", origRpt->rp(), rsltRpt->rp());
        }

        if (this->failed())
            return fail(this->msg());

        return {};
    }

private:
};


template<IPVersion V>
auto compareInverseJPConfigs(
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
            InverseGroupConfigComparator<V> gcc{ii.first};
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
auto verifyInverseUpdates(
        JPConfig<V> const& jpCfg, std::vector<Update<V>> const& updates)
-> Result<void, std::string> {
    using IPAddress = typename IP<V>::Address;

    auto r = pimsm_detail::convertInverseUpdatesToJPConfig(updates);
    if (not r) {
        fmt::memory_buffer buf;
        for (auto const& msg: r.error())
            buf.append(msg);
        return fail(fmt::to_string(buf));
    }

    std::unordered_map<IPAddress, GroupConfig<V> const*> origCfg;

    for (auto const& ge: jpCfg.groups())
        origCfg.try_emplace(ge.group(), &ge);

    return pimsm_detail::compareInverseJPConfigs(origCfg, r.value());
}

} // namespace pimc

