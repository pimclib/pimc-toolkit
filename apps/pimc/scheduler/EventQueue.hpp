#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>

#include "pimc/time/TimeUtils.hpp"

using namespace std::chrono_literals;

namespace pimc {

struct IRunner {
    virtual void run(uint64_t ts) = 0;
    virtual ~IRunner() = default;
};

class EventQueue final {
private:
    struct Event {
        constexpr Event(uint64_t ts, IRunner& r): ts_{ts}, pr_{&r} {}
        constexpr Event(uint64_t ts, IRunner* pr): ts_{ts}, pr_{pr} {}

        uint64_t ts_;
        IRunner* pr_;
    };

    struct EventLess {
        bool operator() (Event const& a, Event const& b) const {
            return a.ts_ > b.ts_;
        }
    };

    static inline std::vector<Event> withCapacity(size_t sz) {
        std::vector<Event> c;
        c.reserve(sz);
        return c;
    }
public:
    explicit EventQueue(size_t sz): pq_{eless_, withCapacity(sz)} {}

    void schedule(unsigned seconds, IRunner* rp) {
        uint64_t ts = gethostnanos() + (NanosInSecond * seconds);
        pq_.emplace(ts, rp);
    }

    void run() {
        while (not pq_.empty()) {
            auto now = pimc::gethostnanos();
            bool cancel{true};

            do {
                auto const &event = pq_.top();
                if (event.ts_ <= now) {
                    event.pr_->run(now);
                    pq_.pop();
                    if (pq_.empty())
                        return;
                    cancel = false;
                } else cancel = true;
            } while (not cancel);

            std::this_thread::sleep_for(100ms);
        }
    }

private:
    EventLess eless_;
    std::priority_queue<Event, std::vector<Event>, EventLess> pq_;
};

} // namespace pimc