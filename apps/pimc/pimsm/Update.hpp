#pragma once

#include <cstdio>
#include <vector>

#include "pimc/net/IP.hpp"
#include "config/PIMSMParams.hpp"

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
            std::vector<Source<V>> joins,
            std::vector<Source<V>> prunes)
            : group_{group}
            , joins_{std::move(joins)}
            , prunes_{std::move(prunes)} {}

    [[nodiscard]]
    IPAddress group() const { return group_; }

    [[nodiscard]]
    std::vector<Source<V>> const& joins() const { return joins_; }

    [[nodiscard]]
    std::vector<Source<V>> const& prunes() const { return prunes_; }

private:
    IPAddress group_;
    std::vector<Source<V>> joins_;
    std::vector<Source<V>> prunes_;
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

template <IPVersion V>
class GroupSummary final {
public:
    using IPAddress = typename IP<V>::Address;
public:
    explicit GroupSummary(GroupEntry<V> const& ge)
    : group_{ge.group()}
    , joins_{ge.joins().size()}
    , prunes_{ge.prunes().size()}
    , size_{pimsm::params<V>::GrpHdrSize + (joins_ + prunes_) * pimsm::params<V>::SrcASize}
    {}

    [[nodiscard]]
    IPAddress group() const { return group_; }

    [[nodiscard]]
    size_t joins() const { return joins_; }

    [[nodiscard]]
    size_t prunes() const { return prunes_; }

    [[nodiscard]]
    size_t size() const { return size_; }

private:
    IPAddress group_;
    size_t joins_;
    size_t prunes_;
    size_t size_;
};

template <IPVersion V>
class UpdateSummary final {
public:
    UpdateSummary(size_t n, Update<V> const& update): n_{n}, size_{0} {
        groups_.reserve(update.groups().size());
        for (auto const& ge: update.groups())
            groups_.emplace_back(ge);

        for (auto const& gs: groups_)
            size_ += gs.size();

        remaining_ = pimsm::params<V>::capacity - size_;
    }

    [[nodiscard]]
    size_t n() const { return n_; }

    [[nodiscard]]
    std::vector<GroupSummary<V>> groups() const { return groups_; }

    [[nodiscard]]
    size_t size() const { return size_; }

    [[nodiscard]]
    size_t remaining() const { return remaining_; }

private:
    size_t n_;
    std::vector<GroupSummary<V>> groups_;
    size_t size_;
    size_t remaining_;
};

} // namespace pimc
