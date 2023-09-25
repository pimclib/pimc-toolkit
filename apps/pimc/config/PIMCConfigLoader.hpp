#pragma once

#include "pimc/core/Optional.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/net/IP.hpp"
#include "pimc/net/IntfTable.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/yaml/Structured.hpp"

#include "ConfigUtils.hpp"
#include "LoggingConfig.hpp"
#include "LoggingConfigLoader.hpp"
#include "PIMCConfig.hpp"
#include "PIMSMConfigLoader.hpp"
#include "JPConfigLoader.hpp"
#include "pimsm/Update.hpp"
#include "pimsm/Pack.hpp"
#include "pimsm/PackSanityCheck.hpp"
#include "pimsm/InversePack.hpp"
#include "pimsm/InversePackSanityCheck.hpp"

namespace pimc {

template <IPVersion V>
class PIMCConfigLoader final: BuilderBase {
public:
    using IPAddress = typename IP<V>::Address;

    explicit PIMCConfigLoader(std::vector<yaml::ErrorContext>& errors)
            : BuilderBase{errors} {}

    void loadPIMCConfig(
            yaml::ValueContext const& vCtx, IntfTable const& intfTable) {
        auto rCfg = chk(vCtx.getMapping());

        if (rCfg) {
            auto rLoggingCfg = chkErrors(
                    loadLoggingConfig(rCfg->optional("logging")));
            if (rLoggingCfg)
                loggingConfig_ = std::move(rLoggingCfg).value();

            auto rPIMSMCfgCtx = chk(rCfg->required("pim"));

            if (rPIMSMCfgCtx) {
                auto rPIMSMCfg = chkErrors(
                        loadPIMSMConfig<V>(rPIMSMCfgCtx.value(), intfTable));

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

        auto updates = pack(jpConfig_.value());
        auto r = verifyUpdates(jpConfig_.value(), updates);

        if (not r)
            throw std::logic_error{r.error()};

        auto inverseUpdates = inversePack(jpConfig_.value());
        auto ri = verifyInverseUpdates(jpConfig_.value(), inverseUpdates);

        if (not ri)
            throw std::logic_error{ri.error()};

        return PIMCConfig{
            std::move(loggingConfig_).value(),
            std::move(pimsmConfig_).value(),
            std::move(jpConfig_).value(),
            std::move(updates),
            std::move(inverseUpdates),
        };
    }

    Optional<LoggingConfig> loggingConfig_;
    Optional<PIMSMConfig<V>> pimsmConfig_;
    Optional<JPConfig<V>> jpConfig_;
};

template <IPVersion V>
auto loadPIMCConfig(yaml::ValueContext const& cfgfgCtx, IntfTable const& intfTable)
-> Result<PIMCConfig<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PIMCConfigLoader<V> pimcConfigLoader{errors};
    pimcConfigLoader.loadPIMCConfig(cfgfgCtx, intfTable);
    if (not errors.empty()) return fail(std::move(errors));
    return pimcConfigLoader.build();
}

} // namespace pimc
