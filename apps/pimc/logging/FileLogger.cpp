#include <cstdio>

#include <filesystem>

#include "pimc/system/SysError.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/time/Timestamp.hpp"
#include "pimc/formatters/FmtChrono.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"
#include "pimc/formatters/LoggingLevelFormatter.hpp"

#include "FileLogger.hpp"

namespace fs = std::filesystem;

namespace fmt {

template <>
struct formatter<pimc::Timestamp>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::Timestamp const& ts, FormatContext& ctx) {
        auto tt = static_cast<time_t>(ts.value / 1'000'000'000ul);
        uint64_t nanos = ts.value / 1'000'000'000ul;
        tm tms;
        auto ptms = localtime_r(&tt, &tms);

        return fmt::format_to(
                ctx.out(), "{:%Y-%m-%d %H:%M:%S}.{:<9} UTC{:%z}",
                *ptms, nanos, *ptms);
    }
};

} // namespace fmt

namespace pimc {

FileLogger::FileLogger(std::string const& logFileName) {
    fs::path lpn{logFileName};

    if (fs::exists(lpn)) {
        if (fs::is_directory(lpn))
            raise<std::runtime_error>(
                    "unable to create log file: '{}' is a directory",
                    logFileName);
        else
            raise<std::runtime_error>(
                    "unable to create log file '{}': file exists",
                    logFileName);
    }

    fp_ = fopen(logFileName.c_str(), "w");

    if (fp_ == nullptr)
        raise<std::runtime_error>(
                "unable to open log file '{}': {}",
                logFileName, SysError{});
}

void FileLogger::log(
        uint64_t ts, Level level, char const* message, size_t sz) {
    char buf[256];
    auto p = fmt::format_to(buf, "{} {}: ", Timestamp{.value = ts}, level);
    auto n = fwrite(buf, static_cast<size_t>(p - buf), 1, fp_);
    if (n != 1)
        raise<std::runtime_error>(
                "unable to write message to log file: {}", SysError{});
    n = fwrite(message, sz, 1, fp_);
    if (n != 1)
        raise<std::runtime_error>(
                "unable to write message to log file: {}", SysError{});
}

FileLogger::~FileLogger() noexcept {
    if (fp_ != nullptr) {
        bool retry;
        do {
            retry = false;
             if (fclose(fp_) == EOF and errno == EINTR)
                 retry = true;
        } while (retry);
    }
}

} // namespace pimc
