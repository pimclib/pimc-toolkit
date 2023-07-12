#pragma once

#include "PIMSMConfig.hpp"
#include "JPConfig.hpp"

namespace pimc {

template <net::IPAddress A>
class PIMCConfig final {
public:
    PIMCConfig(PIMSMConfig<A> pimsmConfig, JPConfig<A> jpConfig)
    : pimsmConfig_{std::move(pimsmConfig)}, jpConfig_{std::move(jpConfig)} {}

    [[nodiscard]]
    PIMSMConfig<A> const& pimsmConfig() const { return pimsmConfig_; }

    [[nodiscard]]
    JPConfig<A> const& jpConfig() const { return jpConfig_; }
private:
    PIMSMConfig<A> pimsmConfig_;
    JPConfig<A> jpConfig_;
};

} // namespace pimc
