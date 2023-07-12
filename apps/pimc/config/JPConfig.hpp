#pragma once

#include <optional>
#include <vector>

#include "pimc/net/IPAddress.hpp"

namespace pimc {

template <net::IPAddress A>
class RPT final {
public:
    RPT(A rp, std::vector<A> prunes)
    : rp_{rp}, prunes_{std::move(prunes)} {}

    /*!
     * \brief Returns the RP address for the group.
     *
     * @return the RP address
     */
    [[nodiscard]]
    A rp() const { return rp_; }

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
    std::vector<A> const& prunes() const {
        return prunes_;
    }

private:
    A rp_;
    std::vector<A> prunes_;
};

template <net::IPAddress A>
class GroupConfig {
public:
    GroupConfig(
            net::IPv4Address group,
            std::optional<RPT<A>> spt,
            std::vector<A> joins)
            : group_{group}
            , spt_{std::move(spt)}
            , joins_{std::move(joins)} {}

    /*!
     * \brief Returns the multicast group.
     *
     * @return the multicast group
     */
    [[nodiscard]]
    A group() const { return group_; }

    /*!
     * \brief Returns the RP-tree join/prun configuration for the
     * group
     *
     * @return the RP-tree join/prun configuration for the group
     */
    [[nodiscard]]
    std::optional<RPT<A>> const& rpt() const { return spt_; }

    /*!
     * \brief Returns a list of SPT-joined sources.
     *
     * The sources are guaranteed to be sorted in the ascending order.
     *
     * @return a list of SPT-joined source sources
     */
    [[nodiscard]]
    std::vector<A> const& spt() const { return joins_; }
private:
    A group_;
    std::optional<RPT<A>> spt_;
    std::vector<A> joins_;
};

template <net::IPAddress A>
class JPConfig final {
public:
    explicit JPConfig(std::vector<GroupConfig<A>> groups)
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
    std::vector<GroupConfig<A>> const& groups() const { return groups_; }
private:
    std::vector<GroupConfig<A>> groups_;
};

} // namespace pimc
