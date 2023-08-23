#pragma once

#include <ctime>
#include <vector>
#include <string>
#include <string_view>

#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/FmtChrono.hpp"

#include "pimc/text/NumberLengths.hpp"
#include "pimc/formatters/MemoryBuffer.hpp"
#include "pimc/formatters/HexAsciiBlock.hpp"
#include "pimc/formatters/NanosText.hpp"
#include "pimc/text/SCLine.hpp"
#include "pimc/formatters/IPv4Formatters.hpp"
#include "pimc/unix/TerminalColors.hpp"

#include "Config.hpp"
#include "PacketInfo.hpp"
#include "RxStats.hpp"

namespace pimc {

struct Interface {
    unsigned value;
    IntfTable const& intfTable;
};

struct TTL {
    int value;
};

struct Timestamp {
    uint64_t value;
};

struct BeaconTime {
    uint64_t value;
};

struct Duration {
    uint64_t value;
};

struct SourceAndPort {
    IPv4Address source;
    uint16_t sport;
};

} // namespace pimc

namespace fmt {

template <>
struct formatter<pimc::Interface>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::Interface const& intf, FormatContext& ctx) {
        if (intf.value != 0) {
            auto rIntfInfo = intf.intfTable.byIndex(intf.value);

            if (rIntfInfo) {
                auto const& intfName =(*rIntfInfo).name;
                return fmt::format_to(ctx.out(), "{} (#{})", intfName, intf.value);
            } else {
                return fmt::format_to(ctx.out(), "*unknown intf* (#{})", intf.value);
            }
        }

        return fmt::format_to(ctx.out(), "N/A");
    }
};

template <>
struct formatter<pimc::TTL>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::TTL const& ttl, FormatContext& ctx) {
        if (ttl.value != -1) {
            if (ttl.value >= 0 and ttl.value <= 255)
                return fmt::format_to(ctx.out(), "{}", ttl.value);
            // This shouldn't happen, but just in case
            return fmt::format_to(ctx.out(), "[Err]");
        }
        return fmt::format_to(ctx.out(), "N/A");
    }
};

template <>
struct formatter<pimc::Timestamp>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::Timestamp const& ts, FormatContext& ctx) {
        auto tt = static_cast<time_t>(ts.value / 1'000'000'000ul);
        uint64_t nanos = ts.value / 1'000'000'000ul;
        pimc::NanosText nt;
        char const* nstext;
        uint64_t carry;
        std::tie(nstext, carry) = nt.prc(nanos, 6);
        tt += static_cast<long>(carry);
        tm tms;

        return fmt::format_to(
                ctx.out(), "{:%H:%M:%S}.{:<6}",
                *localtime_r(&tt, &tms), nstext);
    }
};

template <>
struct formatter<pimc::BeaconTime>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::BeaconTime const& ts, FormatContext& ctx) {
        auto tt = static_cast<time_t>(ts.value / 1'000'000'000ul);
        uint64_t nanos = ts.value / 1'000'000'000ul;
        pimc::NanosText nt;
        char const* nstext;
        uint64_t carry;
        std::tie(nstext, carry) = nt.prc(nanos, 9);
        tt += static_cast<long>(carry);
        tm tms;

        return fmt::format_to(
                ctx.out(), "{:%Y-%m-%d %H:%M:%S}.{}", *localtime_r(&tt, &tms), nstext);
    }
};

template <>
struct formatter<pimc::Duration>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::Duration const& d, FormatContext& ctx) {
        uint64_t nanos = d.value % 1'000'000'000ul;
        uint64_t secs = d.value / 1'000'000'000ul;
        pimc::NanosText nt;
        char const* nstext;
        uint64_t carry;
        std::tie(nstext, carry) = nt.prc(nanos, 6);
        return fmt::format_to(ctx.out(), "{}.{}", secs, nstext);
    }
};

template <>
struct formatter<pimc::SourceAndPort>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::SourceAndPort const& sp, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}:{}", sp.source, sp.sport);
    }
};

} // namespace fmt

namespace pimc {

class OutputHandler {
public:
    constexpr explicit OutputHandler(Config const& cfg): cfg_{cfg} {}

    template <typename ... Ts>
    void warning(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RED_BRIGHT);

