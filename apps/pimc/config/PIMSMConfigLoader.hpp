#pragma once

#include "pimc/formatters/Fmt.hpp"
#include "pimc/net/IP.hpp"
#include "pimc/yaml/BuilderBase.hpp"

#include "PIMSMConfig.hpp"
#include "ConfigUtils.hpp"

namespace pimc::pimsm_config {

template <IPVersion V>
class PIMSMConfigLoader final: BuilderBase {
public:
    using IPAddress = typename IP<V>::Address;

    using BuilderBase::BuilderBase;

    void loadPIMSMConfig(yaml::ValueContext const& vCtx) {
        auto rPIMSMCfg = chk(vCtx.getMapping("PIM-SM config"));

        if (rPIMSMCfg) {
            auto rNei = chk(rPIMSMCfg->required("neighbor")
                    .flatMap(yaml::scalar(fmt::format("neighbor {} address", V{}))));

            if (rNei) {
                auto rNeiA = chk(
                        ucAddr<V>(rNei->value(), UCAddrType::Neighbor)
                        .mapError([&rNei] (auto msg) {
                            return rNei->error(std::move(msg)); }));

                if (rNeiA) {
                    neighbor_ = rNeiA.value();
                }
            }

            auto rIntf = chk(rPIMSMCfg->required("interface")
                    .flatMap(yaml::scalar(fmt::format("PIM SM {} interface", V{}))));

            if (rIntf) {

            }
        }
    }

    [[nodiscard]]
    PIMSMConfig<V> build() const {
        return PIMSMConfig<V>{neighbor_, intfAddr_, intfName_};
    }

private:
    IPAddress neighbor_;
    IPAddress intfAddr_;
    std::string intfName_;
};

template <IPVersion V>
auto loadPIMSMConfig(yaml::ValueContext const& pimsmCfgCtx)
-> Result<PIMSMConfig<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PIMSMConfigLoader<V> pimsmCfgLdr{errors};
    pimsmCfgLdr.loadPIMSMConfig(pimsmCfgCtx);
    if (not errors.empty()) return fail(errors);
    return pimsmCfgLdr.build();
}

} // namespace pimc::pimsm_config
