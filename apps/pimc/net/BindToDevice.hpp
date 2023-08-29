#pragma once

#include <string>
#include "pimc/core/Result.hpp"

namespace pimc {

Result<int, std::string> bindToDevice(
        int s, bool ipv6, char const* intfName, unsigned intfIndex);

} // namespace pimc
