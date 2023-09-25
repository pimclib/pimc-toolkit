#include <unistd.h>
#include <tuple>
#include <string>
#include <string_view>
#include <algorithm>

#include "pimc/system/Exceptions.hpp"
#include "pimc/net/IPv4Address.hpp"
#include "pimc/parsers/NumberParsers.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"
#include "pimc/formatters/MemoryBuffer.hpp"
#include "pimc/formatters/IPv4Formatters.hpp"
#include "pimc/formatters/IntfTableFormatter.hpp"
#include "pimc/text/NumberLengths.hpp"
#include "pimc/unix/GetOptLong.hpp"
#include "version.hpp"

#include "Config.hpp"

#define OID(id) static_cast<uint32_t>(Options::id)


namespace pimc {

namespace {

enum class Options: uint32_t {
    Interface = 1,
    SourceOfG = 2,
    Timeout = 3,
    ShowPayload = 4,
    Sender = 5,
    SetTTL = 6,
    Count = 7,
    NoColors = 8,
    ShowConfig = 9,
    ShowVersion = 10,
};

char const* header =
    "[Options] group[:port]\n\n"
    "where group[:port] may be specified either as 'group:port', e.g. 239.1.2.3:12345\n"
    "or just as a group, e.g. 239.1.2.3, which implies receiving multicast traffic\n"
    "destined for all UDP ports";

auto parseGroupPort(
        std::string const& gp) -> std::tuple<IPv4Address, uint16_t, bool> {
    auto cpos = gp.find(':');

    if (cpos != std::string::npos) {
        // Group port version
        auto sv = std::string_view(gp);
        auto grpsv = sv.substr(0, cpos);
        auto grp = parseIPv4Address(grpsv);
        if (not grp)
            raise<CommandLineError>("invalid multicast group '{}'", grpsv);
        auto portsv = sv.substr(cpos+1);
        auto dport = parseDecimalUInt16(portsv);
        if (not dport)
            raise<CommandLineError>("invalid destination UDP port '{}'", portsv);
        if (dport == 0u)
            raise<CommandLineError>("destination UDP port may not be 0");

        return std::make_tuple(*grp, *dport, false);
    }

    // Group, wildcard port
    auto grp = parseIPv4Address(gp);
    if (not grp)
        raise<CommandLineError>("invalid multicast group/port '{}'", gp);
    return std::make_tuple(*grp, 0, true);
}

auto parseSourceOfG(
        std::vector<std::string> const& sofg) -> IPv4Address {
    if (sofg.empty()) return IPv4Address{};

    auto const& ss = sofg[0];
    auto s = parseIPv4Address(ss);
    if (not s)
        raise<CommandLineError>("invalid source address '{}'", ss);

    if (s->isMcast())
        raise<CommandLineError>("source address may not be multicast ({})", *s);

    if (s->isDefault())
        raise<CommandLineError>("source address may not be default ({})", *s);

    if (s->isLocalBroadcast())
        raise<CommandLineError>("source address may not be broadcast ({})", *s);

    return *s;
}

auto parseTimeoutSecs(std::vector<std::string> const& ts) -> unsigned {
    if (ts.empty()) return 5;

    auto const& tsSpec = ts[0];
    auto rts = parseDecimalUInt32(tsSpec);
    if (not rts)
        raise<CommandLineError>("invalid timeout '{}'", tsSpec);

    auto timeoutSecs = *rts;
    if (timeoutSecs < 1 or timeoutSecs > 600)
        raise<CommandLineError>(
                "invalid timeout of {} seconds, valid range is 0-600", timeoutSecs);

    return timeoutSecs;
}

auto parseTTL(std::vector<std::string> const& ttls, bool sender) -> uint32_t {
    if (not sender) {
        if (not ttls.empty())
            raise<CommandLineError>(
                    "the option --ttl may only be specified with "
                    "the option -s|--sender");

        return 0u;
    }

    if (ttls.empty()) return 255u;

    auto const& ttlSpec = ttls[0];
    auto rTTL = parseDecimalUInt32(ttlSpec);
    if (not rTTL)
        raise<CommandLineError>("invalid TTL '{}'", ttlSpec);

    auto ttl = *rTTL;
    if (ttl < 1 or ttl > 255)
        raise<CommandLineError>("invalid TTL value {}, valid range is 1-255", ttl);

    return ttl;
}

auto parseCount(std::vector<std::string> const& counts) -> unsigned {
    if (counts.empty()) return 0;

    auto const& countSpec = counts[0];
    auto rCount = parseDecimalUInt32(countSpec);
    if (not rCount)
        raise<CommandLineError>("invalid packet count '{}'", countSpec);

    return *rCount;
}


} // anon.namespace

Config Config::fromArgs(int argc, char** argv) {
    auto args = GetOptLong::with(header)
            .optional(
                    OID(Interface), 'i', "interface", "Interface",
                    "The host interface on which to receive/send multicast. The "
                    "interface can be specified by name, e.g. eth0, or by its "
                    "IPv4 address.")
            .optional(
                    OID(SourceOfG), 'S', "source", "IPv4Address",
                    "Indicates that the subscription should be source specific. "
                    "This option implies the use of IGMPv3, which may or may not be "
                    "enabled on the host. If it's not enabled, the host will join (*,G) "
                    "as opposed to (S,G) and filtering by source will be performed by "
                    "the host")
            .optional(
                    OID(Timeout), 't', "timeout", "Seconds",
                    "The timeout in seconds, defaults to 5s. Valid values are "
                    "in range 1-600.")
            .flag(OID(ShowPayload), 'X', "hex-ascii",
                  "Show the payload of the received traffic using split Hex/ASCII "
                  "output similar to tcpdump -XX.")
            .flag(OID(Sender), 's', "sender",
                  "Send multicast traffic. If this flag is set, then the destination "
                  "port may not be omitted.")
            .optional(
                    OID(SetTTL), GetOptLong::LongOnly, "ttl", "TTL",
                    "Set the TTL of the sent traffic to the specified value. "
                    "Defaults to 255. Valid values are in range 1-255. "
                    "This option may only be specified with the flag -s|--sender.")
            .optional(
                    OID(Count), 'c', "count", "NoOfPkts",
                    "Specify the number of packets to receive or to send. By default "
                    "the receiver will keep receiving and the sender will keep sending "
                    "until interrupted. This option will cause the receiver or the "
                    "sender to stop after the specified number of packet was received "
                    "or sent.")
            .flag(OID(NoColors), GetOptLong::LongOnly, "no-colors",
                  "Do not use colored output")
            .flag(OID(ShowConfig), GetOptLong::LongOnly, "show-config",
                  "Show config and exit")
            .flag(OID(ShowVersion), 'v', "version",
                  "show version and exit")
            .args(argc, argv);

    if (args.flag(OID(ShowVersion))) {
        fmt::print("mclst\n{}", version());
        exit(0);
    }

    auto const& gp = args.positional();
    auto const& intf = args.values(OID(Interface));

    if (gp.empty())
        raise<CommandLineError>("no group and destination port specified");

    if (gp.size() > 1)
        raise<CommandLineError>("too many positional parameters");

    if (intf.empty())
        raise<CommandLineError>("interface is required");

    IPv4Address group;
    uint16_t dport;
    bool wildcard;
    std::tie(group, dport, wildcard) = parseGroupPort(gp[0]);

    auto rIntfTable = IntfTable::newTable();
    if (not rIntfTable)
        raise<CommandLineError>(
                "unable to query host's interfaces: {}", rIntfTable.error());

    auto intfTable = std::move(rIntfTable).value();

    auto const& intfName = intf[0];
    auto intfInfo = intfTable.byName(intfName);
    if (not intfInfo) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);
        fmt::format_to(bi, "unknown interface '{}'\n\n", intfName);
        fmt::format_to(bi, "available interfaces:\n");
        formatIntfTable(bi, intfTable, 0);
        throw CommandLineError{fmt::to_string(buf)};
    } else if (not intfInfo->ipv4addr) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);
        fmt::format_to(bi, "interface {} has no IPv4 address\n\n", intfName);
        fmt::format_to(bi, "available interfaces:\n");
        formatIntfTable(bi, intfTable, 0);
        throw CommandLineError{fmt::to_string(buf)};
    }
    auto intfAddr = intfInfo->ipv4addr.value();

    auto sourceAddr = parseSourceOfG(args.values(OID(SourceOfG)));
    auto timeoutSecs = parseTimeoutSecs(args.values(OID(Timeout)));
    bool showPayload = args.flag(OID(ShowPayload));
    auto count = parseCount(args.values(OID(Count)));

    bool sender = args.flag(OID(Sender));
    auto ttl = parseTTL(args.values(OID(SetTTL)), sender);
    if (sender and wildcard)
        raise<CommandLineError>(
                "the destination port must be specified with the option -s|--sender");

    bool noColors = args.flag(OID(NoColors));
    if (not isatty(fileno(stdout)) or not isatty(fileno(stderr)))
        noColors = true;

    bool showConfig = args.flag(OID(ShowConfig));

    return Config{
        group,
        dport,
        wildcard,
        intfName,
        intfAddr,
        sourceAddr,
        timeoutSecs,
        sender,
        ttl,
        count,
        showPayload,
        not noColors,
        std::move(intfTable),
        showConfig,
    };
}

