#pragma once

#include <cstdint>
#include <vector>

#include "pimc/net/IPv4Address.hpp"

namespace pimc {

class IPv4PIMHelloPacket final {
public:
    IPv4PIMHelloPacket(
            IPv4Address source,
            uint16_t helloHoldtime, uint32_t drPriority, uint32_t generationId);

    [[nodiscard]]
    void const* data() const { return static_cast<void const*>(data_.data()); }

    [[nodiscard]]
    size_t size() const { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

} // namespace pimc
