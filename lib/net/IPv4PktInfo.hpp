#pragma once

#include <netinet/in.h>

#include "pimc/core/CompilerUtils.hpp"

#ifdef __APPLE__
#define IF_INDEX(v) (v)
#endif

#ifdef __linux__
#define IF_INDEX(v) static_cast<unsigned>(v)
#endif

namespace pimc {



} // namespace pic