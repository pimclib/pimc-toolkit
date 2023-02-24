#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "pimc/net/dns-utils.hpp"

namespace pimc {
namespace net {

std::tuple<DnsResult::OpCode, std::string, std::string>
DnsUtils::resolve_addr(IPv4Address addr) {
    sockaddr_in sin{};
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = addr.to_nl();

    char hostname[NI_MAXHOST];
    int rc = getnameinfo(
            reinterpret_cast<sockaddr *>(&sin), sizeof(sin),
            hostname, NI_MAXHOST, nullptr, 0, 0);
    if (rc != 0) {
        if (rc == EAI_NONAME) {
            return std::make_tuple(
                    DnsResult::OpCode::UnknownHost,
                    std::string{},
                    std::string{});
        }

        return std::make_tuple(
                DnsResult::OpCode::OtherError,
                std::string{},
                std::string{gai_strerror(rc)});
    }

    return std::make_tuple(
            DnsResult::OpCode::Success,
            std::string{hostname},
            std::string{});
}

std::tuple<DnsResult::OpCode, IPv4Address, std::string>
DnsUtils::resolve_name(const char *name) {
    addrinfo hints{};
    addrinfo *result_ptr = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    int rc = getaddrinfo(name, nullptr, &hints, &result_ptr);

    if (rc != 0) {
        if (result_ptr != nullptr) freeaddrinfo(result_ptr);

        if (rc == EAI_NONAME)
            return std::make_tuple(
                    DnsResult::OpCode::UnknownHost,
                    IPv4::Default,
                    std::string{});

        return std::make_tuple(
                DnsResult::OpCode::OtherError,
                IPv4::Default,
                std::string{gai_strerror(rc)});
    }

    auto result = std::make_tuple(
            DnsResult::OpCode::Success,
            pimc::net::IPv4Address::from_nl(
                    reinterpret_cast<sockaddr_in *>
                    (result_ptr->ai_addr)->sin_addr.s_addr),
            std::string{});

    freeaddrinfo(result_ptr);
    return result;
}

} // namespace net
} // namespace pimc
