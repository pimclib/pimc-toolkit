#pragma once

#include "pimc/net/IP.hpp"

#include "PIMCConfig.hpp"

namespace pimc {

PIMCConfig<v4> loadIPv4Config(int argc, char** argv);

} // namespace pimc
