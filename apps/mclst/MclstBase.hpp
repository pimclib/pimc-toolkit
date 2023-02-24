#pragma once

#include <unistd.h>

#include "Config.hpp"
#include "OutputHandler.hpp"

namespace pimc {

class MclstBase {
protected:
    Config const& cfg_;
    OutputHandler& oh_;
    int socket_;
    bool& stopped_;

    constexpr MclstBase(Config const& cfg, OutputHandler& oh, bool& stopped)
    : cfg_{cfg}, oh_{oh}, socket_{-1}, stopped_{stopped} {}

    ~MclstBase() {
        if (socket_ != -1) {
            int rc;
            do {
                rc = close(socket_);
            } while (rc == -1 and errno == EINTR);
        }
    }
};

} // namespace pimc
