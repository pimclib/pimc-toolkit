#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "PIMSMConfig.hpp"

namespace pimc {

auto loadIPv4PIMSMConfig(yaml::ValueContext const& pimsmCfgCtx)
-> Result<PIMSMConfig<net::IPv4Address>, std::vector<yaml::ErrorContext>>;

} // namespace pimc
