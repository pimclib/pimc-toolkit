#pragma once

#include "pimc/time/TimeUtils.hpp"

#include "Config.hpp"

namespace pimc {

/*!
 * \brief A timer to report the times when packets are received
 * or when timeouts have been experienced.
 *
 * One of the functions of this object is to track accurate
 * timeouts. The poller mechanism, whether `select()` or `poll()`
 * or `epoll()` works fine but only for non-raw sockets. In the
 * case of a raw UDP socket we actually receive all UDP traffic
 * destined for the host, thus we need to filter only the multicast
 * packets we're interested in. As a side effect, this circumstance
 * disables the timeout reported by the poller whenever the poller
 * detects uninteresting traffic on the socket.
 */
class Timer {
public:
    explicit Timer(Config const& cfg)
    : startNs_{gethostnanos()}
    , timestampNs_{startNs_}
    , timeoutNs_{static_cast<uint64_t>(cfg.timeoutSec()) * 1'000'000'000} {}

    /*!
     * This function should be called right after the poller returns to
     * save the host time of which is as close as possible to the poller
     * event.
     */
    void save() { timestampNs_ = gethostnanos(); }

    /*!
     * This function should be called after receiving a packet of interest
     * or right after reporting the timeout.
     */
    void reset() { startNs_ =  timestampNs_; }

    [[nodiscard]]
    constexpr bool timeout() const {
        return timestampNs_ - startNs_ >= timeoutNs_;
    }

    /*!
     * \brief Returns the current timestamp.
     *
     * @return the current timestamp
     */
    [[nodiscard]]
    uint64_t timestamp() const { return timestampNs_; }

private:
    uint64_t startNs_;
    uint64_t timestampNs_;
    uint64_t const timeoutNs_;
};

} // namespace pimc
