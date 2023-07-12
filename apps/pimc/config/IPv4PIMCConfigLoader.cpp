#include "pimc/core/Optional.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/yaml/Structured.hpp"

#include "ConfigUtils.hpp"
#include "PIMCConfig.hpp"
#include "IPv4PIMCConfigLoader.hpp"
#include "IPv4PIMSMConfigLoader.hpp"
#include "IPv4JPConfigLoader.hpp"

namespace pimc {
namespace {

struct PIMCConfigLoader final: BuilderBase {
    explicit PIMCConfigLoader(std::vector<yaml::ErrorContext>& errors)
            : BuilderBase{errors} {}

    void loadPIMCConfig(yaml::ValueContext const& vCtx) {
        auto rCfg = chk(vCtx.getMapping());

        if (rCfg) {
            auto rPIMSMCfgCtx = chk(rCfg->required("pim"));

            if (rPIMSMCfgCtx) {
                auto rPIMSMCfg = chkErrors(loadIPv4PIMSMConfig(rPIMSMCfgCtx.value()));

                if (rPIMSMCfg)
                    pimsmConfig_ = std::move(rPIMSMCfg).value();
            }

            auto rJPCfgCtx = chk(rCfg->required("multicast"));

            if (rJPCfgCtx) {
                auto rJPCfg = chkErrors(loadIPv4JPConfig(rJPCfgCtx.value()));

                if (rJPCfg)
                    jpConfig_ = std::move(rJPCfg).value();
            }

            chkExtraneous(rCfg.value());
        }
    }

    [[nodiscard]]
    PIMCConfig<net::IPv4Address> build() {
        if (not pimsmConfig_ or not jpConfig_)
            raise<std::logic_error>("PIM SM config or J/P config is not loaded");

        return PIMCConfig{
            std::move(pimsmConfig_).value(),
            std::move(jpConfig_).value()
        };
    }

    Optional<PIMSMConfig<net::IPv4Address>> pimsmConfig_;
    Optional<JPConfig<net::IPv4Address>> jpConfig_;
};

} // anon.namespace

auto loadIPv4PIMCConfig(yaml::ValueContext const& cfgfgCtx)
-> Result<PIMCConfig<net::IPv4Address>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PIMCConfigLoader pimcConfigLoader{errors};
    pimcConfigLoader.loadPIMCConfig(cfgfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return pimcConfigLoader.build();
}

} // namespace pimc
