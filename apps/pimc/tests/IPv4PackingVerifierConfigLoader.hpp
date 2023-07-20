#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "pimsm/Update.hpp"

namespace pimc {

auto loadIPv4PackingVerificationConfig(yaml::ValueContext const& jpCfgCtx)
        -> Result<Update<IPv4Address>, std::vector<yaml::ErrorContext>>;

} // namespace pimc
