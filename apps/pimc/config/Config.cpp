#include "pimc/formatters/Fmt.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/yaml/LoadAll.hpp"
#include "pimc/yaml/Structured.hpp"
#include "pimc/yaml/ErrorHandler.hpp"
#include "pimc/unix/GetOptLong.hpp"
#include "version.hpp"

#include "Config.hpp"
#include "PIMCConfigLoader.hpp"
#include "Formatters.hpp"

#define OID(id) static_cast<uint32_t>(Options::id)

namespace pimc::pimsm_config {

namespace {
enum class Options: uint32_t {
    ShowConfig = 1,
    ShowVersion = 2,
};

char const* header = "[Options] pimc-config.yml";

} // anon.namespace

PIMCConfig<IPv4> loadIPv4Config(int argc, char** argv) {
    auto args = GetOptLong::with(header)
            .flag(OID(ShowConfig), GetOptLong::LongOnly, "show-config",
                  "Show config and exit")
            .flag(OID(ShowVersion), 'v', "version",
                  "show version and exit")
            .args(argc, argv);

    if (args.flag(OID(ShowVersion))) {
        fmt::print("pimc\n{}", version());
        exit(0);
    }

    auto const& cfgfn = args.positional();

    if (cfgfn.empty())
        raise<CommandLineError>("no pimc YAML config file specified");

    if (cfgfn.size() > 1)
        raise<CommandLineError>("too many positional parameters");

    auto const& yamlfn = cfgfn[0];
    auto rCfgCnt = yaml::loadAll(yamlfn);

    if (not rCfgCnt)
        throw std::runtime_error{rCfgCnt.error()};

    auto yamlDocs = std::move(rCfgCnt).value();
    if (yamlDocs.size() != 1) {
        raise<std::runtime_error>(
                "pimc YAML configuration must contain exactly 1 document, not {}",
                yamlDocs.size());
    }

    auto rCfg = loadPIMCConfig<IPv4>(yaml::ValueContext::root(yamlDocs[0]));
    if (not rCfg) {
        yaml::StderrErrorHandler ec{yamlfn.c_str()};
        for (auto const& eCtx: rCfg.error())
            ec.showError(eCtx);
        raise<std::runtime_error>(
                "invalid YAML configuration file '{}'", yamlfn);
    }

    auto pimcCfg = std::move(rCfg).value();

    if (args.flag(OID(ShowConfig))) {
        fmt::print("{}", pimcCfg.pimsmConfig());
        fmt::print("{}", pimcCfg.jpConfig());
        exit(0);
    }

    return pimcCfg;
}

} // namespace pimc::pimsm_config
