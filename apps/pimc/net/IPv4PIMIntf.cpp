#include "pimc/unix/CapNetRaw.hpp"

#include "IPv4PIMIntf.hpp"

namespace pimc {


auto IPv4PIMIntf::create(
        char const* progname, PIMCConfig<IPv4> const& cfg,bool& stopped)
-> Result<IPv4PIMIntf, std::string> {

}

} // namespace pimc
