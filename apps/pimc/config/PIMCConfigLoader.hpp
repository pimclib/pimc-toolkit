#pragma once

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "PIMCConfig.hpp"

namespace pimc {

auto loadPIMCConfig(yaml::ValueContext const& cfgfgCtx)
-> Result<PIMCConfig, std::vector<yaml::ErrorContext>>;

} // namespace pimc
