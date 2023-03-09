#pragma once

#include <cstring>
#include <fmt/format.h>

#include "pimc/system/SysError.hpp"

namespace fmt {

template <>
struct formatter<pimc::SysError>: formatter<string_view> {

    template <typename FormatContext>
    auto format(pimc::SysError const& se, FormatContext& ctx) {
        char buf[1024];
        strerror_r(se.syserr, buf, sizeof(buf));
        return format_to(ctx.out(), "{}", buf);
    }
};

} // namespace fmt