#pragma once

#include <cerrno>
#include <string>

namespace pimc {

std::string sysError(int syserr = errno);

} // namespace pimc