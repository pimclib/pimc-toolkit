#pragma once

#include <string>
#include "pimc/core/Result.hpp"

namespace pimc {

Result<int, std::string> makeNonBlocking(int s);

Result<int, std::string> allowReuse(int s);

Result<int, std::string> setRcvdBuffSize(int s, int bufsz);

Result<int, std::string> setMulticastTTL(int s, uint8_t ttl);

} // namespace pimc
