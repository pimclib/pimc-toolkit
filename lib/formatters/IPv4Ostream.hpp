#pragma once

#include <ostream>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"

inline std::ostream& operator<< (
        std::ostream& os, const pimc::net::IPv4Address addr) {
    char buf[64];
    snprintf(buf, 64, "%u.%u.%u.%u",
            addr.oct1(), addr.oct2(), addr.oct3(), addr.oct4());
    os << buf;
    return os;
}

inline std::ostream& operator<< (
        std::ostream& os, const pimc::net::IPv4Prefix prefix) {
    auto addr = prefix.address();
    char buf[64];
    snprintf(buf, 64, "%u.%u.%u.%u/%u",
            addr.oct1(), addr.oct2(), addr.oct3(), addr.oct4(),
            prefix.length());

    os << buf;
    return os;
}
