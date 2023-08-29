#include <string>

#include "pimc/core/Result.hpp"
#include "pimc/unix/CapNetRaw.hpp"

#include <sys/socket.h>
#include <netinet/in.h>

#include "pimc/formatters/Fmt.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"
#include "pimc/formatters/FailureFormatter.hpp"

#include "IPv4RawSocket.hpp"

namespace pimc {

namespace {
char const* LastResortMsg =
#ifdef WITH_LIBCAP
        "unable to open IPv4 PIM socket: "
        "even though the process now has the effective CAP_NET_RAW; "
        "as a last resort try running under sudo";
#else
        "unable to open IPv4 PIM socket, "
        "try running under sudo";
#endif

} // anon.namespace

Result<int, std::string> openIPv4PIMSocket(char const* progname) {
    auto rc = CapNetRaw::raise(progname);

    if (not rc)
        return fail(std::move(rc).error());

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_PIM);

    if (s == -1) {
        if (errno == EPERM)
            return fail(LastResortMsg);
        else
            return sfail("unable to open PIM IP socket: {}", SysError{});
    }

    return s;
}

} // namespace pimc
