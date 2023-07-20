#pragma once

#include <vector>

#include "pimc/net/IP.hpp"

namespace pimc {

template <IPVersion V>
struct Source final {
    using IPAddress = typename IP<V>::Address;

    constexpr Source(IPAddress saddr, bool wildcard, bool rpt)
    : saddr_{saddr}, wildcard_{wildcard}, rpt_{rpt} {}

    IPAddress saddr_;
    bool wildcard_;
    bool rpt_;
};

template <IPVersion V>
class GroupEntry final {
public:
    using IPAddress = typename IP<V>::Address;

    GroupEntry(
            IPAddress group,
            std::vector<Source<IPAddress>> joins,
            std::vector<Source<IPAddress>> prunes)
            : group_{group}
            , joins_{std::move(joins)}
            , prunes_{std::move(prunes)} {}

    [[nodiscard]]
    std::vector<Source<IPAddress>> const& joins() const { return joins_; }

    [[nodiscard]]
    std::vector<Source<IPAddress>> const& prunes() const { return prunes_; }

private:
    IPAddress group_;
    std::vector<Source<IPAddress>> joins_;
    std::vector<Source<IPAddress>> prunes_;
};

template <IPVersion V>
class Update final {
public:
    using IPAddress = typename IP<V>::Address;

    explicit Update(std::vector<GroupEntry<V>> groups)
    : groups_{std::move(groups)} {}

    [[nodiscard]]
    std::vector<GroupEntry<V>> const& groups() const { return groups_; }
private:
    std::vector<GroupEntry<V>> groups_;
};

} // namespace pimc
