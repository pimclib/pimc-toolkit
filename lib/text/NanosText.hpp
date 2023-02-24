#pragma once

#include <cstdint>
#include <tuple>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

/**
 * Nanosecond formatter object.
 */
class NanosText final {
public:
    constexpr NanosText() {};

    /**
     * Formats the specified nanoseconds to the specified precision
     * and returns a tuple consisting of a pointer to the internal buffer
     * containing a c-string representing the formatted value and an
     * unsigned int indicating a carry. The trailing zeroes are always
     * truncated.
     *
     * The carry is equal to 1 only if the nanoseconds were rounded up
     * to satisfy the specified precision and the rounding produced an
     * overflow. For example 999500000 with precision 3 will result in
     * the return value ("", 1).
     *
     * @param nanos the unsigned integer representing the nanoseconds;
     * value greater than 999999999 result in un unspecified behavior
     * @param prec the precision to which to round up the result
     * @return a tuple of the formatted nanosecond value with the specified
     * precision and a carry
     */
    PIMC_ALWAYS_INLINE
    std::tuple<char const*, uint64_t> prc(uint64_t nanos, unsigned prec) {
        carry = 0;
        if (prec == 0 || nanos == 0) buf[0] = '\0';
        else {
            int ln0{0};
            bool ln0set{false};
            for (int i = 8; i > -1; --i) {
                uint64_t lastD = nanos % 10 + carry;
                nanos /= 10;
                if (lastD == 10) lastD = 0;
                else carry = 0;

                if (static_cast<unsigned>(i) < prec) {
                    if (! ln0set) {
                        if (lastD == 0) continue;

                        ln0set = true;
                        ln0 = i;
                    }
                    buf[i] = static_cast<char>('0' + lastD);
                } else {
                    if (static_cast<unsigned>(i) > prec) continue;
                    else carry = lastD > 4 ? 1 : 0;
                }
            }
            if (not ln0set) buf[0] = '\0';
            else buf[ln0 + 1] = '\0';
        }

        return std::make_tuple(buf, carry);
    }

private:
    char buf[10];
    uint64_t carry;
};

} // namespace pimc