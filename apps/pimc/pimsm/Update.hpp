#pragma once

#include <vector>

#include "pimc/net/IP.hpp"

namespace pimc {

template <IPVersion V>
class Source final {
public:
    using IPAddress = typename IP<V>::Address;

    constexpr Source(IPAddress addr, bool wildcard, bool rpt)
    : addr_{addr}, wildcard_{wildcard}, rpt_{rpt} {}

    [[nodiscard]]
    IPAddress addr() const { return addr_; }

    [[nodiscard]]
    bool wildcard() const { return wildcard_; }

    [[nodiscard]]
    bool rpt() const { return rpt_; }

private:
    IPAddress addr_;
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
    IPAddress group() const { return group_; }

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
