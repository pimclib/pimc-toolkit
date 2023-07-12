#include "pimc/formatters/Fmt.hpp"

#include "pimc/net/IPv4Address.hpp"
#include "PIMSMConfig.hpp"
#include "IPv4PIMSMConfigLoader.hpp"
#include "pimc/yaml/BuilderBase.hpp"
#include "ConfigUtils.hpp"

namespace pimc {

namespace {

struct IPv4PIMSMConfigLoader final: BuilderBase {
    explicit IPv4PIMSMConfigLoader(std::vector<yaml::ErrorContext>& errors)
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
    PIMSMConfig<net::IPv4Address> build() const {
        return PIMSMConfig<net::IPv4Address>{neighbor_};
    }

    net::IPv4Address neighbor_;
};

} // anon.namespace

auto loadIPv4PIMSMConfig(yaml::ValueContext const& pimsmCfgCtx)
-> Result<PIMSMConfig<net::IPv4Address>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    IPv4PIMSMConfigLoader pimsmCfgLdr{errors};
    pimsmCfgLdr.loadPIMSMConfig(pimsmCfgCtx);
    if (not errors.empty()) return fail(errors);
    return pimsmCfgLdr.build();
}

} // namespace pimc
