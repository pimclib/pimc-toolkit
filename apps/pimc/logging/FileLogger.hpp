#pragma once

#include <cstdio>
#include <string>

#include "Logging.hpp"

namespace pimc {

class FileLogger final: public ILogger {
public:
    explicit FileLogger(std::string const& logFileName);

    FileLogger(FileLogger const&) = delete;
    FileLogger(FileLogger&&) = delete;
    FileLogger& operator= (FileLogger const&) = delete;
    FileLogger& operator= (FileLogger&&) = delete;

    void log(uint64_t ts, Level level, char const* message, size_t sz) final;

    ~FileLogger() noexcept final;
private:
    FILE* fp_;
};

} // namespace pimc
