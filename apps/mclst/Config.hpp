#pragma once

#include <cstdint>
#include <string>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4IntfTable.hpp"

namespace pimc {

class Config final {
public:
    static Config fromArgs(int argc, char** argv);

    Config(Config const&) = delete;
    Config(Config&&) noexcept = default;
    Config& operator= (Config const&) = delete;
    Config& operator= (Config&&) noexcept = default;

    [[nodiscard]]
    net::IPv4Address group() const { return group_; }

    [[nodiscard]]
    uint16_t dport() const { return dport_; }

    [[nodiscard]]
    bool wildcard() const { return wildcard_; }

    [[nodiscard]]
    std::string const& intf() const { return intf_; }

    [[nodiscard]]
    net::IPv4Address intfAddr() const { return intfAddr_; }

    /**
     * If the returned address is default, the subscription is (*,G),
     * otherwise it's (S,G) where S is the value returned by this
     * function
     *
     * @return the multicast source for the source specific subscriptions,
     * or the default address for the any-source subscriptions.
     */
    [[nodiscard]]
    net::IPv4Address source() const { return source_; }

    [[nodiscard]]
    unsigned timeoutSec() const { return timeoutSec_; }

    [[nodiscard]]
    bool sender() const { return sender_; }

    [[nodiscard]]
    unsigned ttl() const { return ttl_; }

    [[nodiscard]]
    uint64_t count() const { return count_; }

    [[nodiscard]]
    bool showPayload() const { return showPayload_; }

    [[nodiscard]]
    bool colors() const { return colors_; }

    [[nodiscard]]
    IPv4IntfTable const& intfTable() const { return intfTable_; };

    [[nodiscard]]
    bool showConfig() const { return showConfig_; }

    void show() const;

private:
    Config(
        net::IPv4Address group,
        uint16_t dport,
        bool wildcard,
        std::string intf,
        net::IPv4Address intfAddr,
        net::IPv4Address source,
        unsigned timeoutSec,
        bool sender,
        unsigned ttl,
        uint64_t count,
        bool showPayload,
        bool colors,
        IPv4IntfTable intfTable,
        bool showConfig)
        : group_{group}
        , dport_{dport}
        , wildcard_{wildcard}
        , intf_{std::move(intf)}
        , intfAddr_{intfAddr}
        , source_{source}
        , timeoutSec_{timeoutSec}
        , sender_{sender}
        , ttl_{ttl}
        , count_{count}
        , showPayload_{showPayload}
        , colors_{colors}
        , intfTable_{std::move(intfTable)}
        , showConfig_{showConfig} {}

private:
    net::IPv4Address group_;
    uint16_t dport_;
    bool wildcard_;
    std::string intf_;
    net::IPv4Address intfAddr_;
    // If this is 0.0.0.0, we're subscribing to (*,G)
    // otherwise to (S,G) where S is the source_
    net::IPv4Address source_;
    unsigned timeoutSec_;
    bool sender_;
    unsigned ttl_;
    uint64_t count_;
    bool showPayload_;
    bool colors_;
    IPv4IntfTable intfTable_;
    bool showConfig_;
};

} // namespace pimc