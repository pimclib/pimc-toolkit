#pragma once

#include "PIMCConfig.hpp"

namespace pimc {

PIMCConfig<net::IPv4Address> loadIPv4Config(int argc, char** argv);

} // namespace pimc
