#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

#include "pimc/logging/LoggingLevel.hpp"
#include "pimc/time/TimeUtils.hpp"
#include "pimc/formatters/Fmt.hpp"

#include "config/LoggingConfig.hpp"

namespace pimc {

struct ILogger {
    virtual void log(uint64_t ts, Level level, char const* message, size_t sz) = 0;

    virtual ~ILogger() = default;
};

class Logger final {
public:
    static Logger logger(LoggingConfig const& loggingConfig);

    template <typename ... Ts>
    void log(uint64_t ts, Level level,
             fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        if (static_cast<int>(level) > maxLevel_) return;
        buf_.clear();
        auto bi = std::back_inserter(buf_);
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        buf_.push_back('\n');
        log_->log(ts, level, buf_.data(), buf_.size());
    }

    template <typename ... Ts>
    void log(Level level,
             fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(gethostnanos(), level, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void critical(uint64_t ts,
                  fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(ts, Level::Critical, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void critical(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(gethostnanos(), Level::Critical, fs, std::forward<Ts>(args)...);
    }


    template <typename ... Ts>
    void error(uint64_t ts,
               fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(ts, Level::Error, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void error(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(gethostnanos(), Level::Error, fs, std::forward<Ts>(args)...);
    }


    template <typename ... Ts>
    void warning(uint64_t ts,
                 fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(ts, Level::Warning, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void warning(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(gethostnanos(), Level::Warning, fs, std::forward<Ts>(args)...);
    }


    template <typename ... Ts>
    void info(uint64_t ts,
              fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(ts, Level::Info, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void info(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(gethostnanos(), Level::Info, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void debug(uint64_t ts,
             fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(ts, Level::Debug, fs, std::forward<Ts>(args)...);
    }

    template <typename ... Ts>
    void debug(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        log(gethostnanos(), Level::Debug, fs, std::forward<Ts>(args)...);
    }

    [[nodiscard]]
    bool enabled(Level level) const {
        return static_cast<int>(level) <= maxLevel_;
    }

private:
    explicit Logger(int maxLevel, std::unique_ptr<ILogger> log)
    : maxLevel_{maxLevel}, log_{std::move(log)} {}

private:
    int maxLevel_;
    std::unique_ptr<ILogger> log_;
    fmt::memory_buffer buf_;
};

} // namespace pimc
