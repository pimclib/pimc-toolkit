#pragma once

#include "pimc/core/TypeUtils.hpp"
#include "IPv4Address.hpp"

namespace pimc::net {

template <typename A>
concept IPAddress = OneOf<A, IPv4Address>;

} // namespace pimc::net
