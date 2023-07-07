#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/StructuredYaml.hpp"

#include "JPConfig.hpp"

namespace pimc {

auto loadJPConfig(yaml::ValueContext const& jpConfigCtx)
-> Result<JPConfig, std::vector<yaml::ErrorContext>>;

} // namespace pimc
