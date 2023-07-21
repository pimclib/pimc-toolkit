#pragma once

#include <cstring>
#include <ostream>

#include "pimc/system/SysError.hpp"

#include "detect_strerror_r.hpp"

inline std::ostream& operator<< (
        std::ostream& os, pimc::SysError const& se) {
    char buf[1024];
    os << pimc::detail::invoke_strerror_r<sizeof(buf)>(se.syserr, buf);
    return os;
}
