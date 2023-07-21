#pragma once

#include <optional>

#include "pimc/net/IP.hpp"
#include "pimc/parsers/IPv4Parsers.hpp"

namespace pimc {

template <typename>
struct parse {};

template <>
struct parse<IPv4> {

    /*!
     * \copydoc ::pimc::parseIPv4Address(I, S)
     */
    template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::same_as<std::iter_value_t<I>, char>
    static auto address (I first, S last) -> std::optional<IPv4Address> {
        return parseIPv4Address(first, last);
    }

    /*!
     * \copydoc ::pimc::parseIPv4Address(R&&)
     */
    template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, char>
    static auto address(R&& r) -> std::optional<IPv4Address> {
        return parseIPv4Address(std::forward<R>(r));
    }

    /*!
     * \copydoc ::pimc::parseIPv4Address(char const*)
     */
    inline static auto address(char const* s) -> std::optional<IPv4Address> {
        return parseIPv4Address(s);
    }

    /*!
     * \copydoc ::pimc::parseIPv4Prefix(I, S)
     */
    template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::same_as<std::iter_value_t<I>, char>
    static auto prefix (I first, S last) -> std::optional<IPv4Prefix> {
        return parseIPv4Prefix(first, last);
    }

    /*!
     * \copydoc ::pimc::parseIPv4Prefix(R&&)
     */
    template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, char>
    static auto prefix(R&& r) -> std::optional<IPv4Prefix> {
        return parseIPv4Prefix(std::forward<R>(r));
    }

    /*!
     * \copydoc ::pimc::parseIPv4Prefix(char const*)
     */
    inline static auto prefix(char const* s) -> std::optional<IPv4Prefix> {
        return parseIPv4Prefix(s);
    }
};

} // namespace pimc
