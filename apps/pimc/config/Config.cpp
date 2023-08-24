#include "pimc/formatters/Fmt.hpp"
#include "pimc/net/IntfTable.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/yaml/LoadAll.hpp"
#include "pimc/yaml/Structured.hpp"
#include "pimc/yaml/ErrorHandler.hpp"
#include "pimc/unix/GetOptLong.hpp"
#include "version.hpp"

#include "Config.hpp"
#include "PIMCConfigLoader.hpp"
#include "Formatters.hpp"
#include "pimsm/UpdateFormatter.hpp"

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

    auto rIntfTable = IntfTable::newTable();
    if (not rIntfTable) {
        raise<std::runtime_error>(
                "unable to get host interfaces: {}", rIntfTable.error());
    }

    auto rCfg = loadPIMCConfig<IPv4>(
            yaml::ValueContext::root(yamlDocs[0]), rIntfTable.value());
    if (not rCfg) {
        yaml::StderrErrorHandler ec{yamlfn.c_str()};
        for (auto const& eCtx: rCfg.error())
            ec.showError(eCtx);
        raise<std::runtime_error>(
                "invalid YAML configuration file '{}'", yamlfn);
    }

    auto pimcCfg = std::move(rCfg).value();

    if (args.flag(OID(ShowConfig))) {
        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(bi, "{}\n", pimcCfg.pimsmConfig());
        fmt::format_to(bi, "{}\n", pimcCfg.jpConfig());
        fmt::format_to(
                bi, "Will send {} update{}:\n\n",
                pimcCfg.updates().size(), plural(pimcCfg.updates()));
        unsigned n{1};
        for (auto const& update: pimcCfg.updates())
            fmt::format_to(
                    bi, "{}\n", std::tuple<unsigned, Update<IPv4> const&>(n, update));

        mb.push_back(static_cast<char>(0));
        fputs(mb.data(), stdout);
        exit(0);
    }

    return pimcCfg;
}

} // namespace pimc::pimsm_config
