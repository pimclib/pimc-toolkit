#pragma once

#include <fmt/format.h>

namespace pimc {

inline auto& getMemoryBuffer() {
    thread_local fmt::memory_buffer buf;
    buf.clear();
    return buf;
}

} // namespace pimc
