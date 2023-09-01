#pragma once

#include "PIMSMConfig.hpp"
#include "JPConfig.hpp"

#include "pimsm/Update.hpp"

namespace pimc {

template <IPVersion V>
class PIMCConfig final {
public:
    PIMCConfig(
            PIMSMConfig<V> pimsmConfig,
            JPConfig<V> jpConfig,
            std::vector<Update<V>> updates,
            std::vector<Update<V>> inverseUpdates)
            : pimsmConfig_{std::move(pimsmConfig)}
            , jpConfig_{std::move(jpConfig)}
            , updates_{std::move(updates)}
            , inverseUpdates_{std::move(inverseUpdates)} {}

    [[nodiscard]]
    PIMSMConfig<V> const& pimsmConfig() const { return pimsmConfig_; }

    [[nodiscard]]
    JPConfig<V> const& jpConfig() const { return jpConfig_; }

    [[nodiscard]]
    std::vector<Update<V>> const& updates() const { return updates_; }

    [[nodiscard]]
    std::vector<Update<V>> const& inverseUpdates() const { return inverseUpdates_; }
private:
    PIMSMConfig<V> pimsmConfig_;
    JPConfig<V> jpConfig_;
    std::vector<Update<V>> updates_;
    std::vector<Update<V>> inverseUpdates_;
};

} // namespace pimc
