#pragma once

#include <cstdint>
#include <random>

namespace pimc {

class GenerationID final {
public:

    GenerationID(): prng_(std::random_device{}()) {}

    uint32_t next() { return dist_(prng_); }

private:
    std::mt19937 prng_;
    std::uniform_int_distribution<uint32_t> dist_;
};

};