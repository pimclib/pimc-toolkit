#include "pimc/formatters/Fmt.hpp"
#include "pimc/unix/SignalHandler.hpp"

#include "Config.hpp"
#include "OutputHandler.hpp"
#include "Receiver.hpp"
#include "IPRawReceiver.hpp"
#include "Sender.hpp"

namespace {

bool stopped{false};

} // anon.namespace

int main(int argc, char** argv) {
    char const* progname = argv[0];
    try {
        auto const cfg = pimc::Config::fromArgs(argc, argv);

        if (cfg.showConfig()) {
            cfg.show();
            return 0;
        }

        pimc::SignalHandler<>::install
        <SIGINT, SIGTERM, SIGHUP>([] (int) { stopped = true; });

        pimc::OutputHandler oh{cfg};
        if (not cfg.sender()) {
            if (cfg.count() == 0) {
                if (not cfg.wildcard()) {
                    pimc::Receiver<pimc::UnlimitedPackets> r{cfg, oh, stopped};
                    r.run(progname);
                } else {
                    pimc::IPRawReceiver<pimc::UnlimitedPackets> r{cfg, oh, stopped};
                    r.run(progname);
                }
            } else {
                if (not cfg.wildcard()) {
                    pimc::Receiver<pimc::LimitedPackets> r{cfg, oh, stopped};
                    r.run(progname);
                } else {
                    pimc::IPRawReceiver<pimc::LimitedPackets> r{cfg, oh, stopped};
                    r.run(progname);
                }
            }
        } else {
            pimc::Sender s{cfg, oh, stopped};
            s.run();
        }

        return 0;
    } catch (std::runtime_error const& ex) {
        fmt::print(stderr, "error: {}\n", ex.what());
        return 1;
    }
}
