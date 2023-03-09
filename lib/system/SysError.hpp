#pragma once

#include <cerrno>
#include <string>

namespace pimc {

struct SysError final {
    int const syserr;

    SysError(): syserr{errno} {}

    constexpr explicit SysError(int ec): syserr{ec} {}
};

} // namespace pimc