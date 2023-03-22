#pragma once

#include <cstring>

#include <fmt/format.h>

#include "pimc/core/TypeUtils.hpp"

namespace pimc::detail {

inline auto call_strerror_r(int ec, char* buf, size_t sz) {
    return ::strerror_r(ec, buf, sz);
}

constexpr bool GnuResult =
        IsOneOf_v<
                std::invoke_result_t<decltype(call_strerror_r), int, char*, size_t>,
                char*, char const*>;

template <size_t SZ>
char const* invoke_strerror_r(int ec, char* buf) requires GnuResult {
    return strerror_r(ec, buf, SZ);
}

template <size_t SZ>
char const* invoke_strerror_r(int ec, char* buf) requires (not GnuResult) {
    auto rc = ::strerror_r(ec, buf, SZ);
    if (rc != 0) {
        auto r = fmt::format_to_n(buf, SZ-1, "unknown error {}", ec);
        *r.out = static_cast<char>(0);
    }
    return buf;
}

} // namespace pimc::detail
