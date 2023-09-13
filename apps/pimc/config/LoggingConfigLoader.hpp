#pragma once

#include <string_view>
#include <filesystem>

#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/FmtChrono.hpp"
#include "pimc/logging/LoggingLevel.hpp"
#include "pimc/yaml/BuilderBase.hpp"
#include "pimc/text/StringUtils.hpp"
#include "pimc/time/TimeUtils.hpp"
#include "pimc/time/Timestamp.hpp"

#include "ConfigUtils.hpp"
#include "LoggingConfig.hpp"

namespace fs = std::filesystem;

namespace pimc {
struct FNTS {};
} // namespace pimc

namespace fmt {

template <>
struct formatter<pimc::TimeValue<pimc::FNTS>>: formatter<string_view> {
    template <typename FormatContext>
    auto format(pimc::TimeValue<pimc::FNTS> const& ts, FormatContext& ctx) {
        auto tt = static_cast<time_t>(ts.value / 1'000'000'000ul);
        tm tms;

        return fmt::format_to(
                ctx.out(), "{:%Y%m%d-%H%M%S}",
                *localtime_r(&tt, &tms));
    }
};


} // namespace fmt

namespace pimc {

class LoggingConfigLoader final: BuilderBase {
    using LoggingCfgResult = Result<yaml::MappingContext, yaml::ErrorContext>;

    inline static const std::string_view LevelNone{"None"};
    inline static const std::string_view LevelCritical{"Critical"};
    inline static const std::string_view LevelError{"Error"};
    inline static const std::string_view LevelWarning{"Warning"};
    inline static const std::string_view LevelInfo{"Info"};
    inline static const std::string_view LevelDebug{"Debug"};

public:
    explicit LoggingConfigLoader(std::vector<yaml::ErrorContext>& errors)
    : BuilderBase{errors}
    , level_{Level::Info}
    , prefix_{"pimc"} {}

    void loadLoggingConfig(Optional<yaml::ValueContext> const& ovCtx) {
        if (ovCtx) {
            auto rLoggingCfg = chk(ovCtx->getMapping("Logging Config"));

            if (rLoggingCfg) {
                getLevel(rLoggingCfg);
                getDir(rLoggingCfg);
                chkExtraneous(rLoggingCfg.value());
            }
        }
    }

    LoggingConfig build() {
        std::optional<std::string> logFileName;
        if (dir_) {
            fs::path dp{dir_.value()};
            logFileName = (
                    dp /
                    fmt::format(
                            "{}-{}.log",
                            prefix_,
                            TimeValue<FNTS>{.value = pimc::gethostnanos()})).native();
        }
        return LoggingConfig{
            level_,
            std::move(logFileName)
        };
    }

private:
    void getLevel(LoggingCfgResult const& rLoggingCfg) {
        auto oLvl = rLoggingCfg->optional("level");
        if (oLvl) {
            auto rLvl = chk(oLvl->getScalar("Logging Level Name"));

            if (rLvl) {
                auto const& ln = rLvl->value();

                if (ciAsciiStrEq(ln, LevelNone)) {
                    level_ = Level::None;
                } else if (ciAsciiStrEq(ln, LevelCritical)) {
                    level_ = Level::Critical;
                } else if (ciAsciiStrEq(ln, LevelError)) {
                    level_ = Level::Error;
                } else if (ciAsciiStrEq(ln, LevelWarning)) {
                    level_ = Level::Warning;
                } else if (ciAsciiStrEq(ln, LevelInfo)) {
                    level_ = Level::Info;
                } else if (ciAsciiStrEq(ln, LevelDebug)) {
                    level_ = Level::Debug;
                } else {
                    consume(rLvl->error("invalid logging level name '{}'", ln));
                }
            }
        }
    }

    void getDir(LoggingCfgResult const& rLoggingCfg) {
        auto oDir = rLoggingCfg->optional("dir");
        if (oDir) {
            auto rDir = chk(oDir->getScalar("Logging Directory"));

            if (rDir) {
                auto const& dn = rDir->value();
                fs::path pdn{dn};

                if (not fs::exists(pdn)) {
                    consume(rDir->error("directory '{}' does not exist", dn));
                } else if (not fs::is_directory(pdn)) {
                    consume(rDir->error("'{}' is not a directory", dn));
                } else {
                    dir_ = dn;
                }
            }
        }
    }
private:
    Level level_;
    std::optional<std::string> dir_;
    std::string prefix_;
};

inline auto loadLoggingConfig(Optional<yaml::ValueContext> const& ovCtx)
-> Result<LoggingConfig, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    LoggingConfigLoader loggingCfgLdr{errors};
    loggingCfgLdr.loadLoggingConfig(ovCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return loggingCfgLdr.build();
}

} // namespace pimc
