#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/text/Location.hpp"
#include "pimc/yaml/Structured.hpp"

#include "JPConfig.hpp"

namespace pimc {

auto loadJPConfig(yaml::ValueContext const& jpCfgCtx, Location jpCfgLoc)
-> Result<JPConfig, std::vector<yaml::ErrorContext>>;

} // namespace pimc
