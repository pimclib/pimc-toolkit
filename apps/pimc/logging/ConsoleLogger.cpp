#include "pimc/time/Timestamp.hpp"
#include "pimc/formatters/FmtChrono.hpp"
#include "pimc/formatters/NanosText.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"
#include "pimc/formatters/LoggingLevelFormatter.hpp"

#include "ConsoleLogger.hpp"

namespace fmt {

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

} // namespace fmt

namespace pimc {

void ConsoleLogger::log(
        uint64_t ts, Level level, char const* message, size_t sz) {
    buf_.clear();
    auto bi = std::back_inserter(buf_);

    fmt::format_to(bi, "{} {}: ", Timestamp{.value = ts}, level);
    buf_.append(message, message + sz);
    buf_.push_back(static_cast<char>(0));
    fputs(buf_.data(), stdout);
}


} // namespace pimc
