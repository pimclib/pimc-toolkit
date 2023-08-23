#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Deferred.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "IntfTable.hpp"

namespace pimc {

Result<IntfTable, std::string> IntfTable::newTable() {
    ifaddrs *ifaddrList;
    fmt::memory_buffer buf;

    if (getifaddrs(&ifaddrList) == -1) {
        buf.clear();
        auto bi = std::back_inserter(buf);
        fmt::format_to(bi, "getifaddrs() failed: {}", SysError{});
        return fail(fmt::to_string(buf));
    }

    auto d = defer([ifaddrList] { freeifaddrs(ifaddrList); });

    std::map<unsigned, IntfInfo> indexMap;
    std::map<std::string, IntfInfo const*> nameMap;

    for (auto p = ifaddrList; p!= nullptr; p = p->ifa_next) {

        if (p->ifa_addr == nullptr) continue;

        unsigned ifindex = if_nametoindex(p->ifa_name);
        if (ifindex == 0) {
            buf.clear();
            auto bi = std::back_inserter(buf);
            fmt::format_to(
                    bi,
                    "unable to resolve interface name '{}' to index",
                    p->ifa_name);
            return fail(fmt::to_string(buf));
        }

        Optional<IPv4Address> ipv4addr;
        if (p->ifa_addr->sa_family == AF_INET) {
            auto const* sin = reinterpret_cast<sockaddr_in const*>(p->ifa_addr);
            ipv4addr = IPv4Address::from_nl(sin->sin_addr.s_addr);
        }

        auto idxInsOp = indexMap.try_emplace(ifindex, p->ifa_name, ifindex, ipv4addr);
        auto& intfInfo = idxInsOp.first->second;

        if (not idxInsOp.second) {
            // sanity check, all entries with the same index must have
            // the same interface name
            if (intfInfo.name != p->ifa_name)
                raise<std::logic_error>(
                        "intf #{}, previously seen name '{}' != currently seen name '{}'",
                        intfInfo.ifindex, intfInfo.name, p->ifa_name);
            if (ipv4addr)
                intfInfo.ipv4addr = ipv4addr;
        } else nameMap.try_emplace(intfInfo.name, &intfInfo);
    }

    if (indexMap.empty())
        return fail("interface table is empty");

    return IntfTable{
        std::move(indexMap),
        std::move(nameMap)
    };
}

} // namespace pimc
