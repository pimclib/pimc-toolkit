#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "JPConfig.hpp"

namespace pimc {

auto loadIPv4JPConfig(yaml::ValueContext const& jpCfgCtx)
-> Result<JPConfig<net::IPv4Address>, std::vector<yaml::ErrorContext>>;

} // namespace pimc
