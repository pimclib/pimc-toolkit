#include "pimc/formatters/Fmt.hpp"

#include "pimc/net/IPv4Address.hpp"
#include "PIMSMConfig.hpp"
#include "PIMSMConfigLoader.hpp"
#include "pimc/yaml/BuilderBase.hpp"
#include "ConfigUtils.hpp"

namespace pimc {

namespace {

struct PIMSMConfigLoader final: BuilderBase {
    explicit PIMSMConfigLoader(std::vector<yaml::ErrorContext>& errors)
    : BuilderBase{errors} {}

    void loadPIMSMConfig(yaml::ValueContext const& vCtx) {
        auto rPIMSMCfg = chk(vCtx.getMapping("PIM-SM config"));

        if (rPIMSMCfg) {
            auto rNei = chk(rPIMSMCfg->required("neighbor")
                    .flatMap(yaml::scalar("neighbor IPv4 address")));

            if (rNei) {
                auto rNeiA = chk(
                        ucAddr(rNei->value(), UCAddrType::Neighbor)
                        .mapError([&rNei] (auto msg) {
                            return rNei->error(std::move(msg)); }));

                if (rNeiA) {
                    neighbor_ = rNeiA.value();
                }
            }
        }
    }

    [[nodiscard]]
    PIMSMConfig build() const {
        return PIMSMConfig{neighbor_};
    }

    net::IPv4Address neighbor_;
};

} // anon.namespace

auto loadPIMSMConfig(yaml::ValueContext const& pimsmCfgCtx)
-> Result<PIMSMConfig, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PIMSMConfigLoader pimsmCfgLdr{errors};
    pimsmCfgLdr.loadPIMSMConfig(pimsmCfgCtx);
    if (not errors.empty()) return fail(errors);
    return pimsmCfgLdr.build();
}

} // namespace pimc