void Config::show() const {
    auto& buf = getMemoryBuffer();
    auto bi = std::back_inserter(buf);

    if (not sender_) {
        fmt::format_to(bi, "Receive from (");
        if (source_.isDefault()) fmt::format_to(bi, "*, ");
        else fmt::format_to(bi, "{},", source_);
        if (not wildcard_) fmt::format_to(bi, "{}:{})", group_, dport_);
        else fmt::format_to(bi, "{}:*)", group_);
        if (count_ > 0)
            fmt::format_to(bi, ", {} packets only", count_);
        fmt::format_to(bi, "\nShow payload: {}", (showPayload_ ? "YES" : "NO"));
    } else {
        fmt::format_to(
                bi, "Send to {}:{}, 1pps, TTL {}",
                group_, dport_, ttl_);
        if (count_ > 0)
            fmt::format_to(bi, ", {} packets only", count_);
    }
    fmt::format_to(bi, "\n");
    fmt::format_to(bi, "Interface: {} ({})\n", intf_, intfAddr_);
    fmt::format_to(bi, "Colors: {}\n", colors_ ? "YES" : "NO");
    fmt::format_to(bi, "\nHost interfaces:\n\n");
    formatIntfTable(bi, intfTable_, 2);
    buf.push_back(static_cast<char>(0));

    std::fputs(buf.data(), stdout);
}

} // namespace pimc
