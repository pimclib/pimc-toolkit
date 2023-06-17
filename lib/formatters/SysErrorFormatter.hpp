#pragma once

#include <cstring>
#include <type_traits>

#include <fmt/format.h>

#include "pimc/core/TypeUtils.hpp"
#include "pimc/system/SysError.hpp"

#include "detect_strerror_r.hpp"

namespace fmt {

template <>
struct formatter<pimc::SysError>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::SysError const& se, FormatContext& ctx) {
        char buf[1024];
        auto p = pimc::detail::invoke_strerror_r<sizeof(buf)>(se.syserr, buf);
        return fmt::format_to(ctx.out(), "{}", p);
    }
};

} // namespace fmt
