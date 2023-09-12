#pragma once

#include <cstdint>
#include <vector>

#include "pimsm/Update.hpp"

namespace pimc {

class IPv4PIMUpdatePacket final {
public:
    IPv4PIMUpdatePacket(
            Update<IPv4> const& update,
            IPv4Address source, IPv4Address neighbor, uint16_t holdtime);

    [[nodiscard]]
    void const* data() const { return static_cast<void const*>(data_.data()); }

    [[nodiscard]]
    size_t size() const { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

} // namespace pimc
