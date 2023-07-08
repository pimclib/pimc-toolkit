#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "JPConfig.hpp"

namespace pimc {

auto loadJPConfig(yaml::ValueContext const& jpCfgCtx)
-> Result<JPConfig, std::vector<yaml::ErrorContext>>;

} // namespace pimc
