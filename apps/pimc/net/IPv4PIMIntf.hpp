#pragma once

#include <unistd.h>

#include "pimc/core/Result.hpp"
#include "config/PIMCConfig.hpp"

namespace pimc {

class IPv4PIMIntf final {
public:
    static auto create(char const* progname, PIMCConfig<IPv4> const& cfg)
    -> Result<IPv4PIMIntf, std::string>;

    IPv4PIMIntf(IPv4PIMIntf const&) = delete;
    IPv4PIMIntf(IPv4PIMIntf&& rhs) noexcept: socket_{rhs.socket_} {
        rhs.socket_ = -1;
    }

    IPv4PIMIntf& operator= (IPv4PIMIntf const&) = delete;
    IPv4PIMIntf& operator= (IPv4PIMIntf&& rhs) noexcept {
        if (&rhs == this) return *this;

        socket_ = rhs.socket_;
        rhs.socket_ = -1;

        return *this;
    }

    auto send(void const* pktData, size_t sz) const -> Result<void, std::string>;

    ~IPv4PIMIntf() {
        if (socket_ != -1) {
            int rc;
            do {
                rc = close(socket_);
            } while (rc == -1 and errno == EINTR);
        }
    }

private:
    constexpr explicit IPv4PIMIntf(int socket): socket_{socket} {}

private:
    int socket_;
};

} // namespace pimc
