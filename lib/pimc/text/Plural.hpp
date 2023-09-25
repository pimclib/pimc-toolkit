#pragma once

#include <utility>
#include <concepts>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

PIMC_ALWAYS_INLINE
char const* plural(std::size_t sz) {
    return sz != 1 ? "s" : "";
}

template <typename C>
concept HasSize = requires(C c) {
    { c.size() } -> std::same_as<std::size_t>;
};

template <HasSize C>
char const* plural(C&& c) {
    return plural(std::forward<C>(c).size());
}

} // namespace pimc