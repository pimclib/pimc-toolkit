#pragma once

#include <string>

#include "Logging.hpp"

namespace pimc {

class ConsoleLogger final: public ILogger {
public:
    ConsoleLogger() = default;

    ConsoleLogger(ConsoleLogger const&) = delete;
    ConsoleLogger(ConsoleLogger&&) = delete;
    ConsoleLogger& operator= (ConsoleLogger const&) = delete;
    ConsoleLogger& operator= (ConsoleLogger&&) = delete;

    void log(uint64_t ts, Level level, char const* message, size_t sz) final;

    ~ConsoleLogger() noexcept final = default;
private:
    fmt::memory_buffer buf_;
};

} // namespace pimc