        fmt::format_to(bi, "warning: ");
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RESET);

        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stderr);
    }

    template <typename ... Ts>
    void warningTs(
            uint64_t ts, fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RED_BRIGHT);

        fmt::format_to(bi, "{}", Timestamp{.value = ts});
        fmt::format_to(bi, "warning: ");
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RESET);

        buf.push_back('\n');
        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stdout);
    }

    void showTimeout(uint64_t ts) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_WHITE_BRIGHT);

        fmt::format_to(bi, "{} timeout", Timestamp{.value = ts});

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RESET);

        buf.push_back('\n');
        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stdout);
    }

    void showReceivedPacket(PacketInfo const& pktInfo) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_YELLOW_BRIGHT);

        fmt::format_to(
                bi, "{} {}, {}:{}->{}:{}, TTL {}, UDP size {}",
                Timestamp{.value = pktInfo.timestamp},
                Interface{.value = pktInfo.ifIndex, .intfTable = cfg_.intfTable()},
                pktInfo.source, pktInfo.sport, cfg_.group(), pktInfo.dport,
                TTL{.value = pktInfo.ttl}, pktInfo.payloadSize);
        if (pktInfo.mclstBeacon) {
            fmt::format_to(bi, "\n");
            if (cfg_.colors())
                fmt::format_to(bi, TERM_COLOR_BLUE_BRIGHT);
            fmt::format_to(
                    bi, "{:<15} mclst pkt #{}, {}, delta {}ns, ",
                    ' ',
                    pktInfo.remoteSeq,
                    BeaconTime{.value = pktInfo.remoteTimestamp},
                    pktInfo.timestamp - pktInfo.remoteTimestamp);
            fmt::format_to_n(bi, pktInfo.remoteMsgLen, "{}", pktInfo.remoteMsg);
        }

        if (cfg_.showPayload()) {
            fmt::format_to(bi, "\n");
            if (cfg_.colors())
                fmt::format_to(bi, TERM_COLOR_YELLOW);
            formatHexAscii(bi, pktInfo.payload, pktInfo.payloadSize);
        }

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RESET);

        buf.push_back('\n');
        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stdout);
    }

    void showSentPacket(uint64_t ts, uint64_t seq) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_GREEN_BRIGHT);

        fmt::format_to(
                bi, "{} sent packet to {}:{}, seq #{}",
                Timestamp{.value = ts}, cfg_.group(), cfg_.dport(), seq);

        if (cfg_.colors())
            fmt::format_to(bi, TERM_COLOR_RESET);

        buf.push_back('\n');
        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stdout);
    }

    void showRxStats(RxStats const& rxStats, bool stopped) {
        auto& buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (stopped)
            fmt::format_to(bi, "\n");
        fmt::format_to(bi, "\n");

        if (not rxStats) {
            fmt::format_to(bi, "No traffic received for {}", cfg_.group());
            if (cfg_.wildcard()) fmt::format_to(bi, ":*");
            else fmt::format_to(bi, ":{}", cfg_.dport());
            fmt::format_to(bi, " in {} sec", Duration{.value = rxStats.durationNanos()});
            buf.push_back('\n');
            buf.push_back(static_cast<char>(0));
            fputs(buf.data(), stdout);
            return;
        }

        std::size_t sourceFldLen = strlen(CapSource);
        std::size_t dportFldLen = strlen(CapDPort);
        std::size_t pktsFldLen = strlen(CapPkts);
        std::size_t bytesFldLen = strlen(CapBytes);
        std::size_t apsFldLen = strlen(CapAPS);
        std::size_t rateFldLen = strlen(CapRate);

        std::vector<FlowStatsView> fsvs;
        fsvs.reserve(rxStats.size());
        rxStats.forEach(
                [&fsvs, duration=rxStats.durationNanos(),
                 &sourceFldLen, &dportFldLen, &pktsFldLen,
                 &bytesFldLen, &apsFldLen, &rateFldLen]
                (auto source, auto sport, auto dport, auto const& fs) {
                    fsvs.emplace_back(source, sport, dport, fs, duration);
                    auto const& fsv = fsvs.back();
                    sourceFldLen = std::max(sourceFldLen, fsv.spSize());
                    dportFldLen = std::max(dportFldLen, fsv.dportSize());
                    pktsFldLen = std::max(pktsFldLen, fsv.packetsSize());
                    bytesFldLen = std::max(bytesFldLen, fsv.bytesSize());
                    apsFldLen = std::max(apsFldLen, fsv.apsSize());
                    rateFldLen = std::max(rateFldLen, fsv.rateSize());
                });

        auto fs = fmt::format(
                "{{:<{}}} {{:<{}}} {{:>{}}} {{:>{}}} {{:>{}}} {{:>{}}}\n",
                sourceFldLen, dportFldLen, pktsFldLen, bytesFldLen,
                apsFldLen, rateFldLen);

        SCLine<'='> sep{std::max({
            sourceFldLen, dportFldLen, pktsFldLen, bytesFldLen, apsFldLen, rateFldLen})};

        fmt::format_to(bi, "Traffic received for {}", cfg_.group());
        if (cfg_.wildcard()) fmt::format_to(bi, ":*");
        else fmt::format_to(bi, ":{}", cfg_.dport());
        fmt::format_to(bi, " in {} sec\n\n", Duration{.value = rxStats.durationNanos()});

        fmt::format_to(
                bi, fmt::runtime(fs),
                CapSource, CapDPort, CapPkts, CapBytes, CapAPS, CapRate);
        fmt::format_to(
                bi, fmt::runtime(fs),
                sep(sourceFldLen), sep(dportFldLen), sep(pktsFldLen),
                sep(bytesFldLen), sep(apsFldLen), sep(rateFldLen));
        for (auto const& fsv: fsvs)
            fmt::format_to(
                    bi, fmt::runtime(fs),
                    fsv.sp(), fsv.dport(), fsv.packets(),
                    fsv.bytes(), fsv.aps(), fsv.rate());
        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stdout);
    }

    void showTxStats(uint64_t count, bool stopped) {
        auto &buf = getMemoryBuffer();
        auto bi = std::back_inserter(buf);

        if (stopped)
            fmt::format_to(bi, "\n");

        fmt::format_to(bi, "Sent {} packets", count);

        buf.push_back('\n');
        buf.push_back(static_cast<char>(0));
        fputs(buf.data(), stdout);
    }

