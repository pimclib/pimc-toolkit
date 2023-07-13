#pragma once

#include <vector>

#include "pimc/core/TypeUtils.hpp"
#include "pimc/net/IPAddress.hpp"

namespace pimc {

template <net::IPAddress A>
struct Source final {
    constexpr Source(A saddr, bool wildcard, bool rpt)
    : saddr_{saddr}, wildcard_{wildcard}, rpt_{rpt} {}

    A saddr_;
    bool wildcard_;
    bool rpt_;
};

template <net::IPAddress A>
class GroupEntry final {
public:
    GroupEntry(A group, std::vector<Source<A>> joins, std::vector<Source<A>> prunes)
    : group_{group}, joins_{std::move(joins)}, prunes_{std::move(prunes)} {}

    [[nodiscard]]
    std::vector<Source<A>> const& joins() const { return joins_; }

    [[nodiscard]]
    std::vector<Source<A>> const& prunes() const { return prunes_; }

private:
    A group_;
    std::vector<Source<A>> joins_;
    std::vector<Source<A>> prunes_;
};

template <net::IPAddress A>
class Update final {
public:
    explicit Update(std::vector<GroupEntry<A>> groups)
    : groups_{std::move(groups)} {}

    [[nodiscard]]
    std::vector<GroupEntry<A>> const& groups() const { return groups_; }
private:
    std::vector<GroupEntry<A>> groups_;
};

} // namespace pimc
