#pragma once

#include <optional>
#include <vector>

#include "pimc/net/IPv4Address.hpp"

namespace pimc {

class RPTConfig final {
public:
    RPTConfig(net::IPv4Address rp, std::vector<net::IPv4Address> prunes)
    : rp_{rp}, prunes_{std::move(prunes)} {}

    [[nodiscard]]
    net::IPv4Address rp() const { return rp_; }

    [[nodiscard]]
    std::vector<net::IPv4Address> const& prunes() const {
        return prunes_;
    }

private:
    net::IPv4Address rp_;
    std::vector<net::IPv4Address> prunes_;
};

class GroupConfig {
public:
    GroupConfig(
            net::IPv4Address group,
            std::optional<RPTConfig> spt,
            std::vector<net::IPv4Address> joins)
            : group_{group}
            , spt_{std::move(spt)}
            , joins_{std::move(joins)} {}

    [[nodiscard]]
    net::IPv4Address group() const { return group_; }

    [[nodiscard]]
    std::optional<RPTConfig> const& rpt() const { return spt_; }

    [[nodiscard]]
    std::vector<net::IPv4Address> const& spt() const { return joins_; }
private:
    net::IPv4Address group_;
    std::optional<RPTConfig> spt_;
    std::vector<net::IPv4Address> joins_;
};

class JPConfig final {
public:
    explicit JPConfig(std::vector<GroupConfig> groups)
    : groups_{std::move(groups)} {}

    [[nodiscard]]
    std::vector<GroupConfig> const& groups() const { return groups_; }
private:
    std::vector<GroupConfig> groups_;
};

} // namespace pimc
