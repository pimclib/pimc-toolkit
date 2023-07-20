#pragma once

#include "pimc/net/IP.hpp"
#include "Fmt.hpp"
#include "IPv4Formatters.hpp"

namespace fmt {

template <>
struct formatter<pimc::IPv4>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::IPv4 const&, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "IPv4");
    }
};

template <>
struct formatter<pimc::IPv6>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::IPv6 const&, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "IPv6");
    }
};

} // namespace fmt

