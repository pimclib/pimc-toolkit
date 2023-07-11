#pragma once

#include "PIMSMConfig.hpp"
#include "JPConfig.hpp"

namespace pimc {

class PIMCConfig final {
public:
    PIMCConfig(PIMSMConfig pimsmConfig, JPConfig jpConfig)
    : pimsmConfig_{std::move(pimsmConfig)}, jpConfig_{std::move(jpConfig)} {}

    [[nodiscard]]
    PIMSMConfig const& pimsmConfig() const { return pimsmConfig_; }

    [[nodiscard]]
    JPConfig const& jpConfig() const { return jpConfig_; }
private:
    PIMSMConfig pimsmConfig_;
    JPConfig jpConfig_;
};

} // namespace pimc
