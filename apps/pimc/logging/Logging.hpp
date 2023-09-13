#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

#include "pimc/logging/LoggingLevel.hpp"
#include "pimc/formatters/Fmt.hpp"

namespace pimc {

struct ILogger {
    virtual void log(uint64_t ts, Level level, char const* message, size_t sz) = 0;

    virtual ~ILogger() = default;
};

class Logger final {
public:

    template <typename ... Ts>
    void log(uint64_t ts, Level level,
             fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        buf_.clear();
        auto bi = std::back_inserter(buf_);
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        buf_.push_back('\n');
        log_->log(ts, level, buf_.data(), buf_.size());
    }

private:
    std::unique_ptr<ILogger> log_;
    fmt::memory_buffer buf_;
};

} // namespace pimc
