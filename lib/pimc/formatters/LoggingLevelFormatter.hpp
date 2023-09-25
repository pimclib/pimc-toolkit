#pragma once

#include "pimc/logging/LoggingLevel.hpp"
#include "Fmt.hpp"

namespace fmt {
template<>
struct formatter<pimc::Level> : formatter<string_view> {
    template<typename FormatContext>
    auto format(pimc::Level const &level, FormatContext &ctx) {
        switch (level) {

        case pimc::Level::None:
            return fmt::format_to(ctx.out(), "NONE");
        case pimc::Level::Critical:
            return fmt::format_to(ctx.out(), "CRITICAL");
        case pimc::Level::Error:
            return fmt::format_to(ctx.out(), "ERROR");
        case pimc::Level::Warning:
            return fmt::format_to(ctx.out(), "WARNING");
        case pimc::Level::Info:
            return fmt::format_to(ctx.out(), "INFO");
        case pimc::Level::Debug:
            return fmt::format_to(ctx.out(), "DEBUG");
        }

        return fmt::format_to(ctx.out(), "Level {}", static_cast<int>(level));
    }
};

} // namespace fmt
