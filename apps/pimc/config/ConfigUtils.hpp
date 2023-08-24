#pragma once

#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Result.hpp"
#include "pimc/net/IP.hpp"
#include "pimc/parsers/IPParsers.hpp"
#include "pimc/formatters/IPFormatters.hpp"
#include "pimc/yaml/Structured.hpp"
#include "pimc/yaml/BuilderBase.hpp"

namespace pimc {
enum class JPSourceType: unsigned {
    RP = 0,
    RptPruned = 1,
    SptJoined = 2
};
} // namespace pimc

namespace fmt {

template <>
struct formatter<pimc::JPSourceType>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::JPSourceType const& jpst, FormatContext& ctx) {
        switch (jpst) {

        case pimc::JPSourceType::RP:
            return fmt::format_to(ctx.out(), "RP");
        case pimc::JPSourceType::RptPruned:
            return fmt::format_to(ctx.out(), "RPT-pruned source");
        case pimc::JPSourceType::SptJoined:
            return fmt::format_to(ctx.out(), "SPT-joined source");
        }

        return fmt::format_to(
                ctx.out(), "unknown source type {}", static_cast<unsigned>(jpst));
    }
};

} // namespace fmt

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


struct JPSourceInfo {
    JPSourceType type_;
    int line_;
};


template <IPVersion V>
inline auto ucAddr(std::string const& s, UCAddrType typ)
-> Result<typename IP<V>::Address, std::string> {
    auto osa = parse<V>::address(s);
    if (not osa)
        return fail(fmt::format("invalid {} {} address '{}'", V{}, typ, s));

    IPv4Address sa = osa.value();
    if (sa.isDefault() or sa.isLocalBroadcast())
        return fail(fmt::format("invalid {} {} address {}", V{}, typ, sa));

    if (sa.isLoopback())
        return fail(fmt::format(
                "invalid {} {} address {}: address may not be loopback", V{}, typ, sa));

    if (sa.isMcast())
        return fail(fmt::format(
                "invalid {} {} address {}: address may not be multicast", V{}, typ, sa));

    return sa;
}

struct BuilderBase: yaml::BuilderBase<BuilderBase> {
    constexpr explicit BuilderBase(std::vector<yaml::ErrorContext>& errors)
    : errors_{errors} {}

    template <typename T>
    auto chkErrors(Result<T, std::vector<yaml::ErrorContext>> r)
    -> Result<T, std::vector<yaml::ErrorContext>> {
        if (not r) {
            errors_.reserve(errors_.size() + r.error().size());
            for (auto& e: r.error())
                errors_.emplace_back(std::move(e));
        }

        return r;
    }

    void chkExtraneous(yaml::MappingContext const& mCtx) {
        auto extraneous = mCtx.extraneous();
        if (not extraneous.empty()) {
            errors_.reserve(errors_.size() + extraneous.size());
            for (auto& e: extraneous)
                errors_.emplace_back(std::move(e));
        }
    }

    void consume(yaml::ErrorContext ectx) {
        errors_.emplace_back(std::move(ectx));
    }

    std::vector<yaml::ErrorContext>& errors_;
};

} // namespace pimc