#pragma once

#include "pimc/core/TypeUtils.hpp"

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"

namespace pimc {

struct v4 final {};

struct v6 final {};

using IPv4 = v4;
using IPv6 = v6;

template <typename V>
concept IPVersion = OneOf<V, v4, v6>;

template <IPVersion V>
struct IP {};

template <>
struct IP<v4> {
    using Address = IPv4Address;
    using Prefix = IPv4Prefix;
};

} // namespace pimc