#pragma once

#include <string>

#include "pimc/core/Result.hpp"

namespace pimc {

Result<int, std::string> openIPv4RawSocket(char const* progname);

} // namespace pimc
