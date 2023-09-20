#include "pimc/unix/SignalHandler.hpp"
#include "pimc/unix/GetOptLong.hpp"

#include "config/Config.hpp"
#include "logging/Logging.hpp"
#include "scheduler/IPv4Exec.hpp"

namespace {

bool stopped{false};

} // anon.namespace

int main(int argc, char** argv) {
    try {
        pimc::SignalHandler<>::install
                <SIGINT, SIGTERM, SIGHUP>([] (int) { stopped = true; });

        auto cfg = pimc::loadIPv4Config(argc, argv);
        pimc::Logger log = pimc::Logger::logger(cfg.loggingConfig());
        if (not pimc::ipv4exec(cfg, log, argv[0], stopped))
            return 1;
    } catch (pimc::CommandLineError const& cliErr){
        fmt::print(stderr, "error: {}\n", cliErr.what());
        return 2;
    } catch (std::runtime_error const& rtErr) {
        fmt::print(stderr, "error: {}\n", rtErr.what());
        return 1;
    }

    return 0;
}
