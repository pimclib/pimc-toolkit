#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "pimc/core/Deferred.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/net/SocketUtils.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/FailureFormatter.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "IPv4PIMIntf.hpp"
#include "IPv4RawSocket.hpp"
#include "BindToDevice.hpp"

namespace pimc {

auto IPv4PIMIntf::create(
        char const* progname, PIMCConfig<IPv4> const& cfg)
-> Result<IPv4PIMIntf, std::string> {
    auto rs = openIPv4PIMSocket(progname)
            .flatMap([] (int s) { return allowReuse(s); });

    if (not rs)
        return fail(std::move(rs).error());

    int s = rs.value();
    auto d = defer([s] {
        int rc;
        do {
            rc = close(s);
        } while (rc == -1 and errno == EINTR);
    });

    unsigned ipHdrOpt{1};
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &ipHdrOpt, sizeof(ipHdrOpt)) == -1)
        return sfail(
                "unable to configure socket to be supplied with a custom IP header: {}",
                SysError{});

    auto rbd = bindToDevice(
            s, false,
            cfg.pimsmConfig().intfName().c_str(), cfg.pimsmConfig().intfIndex());

    if (not rbd)
        return fail(std::move(rbd).error());

    d.cancel();
    return IPv4PIMIntf{cfg, s};
}

} // namespace pimc
