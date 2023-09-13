#pragma once

#include <concepts>
#include <ranges>
#include <algorithm>

namespace pimc {

/*!
 * \brief Checks the case insensitive equality of two ASCII strings.
 *
 * @tparam R1 The type of \p lhs
 * @tparam R2 Tye type of \p rhs
 * @param lhs the first string
 * @param rhs the second string
 * @return true if the strings are case-insensitively equal, false
 * otherwise
 */
template <
        std::ranges::input_range R1,
        std::ranges::input_range R2>
requires std::same_as<std::ranges::range_value_t<R1>, char> and
         std::same_as<std::ranges::range_value_t<R2>, char>
bool ciAsciiStrEq(R1&& lhs, R2&& rhs) {
    auto toLower{std::ranges::views::transform(
            [] (char c) { return static_cast<char>(std::tolower(c)); })};

    return std::ranges::equal(lhs | toLower, rhs | toLower);
}

} // namespace pimc
