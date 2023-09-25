#include "Logging.hpp"
#include "ConsoleLogger.hpp"
#include "FileLogger.hpp"

namespace pimc {

Logger Logger::logger(LoggingConfig const& loggingConfig) {
    int maxLevel = static_cast<int>(loggingConfig.level());
    if (loggingConfig.logFileName()) {
        auto const& lfn = loggingConfig.logFileName().value();
        return Logger{maxLevel, std::make_unique<FileLogger>(lfn)};
    }

    return Logger{maxLevel, std::make_unique<ConsoleLogger>()};
}

} // namespace pimc
