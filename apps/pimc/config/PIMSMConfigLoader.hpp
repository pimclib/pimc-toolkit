#pragma once

#include "pimc/formatters/Fmt.hpp"
#include "pimc/net/IP.hpp"
#include "pimc/net/IntfTable.hpp"
#include "pimc/formatters/IntfTableFormatter.hpp"
#include "pimc/yaml/BuilderBase.hpp"

#include "PIMSMConfig.hpp"
#include "ConfigUtils.hpp"

namespace pimc::pimsm_config {

template <IPVersion V>
class PIMSMConfigLoader final: BuilderBase {
public:
    using IPAddress = typename IP<V>::Address;

    using BuilderBase::BuilderBase;

    void loadPIMSMConfig(
            yaml::ValueContext const& vCtx, IntfTable const& intfTable) {
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
                auto rIntfInfo = intfTable.byName(rIntf->value());
                if (not rIntfInfo) {
                    auto& buf = getMemoryBuffer();
                    auto bi = std::back_inserter(buf);
                    fmt::format_to(bi, "unknown interface '{}'\n\n", rIntf->value());
                    fmt::format_to(bi, "  available interfaces:\n");
                    formatIntfTable(bi, intfTable, 2, false);
                    consume(rIntf->error(fmt::to_string(buf)));
                } else {
                    Optional<IPAddress> intfAddr = IPIntf<V>::address(rIntfInfo.value());
                    if (not intfAddr) {
                        auto& buf = getMemoryBuffer();
                        auto bi = std::back_inserter(buf);
                        fmt::format_to(bi, "interface {} has no {} address\n", rIntf->value(), V{});
                        fmt::format_to(bi, "  available interfaces:\n");
                        formatIntfTable(bi, intfTable, 2, false);
                        consume(rIntf->error(fmt::to_string(buf)));
                    } else intfAddr_ = intfAddr.value();
                }
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
auto loadPIMSMConfig(
        yaml::ValueContext const& pimsmCfgCtx, IntfTable const& intfTable)
-> Result<PIMSMConfig<V>, std::vector<yaml::ErrorContext>> {
    std::vector<yaml::ErrorContext> errors;
    PIMSMConfigLoader<V> pimsmCfgLdr{errors};
    pimsmCfgLdr.loadPIMSMConfig(pimsmCfgCtx, intfTable);
    if (not errors.empty()) return fail(errors);
    return pimsmCfgLdr.build();
}

} // namespace pimc::pimsm_config