private:
    inline static char const* const CapSource{"Source"};
    inline static char const* const CapDPort{"DPort"};
    inline static char const* const CapPkts{"Pkts"};
    inline static char const* const CapBytes{"Bytes"};
    inline static char const* const CapAPS{"APS"};
    inline static char const* const CapRate{"Rate"};

    class FlowStatsView final {
    public:
        FlowStatsView(
                IPv4Address source, uint16_t sport, uint16_t dport,
                FlowStats const& fs, uint64_t duration)
                : source_{source}
                , sport_{sport}
                , dport_{dport}
                , packets_{fs.pkts()}
                , bytes_{fs.bytes()}
                , aps_{fmt::format("{:.2f}", fs.aps())} {
            double rate =
                    static_cast<double>(fs.bytes() << 3u)
                    * 1'000'000'000 / static_cast<double>(duration);
            if (rate < 1000)
                rate_ = fmt::format("{:.2f}bps", rate);
            else if (rate < 1'000'000)
                rate_ = fmt::format("{:.2f}Kbps", rate/1'000);
            else if (rate < 1'000'000'000)
                rate_ = fmt::format("{:.2f}Mbps", rate/1'000'000);
            else rate_ = fmt::format("{:.2f}Gbps", rate/1'000'000'000);
        }

        [[nodiscard]]
        size_t spSize() const { return source_.charlen() + 1 + decimalUIntLen(sport_); }

        [[nodiscard]]
        SourceAndPort sp() const { return {.source = source_, .sport = sport_}; }

        [[nodiscard]]
        size_t dportSize() const { return decimalUIntLen(dport_); }

        [[nodiscard]]
        uint16_t dport() const { return dport_; }

        [[nodiscard]]
        size_t packetsSize() const { return decimalUIntLen(packets_); }

        [[nodiscard]]
        uint64_t packets() const { return packets_; }

        [[nodiscard]]
        size_t bytesSize() const { return decimalUIntLen(bytes_); }

        [[nodiscard]]
        uint64_t bytes() const { return bytes_; }

        [[nodiscard]]
        size_t apsSize() const { return aps_.size(); }

        [[nodiscard]]
        std::string const& aps() const { return aps_; }

        [[nodiscard]]
        size_t rateSize() const { return rate_.size(); }

        [[nodiscard]]
        std::string const& rate() const { return rate_; }

    private:
        IPv4Address source_;
        uint16_t sport_;
        uint16_t dport_;
        uint64_t packets_;
        uint64_t bytes_;
        std::string aps_;
        std::string rate_;
    };

private:
    Config const& cfg_;
};

} // namespace pimc
