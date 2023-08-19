#pragma once

#include "pimc/core/Optional.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/net/IP.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/yaml/Structured.hpp"

#include "ConfigUtils.hpp"
#include "PIMCConfig.hpp"
#include "PIMSMConfigLoader.hpp"
#include "JPConfigLoader.hpp"

namespace pimc::pimsm_config {

template <IPVersion V>
class PIMCConfigLoader final: BuilderBase {
public:
    using IPAddress = typename IP<V>::Address;

    explicit PIMCConfigLoader(std::vector<yaml::ErrorContext>& errors)
            : BuilderBase{errors} {}

    void loadPIMCConfig(yaml::ValueContext const& vCtx) {
        auto rCfg = chk(vCtx.getMapping());

        if (rCfg) {
            auto rPIMSMCfgCtx = chk(rCfg->required("pim"));

            if (rPIMSMCfgCtx) {
                auto rPIMSMCfg = chkErrors(loadPIMSMConfig<V>(rPIMSMCfgCtx.value()));

                if (rPIMSMCfg)
                    pimsmConfig_ = std::move(rPIMSMCfg).value();
            }

            auto rJPCfgCtx = chk(rCfg->required("multicast"));

            if (rJPCfgCtx) {
                auto rJPCfg = chkErrors(loadJPConfig<V>(rJPCfgCtx.value()));

                if (rJPCfg)
                    jpConfig_ = std::move(rJPCfg).value();
            }

            chkExtraneous(rCfg.value());
        }
    }

    [[nodiscard]]
    PIMCConfig<V> build() {
        if (not pimsmConfig_ or not jpConfig_)
            raise<std::logic_error>("PIM SM config or J/P config is not loaded");

        return PIMCConfig{
            std::move(pimsmConfig_).value(),
            std::move(jpConfig_).value()
        };
    }

    Optional<PIMSMConfig<V>> pimsmConfig_;
    Optional<JPConfig<V>> jpConfig_;
};

template <IPVersion V>
auto loadPIMCConfig(yaml::ValueContext const& cfgfgCtx)
-> Result<PIMCConfig<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PIMCConfigLoader<V> pimcConfigLoader{errors};
    pimcConfigLoader.loadPIMCConfig(cfgfgCtx);
    if (not errors.empty()) return fail(std::move(errors));
    return pimcConfigLoader.build();
}

} // namespace pimc::pimsm_config