#pragma once

#include <cstdint>

namespace pimc {

/*!
 * Returns the number of decimal digits in \p v.
 *
 * @param v the value
 * @return the number of digits in \p v
 */
inline std::size_t decimalUIntLen(uint64_t v) {
    if (v == 0) return 1ul;

    std::size_t sz{0ul};
    while (v > 0) {
        ++sz;
        v /= 10;
    }
    return sz;
}

/*!
 * \brief For a positive \p v returns the number of decimal digits in it,
 * for a negative one, returns the number of digits plus one (for the minus
 * sign)
 *
 * @param v the value
 * @return the number of characters required to format \p v
 */
inline std::size_t decimalIntLen(int64_t v) {
    if (v >= 0)
        return decimalUIntLen(static_cast<uint64_t>(v));

    return decimalUIntLen(static_cast<uint64_t>(-v)) + 1;
}

} // namespace pimc
