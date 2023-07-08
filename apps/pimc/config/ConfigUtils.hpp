#pragma once

#include <fmt/format.h>

#include "pimc/core/Result.hpp"
#include "pimc/net/IPv4Address.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"
#include "pimc/formatters/IPv4Formatters.hpp"

namespace pimc {
enum class UCAddrType: unsigned {
    RP = 0,
    Source = 1,
    Neighbor = 2
};
} // namespace pimc

namespace fmt {

template <>
struct formatter<pimc::UCAddrType>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::UCAddrType const& typ, FormatContext& ctx) {
        switch (typ) {

        case pimc::UCAddrType::RP:
            return fmt::format_to(ctx.out(), "RP");
        case pimc::UCAddrType::Source:
            return fmt::format_to(ctx.out(), "source");
        case pimc::UCAddrType::Neighbor:
            return fmt::format_to(ctx.out(), "neighbor");
        }

        return fmt::format_to(
                ctx.out(), "unicast address type {}", static_cast<unsigned>(typ));
    }
};

} // namespace fmt

namespace pimc {

inline auto ucAddr(
        std::string const& s, UCAddrType typ) -> Result<net::IPv4Address, std::string> {
    auto osa = parseIPv4Address(s);
    if (not osa)
        return fail(fmt::format("invalid {} IPv4 address '{}'", typ, s));

    net::IPv4Address sa = osa.value();
    if (sa.isDefault() or sa.isLocalBroadcast())
        return fail(fmt::format("invalid {} IPv4 address {}", typ, sa));

    if (sa.isLoopback())
        return fail(fmt::format(
                "invalid {} IPv4 address {}: address may not be loopback", typ, sa));

    if (sa.isMcast())
        return fail(fmt::format(
                "invalid {} IPv4 address {}: address may not be multicast", typ, sa));

    return sa;
}

} // namespace pimc