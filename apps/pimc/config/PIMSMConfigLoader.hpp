#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "PIMSMConfig.hpp"

namespace pimc {

auto loadPIMSMConfig(yaml::ValueContext const& pimsmCfgCtx)
-> Result<PIMSMConfig, std::vector<yaml::ErrorContext>>;

} // namespace pimc
