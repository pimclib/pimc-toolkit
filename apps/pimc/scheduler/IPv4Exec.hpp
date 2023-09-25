#pragma once

#include "config/Config.hpp"
#include "logging/Logging.hpp"

namespace pimc {

bool ipv4exec(
        PIMCConfig<IPv4> const& cfg, Logger& log, char const* progname, bool& stopped);

} // namespace pimc
