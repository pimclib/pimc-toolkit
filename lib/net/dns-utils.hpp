#pragma once

#include <string>
#include <tuple>

#include "pimc/net/IPv4Net.hpp"

namespace pimc {
namespace net {

/**
 * The DnsResult namespace contains the parameters to the result
 * tuple returned by the DnsUtils static methods
 */
namespace DnsResult {

static constexpr unsigned Status = 0;
static constexpr unsigned Value = 1;
static constexpr unsigned Error = 2;

enum class OpCode : unsigned {
    Success = 0,
    UnknownHost = 1,
    OtherError = 2
};
}

/**
 * A factory class which contains helper DNS static methods.
 */
class DnsUtils final {
public:
    /**
     * Returns a tuple, which, if this call succeeds, contains opcode SUCCESS
     * and the name corresponding to the specified IP v4 address. If this call
     * fails, the tuple contains the corresponding error opcode and if the
     * opcode is not UNKNOWN_HOST, the last parameter of the tuple provides
     * the textual description of the error.
     *
     * @param addr the IP v4 address to resolve
     * @return the result
     */
    static std::tuple<DnsResult::OpCode, std::string, std::string>
    resolve_addr(IPv4Address addr);

    /**
     * Returns a tuple, which, if this call succeeds, contains opcode SUCCESS
     * and the IP v4 address corresponding to the specified name. If this call
     * fails, the tuple contains the corresponding error opcode and if the
     * opcode is not UNKNOWN_HOST, the last parameter of the tuple provides
     * the textual description of the error.
     *
     * @param name the name to resolve to IP v4 address
     * @return the result
     */
    static std::tuple<DnsResult::OpCode, IPv4Address, std::string>
    resolve_name(const char *name);

    /**
     * Returns a tuple, which, if this call succeeds, contains opcode SUCCESS
     * and the IP v4 address corresponding to the specified name. If this call
     * fails, the tuple contains the corresponding error opcode and if the
     * opcode is not UNKNOWN_HOST, the last parameter of the tuple provides
     * the textual description of the error.
     *
     * @param name the name to resolve to IP v4 address
     * @return the result
     */
    inline static std::tuple<DnsResult::OpCode, IPv4Address, std::string>
    resolve_name(const std::string &name) { return resolve_name(name.c_str()); }

    DnsUtils() = delete;

    ~DnsUtils() = delete;
};


} // namespace net
} // namespace pimc


