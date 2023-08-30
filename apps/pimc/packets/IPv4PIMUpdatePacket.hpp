#pragma once

#include <cstdint>
#include <vector>

#include "pimsm/Update.hpp"

namespace pimc {

class IPv4PIMUpdatePacket final {
public:
    static IPv4PIMUpdatePacket create(
            Update<IPv4> const& update,
            IPv4Address source, IPv4Address neighbor, uint16_t holdtime);

    [[nodiscard]]
    void const* data() const { return static_cast<void const*>(data_.data()); }

    [[nodiscard]]
    size_t size() const { return data_.size(); }
private:
    explicit IPv4PIMUpdatePacket(std::vector<uint8_t> data): data_{std::move(data)} {}

private:
    std::vector<uint8_t> data_;
};

} // namespace pimc
