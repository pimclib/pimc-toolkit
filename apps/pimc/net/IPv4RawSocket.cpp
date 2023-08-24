#pragma once

#include <string>

#include "pimc/core/Result.hpp"
#include "pimc/unix/CapNetRaw.hpp"

#include "IPv4RawSocket.hpp"

namespace pimc {

namespace {
inline static char const* LastResortMsg =
#ifdef WITH_LIBCAP
        "unable to open IPv4 raw socket: "
        "even though the process now has the effective CAP_NET_RAW; "
        "as a last resort try running under sudo";
#else
        "unable to open IPv4 raw socket, "
        "try running under sudo";
#endif

} // anon.namespace

Result<int, std::string> openIPv4RawSocket(char const* progname) {
    
}

} // namespace pimc
