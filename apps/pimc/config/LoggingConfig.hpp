#pragma once

#include <optional>
#include <string>

#include "pimc/logging/LoggingLevel.hpp"

namespace pimc {

class LoggingConfig final {
public:
    LoggingConfig(
            Level level,
            std::optional<std::string> logFileName)
            : level_{level}
            , logFileName_{std::move(logFileName)} {}

    [[nodiscard]]
    Level level() const { return level_; }

    [[nodiscard]]
    std::optional<std::string> const& logFileName() const { return logFileName_; }

private:
    Level level_;
    std::optional<std::string> logFileName_;
    std::string prefix_;
};

} // namespace pimc
