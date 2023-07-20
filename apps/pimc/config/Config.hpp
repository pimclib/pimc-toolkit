#pragma once

#include "pimc/net/IP.hpp"

#include "PIMCConfig.hpp"

namespace pimc::pimsm_config {

PIMCConfig<v4> loadIPv4Config(int argc, char** argv);

} // namespace pimc::pimsm_config
