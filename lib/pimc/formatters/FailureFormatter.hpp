#pragma once

#include <string>
#include "pimc/core/Result.hpp"
#include "Fmt.hpp"

namespace pimc {

template <typename ... Ts>
Failure<std::string> sfail(fmt::format_string<Ts...> const& fs, Ts&& ... args) {
    return fmt::format(fs, std::forward<Ts>(args)...);
}

} // namespace pimc
