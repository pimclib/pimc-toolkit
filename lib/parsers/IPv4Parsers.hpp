#pragma once

#include <cstdint>
#include <cctype>
#include <concepts>
#include <iterator>
#include <ranges>
#include <tuple>
#include <optional>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"
#include "pimc/text/CString.hpp"

namespace pimc {

namespace detail {

template<uint32_t MaxVal, std::input_iterator I, std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseBoundedUInt(I first, S last) -> std::tuple<uint32_t, I> {
    uint32_t oct{0};
    I ii{first};
    bool haveDigs{false};
    while (ii != last) {
        auto c = *ii;
        if (std::isdigit(static_cast<unsigned char>(c))) {
            oct = oct * 10 + static_cast<uint32_t>(c - '0');
            if (oct > MaxVal)
                return std::make_tuple(0u, first);
            haveDigs = true;
            ++ii;
        } else break;
    }

    if (not haveDigs) return std::make_tuple(0u, first);
    return std::make_tuple(oct, ii);
}

template <std::input_iterator I, std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseIPv4AddressImpl(I first, S last)  -> std::tuple<uint32_t, I> {
    I ii{first}, is;
    uint32_t o1;
    std::tie(o1, is) = parseBoundedUInt<255u>(ii, last);
    if (is == ii or is == last or *is != '.') return std::make_tuple(0u, first);

    uint32_t o2;
    ii = is;
    ++ii;
    std::tie(o2, is) = parseBoundedUInt<255u>(ii, last);
    if (is == ii or is == last or *is != '.') return std::make_tuple(0u, first);

    uint32_t o3;
    ii = is;
    ++ii;
    std::tie(o3, is) = parseBoundedUInt<255u>(ii, last);
    if (is == ii or is == last or *is != '.') return std::make_tuple(0u, first);

    uint32_t o4;
    ii = is;
    ++ii;
    std::tie(o4, is) = parseBoundedUInt<255u>(ii, last);
    if (is == ii) return std::make_tuple(0u, first);

    return std::make_tuple((o1 << 24u) + (o2 << 16u) + (o3 << 8u) + o4, is);
}

template <std::input_iterator I, std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseIPv4PrefixLengthImpl(I first, S last)  -> std::tuple<uint32_t, I> {
    I ii{first};

    if (ii == last) return std::make_tuple(0u, first);
    if (*ii != '/') return std::make_tuple(0u, first);
    ++ii;

    uint32_t plen;
    I is;
    std::tie(plen, is) = parseBoundedUInt<32>(ii, last);
    if (is == ii) return std::make_tuple(0u, first);

    return std::make_tuple(plen, is);
}

} // namespace detail

/*!
 * \brief Parses the input as an IPv4 address in the dotted decimal notation.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return an optional value containing an IPv4 address if parsing is successful, or
 * an empty optional value otherwise
 */
template <std::input_iterator I, std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseIPv4Address(I first, S last) -> std::optional<IPv4Address> {
    I is;
    uint32_t addr;
    std::tie(addr, is) = detail::parseIPv4AddressImpl(first, last);
    if (is != last) return {};
    return IPv4Address{addr};
}

/*!
 * \brief Parses the input as an IPv4 address in the dotted decimal notation.
 *
 * @tparam R the character input range type
 * @param r the input
 * @return an optional value containing an IPv4 address if parsing is successful, or
 * an empty optional value otherwise
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseIPv4Address(R&& r) -> std::optional<IPv4Address> {
    return parseIPv4Address(r.begin(), r.end());
}

/*!
 * \brief Parses the input as an IPv4 address in the dotted decimal notation.
 *
 * @param s the input
 * @return an optional value containing an IPv4 address if parsing is successful, or
 * an empty optional value otherwise
 */
inline auto parseIPv4Address(char const* s) -> std::optional<IPv4Address> {
    return parseIPv4Address(s, pimc::cssentinel{});
}

/*!
 * \brief Parses the input as an IPv4 prefix where the address is in the dotted
 * decimal notation followed by a slash followed by the prefix length.
 *
 * @tparam I the input iterator type
 * @tparam S the sentinel type for the input iterator
 * @param first the iterator positioned at the first character of input
 * @param last the sentinel for the end of input
 * @return an optional value containing an IPv4 prefix if parsing is successful, or
 * an empty optional value otherwise
 */
template <std::input_iterator I, std::sentinel_for<I> S>
requires std::same_as<std::iter_value_t<I>, char>
auto parseIPv4Prefix(I first, S last) -> std::optional<IPv4Prefix> {
    I ii{first}, is;
    uint32_t addr;
    std::tie(addr, is) = detail::parseIPv4AddressImpl(ii, last);
    if (is == ii or is == last) return {};

    uint32_t plen;
    ii = is;
    std::tie(plen, is) = detail::parseIPv4PrefixLengthImpl(ii, last);
    if (is == ii or is != last) return {};

    return IPv4Prefix::make(IPv4Address{addr}, plen);
}

/*!
 * \brief Parses the input as an IPv4 prefix where the address is in the dotted
 * decimal notation followed by a slash followed by the prefix length.
 *
 * @tparam R the character input range type
 * @param r the input
 * @return an optional value containing an IPv4 prefix if parsing is successful, or
 * an empty optional value otherwise
 */
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, char>
auto parseIPv4Prefix(R&& r) -> std::optional<IPv4Prefix> {
    return parseIPv4Prefix(r.begin(), r.end());
}

/*!
 * \brief Parses the input as an IPv4 prefix where the address is in the dotted
 * decimal notation followed by a slash followed by the prefix length.
 *
 * @param s the input
 * @return an optional value containing an IPv4 prefix if parsing is successful, or
 * an empty optional value otherwise
 */
inline auto parseIPv4Prefix(char const* s) -> std::optional<IPv4Prefix> {
    return parseIPv4Prefix(s, cssentinel{});
}

} // namespace pimc