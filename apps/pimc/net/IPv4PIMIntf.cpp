#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "pimc/core/Deferred.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/unix/CapState.hpp"
#include "pimc/net/SocketUtils.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/FailureFormatter.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "pimsm/PIMSMParams.hpp"
#include "IPv4PIMIntf.hpp"
#include "IPv4RawSocket.hpp"
#include "BindToDevice.hpp"

namespace pimc {

auto IPv4PIMIntf::create(
        char const* progname, PIMCConfig<IPv4> const& cfg, Logger& log)
-> Result<IPv4PIMIntf, std::string> {
    auto rcap = CapState::program(progname)
            .raise(CAP_(NET_RAW), CAP_(NET_BIND_SERVICE));

    if (not rcap)
        return fail(std::move(rcap).error());

    auto rs = openIPv4PIMSocket()
            .flatMap([] (int s) { return allowReuse(s); });

    if (not rs)
        return fail(std::move(rs).error());

    log.debug("created IPv4 PIM socket");

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

    log.debug(
            "bound the IPv4 PIM socket to device {} (#{})",
            cfg.pimsmConfig().intfName(), cfg.pimsmConfig().intfIndex());

    d.cancel();
    return IPv4PIMIntf{s};
}

auto IPv4PIMIntf::send(
        void const* pktData, size_t sz, std::string const& pktName) const
-> Result<void, std::string> {
    sockaddr_in sinPim;
    memset(&sinPim, 0, sizeof(sinPim));
    sinPim.sin_family = AF_INET;
    sinPim.sin_addr.s_addr = pimsm::params<IPv4>::AllPIMRouters.to_nl();

    if (sendto(socket_, pktData, sz, 0,
               reinterpret_cast<sockaddr*>(&sinPim), sizeof(sinPim)) == -1)
        return sfail("unable to send PIM {} packet: {}", pktName, SysError{});

    return {};
}

} // namespace pimc
