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
    IPv4PIMIntf(IPv4PIMIntf&& rhs) noexcept: cfg_{rhs.cfg_}, socket_{rhs.socket_} {
        rhs.socket_ = -1;
    }

    IPv4PIMIntf& operator= (IPv4PIMIntf const&) = delete;
    IPv4PIMIntf& operator= (IPv4PIMIntf&& rhs) noexcept {
        if (&rhs == this) return *this;

        socket_ = rhs.socket_;
        rhs.socket_ = -1;

        return *this;
    }

    ~IPv4PIMIntf() {
        if (socket_ != -1) {
            int rc;
            do {
                rc = close(socket_);
            } while (rc == -1 and errno == EINTR);
        }
    }

private:
    constexpr IPv4PIMIntf(PIMCConfig<IPv4> const& cfg, int socket)
    : cfg_{cfg}, socket_{socket} {}

private:
    PIMCConfig<IPv4> const& cfg_;
    int socket_;
};

} // namespace pimc
