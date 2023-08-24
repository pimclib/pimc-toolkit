#include "pimc/formatters/Fmt.hpp"

#include "pimc/unix/GetOptLong.hpp"

#include "config/Config.hpp"
#include "net/IPv4PIMIntf.hpp"

int main(int argc, char** argv) {
    try {
        auto cfg = pimc::loadIPv4Config(argc, argv);
    } catch (pimc::CommandLineError const& cliErr){
        fmt::print(stderr, "error: {}\n", cliErr.what());
        return 2;
    } catch (std::runtime_error const& rtErr) {
        fmt::print(stderr, "error: {}\n", rtErr.what());
        return 1;
    }
    return 0;
}
