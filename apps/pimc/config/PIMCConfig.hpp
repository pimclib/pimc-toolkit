#pragma once

#include "PIMSMConfig.hpp"
#include "JPConfig.hpp"

namespace pimc::pimsm_config {

template <IPVersion V>
class PIMCConfig final {
public:
    PIMCConfig(PIMSMConfig<V> pimsmConfig, JPConfig<V> jpConfig)
    : pimsmConfig_{std::move(pimsmConfig)}, jpConfig_{std::move(jpConfig)} {}

    [[nodiscard]]
    PIMSMConfig<V> const& pimsmConfig() const { return pimsmConfig_; }

    [[nodiscard]]
    JPConfig<V> const& jpConfig() const { return jpConfig_; }
private:
    PIMSMConfig<V> pimsmConfig_;
    JPConfig<V> jpConfig_;
};

} // namespace pimc::pimsm_config
