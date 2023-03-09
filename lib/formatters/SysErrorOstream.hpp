#pragma once

#include <cstring>
#include <ostream>

#include "pimc/system/SysError.hpp"

inline std::ostream& operator<< (
        std::ostream& os, pimc::SysError const& se) {
    char buf[1024];
    strerror_r(se.syserr, buf, sizeof(buf));
    os << buf;
    return os;
}
