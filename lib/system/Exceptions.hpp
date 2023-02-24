#pragma once

#include <concepts>
#include <stdexcept>

#include <fmt/format.h>

#include "pimc/text/MemoryBuffer.hpp"

namespace pimc {

/**
 * Throws the specified exception with the error message formatted
 * per the supplied parameters.
 *
 * @tparam Ex an ``std::runtime_error``, or a ``std::logic_error`` or
 *         any exception derived from any of these
 * @tparam Ts the types of the parameters to the format string
 * @param fs the format string
 * @param args the arguments to match the format string
 */
template <typename Ex, typename ... Ts>
requires
        std::derived_from<Ex, std::logic_error> or
        std::derived_from<Ex, std::runtime_error>
[[noreturn]]
void raise(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
    auto& mb = getMemoryBuffer();
    auto bi = std::back_inserter(mb);
    fmt::format_to(bi, fs, std::forward<Ts>(args)...);
    throw Ex{fmt::to_string(mb)};
}

} // namespace pimc
