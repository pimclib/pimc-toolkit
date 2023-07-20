#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "pimc/formatters/Fmt.hpp"

#include "pimc/core/Deferred.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/text/MemoryBuffer.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#include "IPv4IntfTable.hpp"

namespace pimc {

Result<IPv4IntfTable, std::string> IPv4IntfTable::newTable() {
    ifaddrs *ifaddrList;

    if (getifaddrs(&ifaddrList) == -1) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);
        fmt::format_to(bi, "getifaddrs() failed: {}", SysError{});
        return fail(fmt::to_string(buf));
    }

    auto d = defer([ifaddrList] { freeifaddrs(ifaddrList); });

    std::vector<IPv4IntfInfo> intfs;
    for (auto p = ifaddrList; p!= nullptr; p = p->ifa_next) {
        if (p->ifa_addr == nullptr) continue;

        if (p->ifa_addr->sa_family == AF_INET) {
            unsigned ifindex = if_nametoindex(p->ifa_name);
            if (ifindex == 0) {
                auto& buf = getMemoryBuffer();
                auto bi = std::back_inserter(buf);
                fmt::format_to(
                        bi,
                        "unable to resolve interface name '{}' to index",
                        p->ifa_name);
                return fail(fmt::to_string(buf));
            }

            auto const* sin = reinterpret_cast<sockaddr_in const*>(p->ifa_addr);
            intfs.emplace_back(IPv4IntfInfo{
                .name = p->ifa_name,
                .ifindex = ifindex,
                .address = IPv4Address::from_nl(sin->sin_addr.s_addr)
            });
        }
    }
    if (intfs.empty())
        return fail("interface table is empty");

    std::unordered_map<unsigned, IPv4IntfInfo const*> indexMap;
    std::unordered_map<std::string, IPv4IntfInfo const*> nameMap;
    for (auto const& intfinfo: intfs) {
        indexMap[intfinfo.ifindex] = &intfinfo;
        nameMap[intfinfo.name] = &intfinfo;
    }

    return IPv4IntfTable{
        std::move(intfs),
        std::move(indexMap),
        std::move(nameMap)
    };
}

} // namespace pimc