#pragma once

#include <optional>
#include <vector>

#include "pimc/net/IP.hpp"

namespace pimc {

template <IPVersion V>
class RPT final {
public:
    using IPAddress = typename IP<V>::Address;

    RPT(IPAddress rp, std::vector<IPAddress> prunes)
    : rp_{rp}, prunes_{std::move(prunes)} {}

    /*!
     * \brief Returns the RP address for the group.
     *
     * @return the RP address
     */
    [[nodiscard]]
    IPAddress rp() const { return rp_; }

    /*!
     * \brief Returns a list of RPT-pruned sources.
     *
     * These are the entries Prune(S,G,rpt). The returned list is guaranteed
     * to be sorted in the ascending order and not to have more than the
     * maximum number of entries (which, for IPv4, is 180).
     *
     * @return a list of RPT-pruned sources
     */
    [[nodiscard]]
    std::vector<IPAddress> const& prunes() const {
        return prunes_;
    }

private:
    IPAddress rp_;
    std::vector<IPAddress> prunes_;
};

template <IPVersion V>
class GroupConfig {
public:
    using IPAddress = typename IP<V>::Address;

    GroupConfig(
            IPv4Address group,
            std::optional<RPT<V>> spt,
            std::vector<IPAddress> joins)
            : group_{group}
            , spt_{std::move(spt)}
            , joins_{std::move(joins)} {}

    /*!
     * \brief Returns the multicast group.
     *
     * @return the multicast group
     */
    [[nodiscard]]
    IPAddress group() const { return group_; }

    /*!
     * \brief Returns the RP-tree join/prun configuration for the
     * group
     *
     * @return the RP-tree join/prun configuration for the group
     */
    [[nodiscard]]
    std::optional<RPT<V>> const& rpt() const { return spt_; }

    /*!
     * \brief Returns a list of SPT-joined sources.
     *
     * The sources are guaranteed to be sorted in the ascending order.
     *
     * @return a list of SPT-joined source sources
     */
    [[nodiscard]]
    std::vector<IPAddress> const& spt() const { return joins_; }
private:
    IPAddress group_;
    std::optional<RPT<V>> spt_;
    std::vector<IPAddress> joins_;
};

template <IPVersion V>
class JPConfig final {
public:
    using IPAddress = typename IP<V>::Address;

    explicit JPConfig(std::vector<GroupConfig<V>> groups)
    : groups_{std::move(groups)} {}

    /*!
     * \brief Returns a list multicast group configurations.
     *
     * The list is guaranteed to be sorted by multicast group in the
     * ascending order.
     *
     * @return a list of multicast group configurations
     */
    [[nodiscard]]
    std::vector<GroupConfig<V>> const& groups() const { return groups_; }
private:
    std::vector<GroupConfig<V>> groups_;
};

} // namespace pimc
