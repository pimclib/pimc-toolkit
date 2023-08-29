#include <cstdint>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <string>
#include "pimc/core/Result.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/FailureFormatter.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "SocketUtils.hpp"

namespace pimc {

Result<int, std::string> makeNonBlocking(int s) {
    // Make socket non-blocking
    int flags = fcntl(s, F_GETFL);
    if (flags == -1)
        return sfail(
                "fcntl() failed to get socket flags: {}", SysError{});

    flags |= O_NONBLOCK;
    fcntl(s, F_SETFL, flags);
    if (flags == -1)
        return sfail(
                "fcntl() failed to make socket non-blocking: {}", SysError{});

    return s;
}

Result<int, std::string> allowReuse(int s) {
    // allow multiple sockets use the same UDP ports
    int allowReuse = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                   &allowReuse, sizeof(allowReuse)) == -1)
        return sfail("enabling transport port reuse failed: {}", SysError{});

    return s;
}

Result<int, std::string> setRcvdBuffSize(int s, int bufsz) {
    int bufSize{bufsz};
    if (setsockopt(s, SOL_SOCKET,
                   SO_RCVBUF, &bufSize, sizeof(bufSize)) == -1) {
        return sfail(
                "failed to set receive buffer size to {} bytes: {}",
                bufSize, SysError{});
    }

    return s;
}

Result<int, std::string> setMulticastTTL(int s, uint8_t ttl) {
    if (setsockopt(s, IPPROTO_IP,
                   IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1)
        return sfail(
                "unable to set multicast TTL to {}: {}",
                static_cast<unsigned>(ttl), SysError{});

    return s;
}

} // namespace pimc
