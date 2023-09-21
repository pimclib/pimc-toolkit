#pragma once

#include <string>

#include "pimc/core/Result.hpp"

namespace pimc {

Result<int, std::string> openIPv4PIMSocket();

} // namespace pimc
