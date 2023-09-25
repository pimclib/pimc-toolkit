#include <chrono>
#include <thread>

#include "pimc/formatters/Fmt.hpp"
#include "pimc/events/FixedEventQueue.hpp"
#include "pimc/time/TimeUtils.hpp"
#include "pimc/formatters/MemoryBuffer.hpp"
#include "pimc/text/Plural.hpp"

#include "config/Config.hpp"
#include "config/Formatters.hpp"
#include "logging/Logging.hpp"
#include "net/IPv4PIMIntf.hpp"
#include "pimsm/UpdateFormatter.hpp"
#include "Timer.hpp"
#include "IPv4HelloEvent.hpp"
#include "IPv4GoodbyeEvent.hpp"
#include "IPv4JPUpdateEvent.hpp"
#include "IPv4GoodbyeJPUpdateEvent.hpp"
#include "IPv4Exec.hpp"

using namespace std::chrono_literals;

namespace pimc {

bool ipv4exec(
        PIMCConfig<IPv4> const& cfg, Logger& log, char const* progname, bool& stopped) {
    auto ts = gethostnanos();
    log.debug(ts, "PIM SM config:\n{}", cfg.pimsmConfig());
    log.debug(ts, "Join/Prune Config:\n {}", cfg.jpConfig());
    if (log.enabled(Level::Debug)) {
        auto const& updates = cfg.updates();
        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(
                bi, "Will be sending {} update{}:\n",
                updates.size(), plural(updates));
        unsigned n{1};
        for (auto const &update: cfg.updates()){
            std::tuple<unsigned, Update<pimc::IPv4> const &> t{n++, update};
            fmt::format_to(bi, "{}", t);
        }
        mb.push_back(static_cast<char>(0));
        log.debug(ts, "{}", mb.data());

        auto const& inverseUpdates = cfg.inverseUpdates();
        mb.clear();
        bi = std::back_inserter(mb);
        fmt::format_to(
                bi, "Once terminated will send {} inverse update{}:\n",
                inverseUpdates.size(), plural(inverseUpdates));
        n = 1;

        for (auto const& update: inverseUpdates) {
            std::tuple<unsigned, Update<pimc::IPv4> const &> t{n++, update};
            fmt::format_to(bi, "{}", t);
        }
        mb.push_back(static_cast<char>(0));
        log.debug(ts, "{}", mb.data());
    }

    auto rPIMIntf = IPv4PIMIntf::create(progname, cfg, log);
    if (rPIMIntf) {
        Timer timer;
        IPv4GoodbyeEvent goodbyeEvent{log, rPIMIntf.value(), cfg.pimsmConfig()};
        IPv4GoodbyeJPUpdateEvent goodbyeJpUpdateEvent{log, rPIMIntf.value(), cfg};
        FixedEventQueue<std::string, IPv4HelloEvent, IPv4JPUpdateEvent> events{
            std::forward_as_tuple(log, rPIMIntf.value(), timer, cfg.pimsmConfig()),
            std::forward_as_tuple(log, rPIMIntf.value(), timer, cfg)};

        while (not stopped) {
            timer.update();
            auto r = events.runOnce();

            if (not r) {
                log.error(timer.cts(), "{}", r.error());
                return false;
            }

            std::this_thread::sleep_for(100ms);
        }

        timer.update();
        auto r1 = goodbyeJpUpdateEvent.send();
        if (not r1) {
            log.error(timer.cts(), "{}", r1.error());
            return false;
        }

        timer.update();
        auto r2 = goodbyeEvent.send();
        if (not r2) {
            log.error(timer.cts(), "{}", r2.error());
            return false;
        }
    } else {
        log.error("{}", rPIMIntf.error());
        return false;
    }

    return true;
}

} // namespace pimc
