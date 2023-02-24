#define _GNU_SOURCE 1

#include <cstring>
#include <string>

#include "pimc/system/SysError.hpp"

namespace pimc {

std::string sysError(int syserr) {
    char buf[1024];
    strerror_r(syserr, buf, sizeof(buf));
    return std::string{buf};
}

}