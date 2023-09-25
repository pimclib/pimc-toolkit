#pragma once

#include <cstdint>
#include <concepts>
#include <iterator>
#include <ranges>

#include "pimc/core/TypeUtils.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/text/CString.hpp"

namespace pimc {

/*!
 * The error codes indicating the type of failure which can happen
 * while parsing numbers.
 */
enum class NumberParseError: unsigned {
    /*!
     * The input contains characters which do not form a valid decimal number
     * for the target numeric type
     */
    Invalid = 1,
    /*!
     * The input represents a number which is outside of the limits
     * of the target numeric type
     */
    Overflow = 2
};

/*!
 * A generic algorithm for parsing unsigned decimal integers.
 *
 * @tparam UIntType the target unsigned integer type
 * @tparam LastStep the "last step" number
 * @tparam LastDigit the "last step" digit
 * @tparam I The input iterator
 * @tparam S The sentinel for the input iterator
 * @param first the start of input
 * @param last the end of input
 * @return a Result containing a value or the parsing error
 */
template <UInt UIntType,
          UIntType LastStep,
          UIntType LastDigit,
          std::input_iterator I,
          std::sentinel_for<I> S>
          requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalUnsignedInteger(
        I first, S last) -> Result<UIntType, NumberParseError> {
    if (first == last)
        return fail(NumberParseError::Invalid);

    bool ovf{false};
    UIntType acc{0};

    for (std::input_iterator auto ii = first; ii != last; ++ii) {
        auto c = *ii;
        if (not std::isdigit(static_cast<unsigned char>(c)))
            return fail(NumberParseError::Invalid);

        if (ovf) continue;

        pimc::UInt auto d = static_cast<UIntType>(c - '0');
        if (acc < LastStep || (acc == LastStep && d <= LastDigit)) {
            acc = static_cast<UIntType>(acc * 10 + d);
            continue;
        }

        ovf = true;
    }

    if (ovf)
        return fail(NumberParseError::Overflow);

    return acc;
}

/*!
 * Parses the input as uint64_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint64_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
        requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalUInt64(
        I first, S last) -> Result<uint64_t, NumberParseError> {
    return parseDecimalUnsignedInteger<
            uint64_t, 1844674407370955161ul, 5ul, I, S>(first, last);
}

/*!
 * Parses the input as uint64_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed uint64_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalUInt64(R&& r) -> Result<uint64_t, NumberParseError> {
    return parseDecimalUInt64(r.begin(), r.end());
}

/*!
 * Parses the input as uint64_t.
 *
 * @param s the input
 * @return a result containing the parsed uint64_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalUInt64(char const* s)
-> Result<uint64_t, NumberParseError> {
    return parseDecimalUInt64(s, cssentinel{});
}

/*!
 * Parses the input as uint32_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint32_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
        requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalUInt32(
        I first, S last) -> Result<uint32_t, NumberParseError> {
    return parseDecimalUnsignedInteger<
            uint32_t, 429496729u, 5u, I, S>(first, last);
}

/*!
 * Parses the input as uint32_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed uint32_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalUInt32(R&& r) -> Result<uint32_t, NumberParseError> {
    return parseDecimalUInt32(r.begin(), r.end());
}

/*!
 * Parses the input as uint32_t.
 *
 * @param s the input
 * @return a result containing the parsed uint32_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalUInt32(char const* s)
-> Result<uint32_t, NumberParseError> {
    return parseDecimalUInt32(s, cssentinel{});
}

/*!
 * Parses the input as uint16_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint16_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
        requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalUInt16(
        I first, S last) -> Result<uint16_t, NumberParseError> {
    return parseDecimalUnsignedInteger<
            uint16_t, 6553u, 5u, I, S>(first, last);
}

/*!
 * Parses the input as uint16_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed uint16_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalUInt16(R&& r) -> Result<uint16_t, NumberParseError> {
    return parseDecimalUInt16(r.begin(), r.end());
}

/*!
 * Parses the input as uint16_t.
 *
 * @param s the input
 * @return a result containing the parsed uint16_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalUInt16(char const* s)
-> Result<uint16_t, NumberParseError> {
    return parseDecimalUInt16(s, cssentinel{});
}

/*!
 * Parses the input as uint8_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint8_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
        requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalUInt8(
        I first, S last) -> Result<uint8_t, NumberParseError> {
    return parseDecimalUnsignedInteger<
            uint8_t, 25u, 5u, I, S>(first, last);
}

/*!
 * Parses the input as uint8_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed uint8_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalUInt8(R&& r) -> Result<uint8_t, NumberParseError> {
    return parseDecimalUInt8(r.begin(), r.end());
}

/*!
 * Parses the input as uint8_t.
 *
 * @param s the input
 * @return a result containing the parsed uint8_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalUInt8(char const* s)
-> Result<uint8_t, NumberParseError> {
    return parseDecimalUInt8(s, cssentinel{});
}

/*!
 * A generic algorithm for parsing unsigned decimal integers.
 *
 * @tparam SIntType the target signed integer type
 * @tparam LastStep the "last step" number
 * @tparam PositiveLastDigit the positive "last step" digit
 * @tparam NegativeLastDigit the negative "last step" digit
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator type
 * @param first the start of input
 * @param last the end of input
 * @return a Result containing a value or the parsing error
 */
template <SInt SIntType,
        SIntType LastStep,
        SIntType PositiveLastDigit,
        SIntType NegativeLastDigit,
        std::input_iterator I,
        std::sentinel_for<I> S>
        requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalSignedInteger(
        I first, S last) -> Result<SIntType, NumberParseError> {
    if (first == last)
        return fail(NumberParseError::Invalid);

    SIntType lastDigit = PositiveLastDigit;
    bool positive = true;

    auto c = *first;
    if (c == '-') {
        lastDigit = NegativeLastDigit;
        positive = false;

        if (++first == last)
            return fail(NumberParseError::Invalid);
    } else if (c == '+') {
        if (++first == last)
            return fail(NumberParseError::Invalid);
    }

    bool ovf{false};
    SIntType acc{0};

    for (std::input_iterator auto ii = first; ii != last; ++ii) {
        c = *ii;
        if (not std::isdigit(static_cast<unsigned char>(c)))
            return fail(NumberParseError::Invalid);

        if (ovf) continue;

        pimc::SInt auto d = static_cast<SIntType>(c - '0');
        if (acc < LastStep || (acc == LastStep && d <= lastDigit)) {
            acc = static_cast<SIntType>(acc * 10 + d);
            continue;
        }

        ovf = true;
    }

    if (ovf)
        return fail(NumberParseError::Overflow);

    if (positive) return acc;

    return static_cast<SIntType>(-acc);
}

/*!
 * Parses the input as int64_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint64_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
        requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalInt64(
        I first, S last) -> Result<int64_t, NumberParseError> {
    return parseDecimalSignedInteger<
            int64_t, 922337203685477580l, 7l, 8l, I, S>(first, last);
}

/*!
 * Parses the input as int64_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed int64_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalInt64(R&& r) -> Result<int64_t, NumberParseError> {
    return parseDecimalInt64(r.begin(), r.end());
}

/*!
 * Parses the input as int64_t.
 *
 * @param s the input
 * @return a result containing the parsed int64_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalInt64(char const* s)
-> Result<int64_t, NumberParseError> {
    return parseDecimalInt64(s, cssentinel{});
}

/*!
 * Parses the input as int32_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint32_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalInt32(
        I first, S last) -> Result<int32_t, NumberParseError> {
    return parseDecimalSignedInteger<
            int32_t, 214748364, 7, 8, I, S>(first, last);
}

/*!
 * Parses the input as int32_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed int32_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalInt32(R&& r) -> Result<int32_t, NumberParseError> {
    return parseDecimalInt32(r.begin(), r.end());
}

/*!
 * Parses the input as int32_t.
 *
 * @param s the input
 * @return a result containing the parsed int32_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalInt32(char const* s)
-> Result<int32_t, NumberParseError> {
    return parseDecimalInt32(s, cssentinel{});
}

/*!
 * Parses the input as int16_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint16_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalInt16(
        I first, S last) -> Result<int16_t, NumberParseError> {
    return parseDecimalSignedInteger<
            int16_t, 3276, 7, 8, I, S>(first, last);
}

/*!
 * Parses the input as int16_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed int16_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalInt16(R&& r) -> Result<int16_t, NumberParseError> {
    return parseDecimalInt16(r.begin(), r.end());
}

/*!
 * Parses the input as int16_t.
 *
 * @param s the input
 * @return a result containing the parsed int16_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalInt16(char const* s)
-> Result<int16_t, NumberParseError> {
    return parseDecimalInt16(s, cssentinel{});
}

/*!
 * Parses the input as int8_t.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return a result containing the parsed uint8_t or an error code if the
 * input is invalid
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseDecimalInt8(
        I first, S last) -> Result<int8_t, NumberParseError> {
    return parseDecimalSignedInteger<
            int8_t, 12, 7, 8, I, S>(first, last);
}

/*!
 * Parses the input as int8_t
 *
 * @tparam R the character input range type
 * @param r the input
 * @return a result containing the parsed int8_t or an error code if the
 * input is invalid
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseDecimalInt8(R&& r) -> Result<int8_t, NumberParseError> {
    return parseDecimalInt8(r.begin(), r.end());
}

/*!
 * Parses the input as int8_t.
 *
 * @param s the input
 * @return a result containing the parsed int8_t or an error code if the
 * input is invalid
 */
inline auto parseDecimalInt8(char const* s)
-> Result<int8_t, NumberParseError> {
    return parseDecimalInt8(s, cssentinel{});
}

} // namespace pimc
