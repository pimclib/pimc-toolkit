#pragma once

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "PIMCConfig.hpp"

namespace pimc {

auto loadIPv4PIMCConfig(yaml::ValueContext const& cfgfgCtx)
-> Result<PIMCConfig<net::IPv4Address>, std::vector<yaml::ErrorContext>>;

} // namespace pimc
