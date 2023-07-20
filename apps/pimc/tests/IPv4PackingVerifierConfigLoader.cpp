#pragma once

#include <vector>

#include "pimc/core/Result.hpp"
#include "pimc/yaml/Structured.hpp"

#include "pimc/net/IPv4Address.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"
#include "pimc/packets/PIMSMv2.hpp"
#include "pimc/yaml/BuilderBase.hpp"

#include "config/ConfigUtils.hpp"
#include "pimsm/Update.hpp"

namespace pimc {

auto loadIPv4PackingVerificationConfig(yaml::ValueContext const& jpCfgCtx)
        -> Result<Update<IPv4Address>, std::vector<yaml::ErrorContext>> {

}

} // namespace pimc
