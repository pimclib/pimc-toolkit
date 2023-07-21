#pragma once

#include "pimc/net/IPv4Address.hpp"

namespace pimc {

/*!
 * An IPv4 prefix -- this is a wrapper over two 32-bit unsigned integer values
 * which represent the IP address and prefix length portions of an IPv4 prefix.
 * This class provides the IPv4 prefix specific arithmetic operations.
 */
class IPv4Prefix final {
public:
    /*!
     * Constructs the default prefix 0.0.0.0/0
     */
    constexpr IPv4Prefix(): addr_{}, plen_{0} {}

    /*!
     * A factory method which creates an IP prefix from the specified IP address
     * and prefix length. The resulting prefix will have the subnet mask applied
     * to its address to ensure the canonical internal representation, i.e.
     * address 123.235.34.45 with prefix length /15 becomes 123.234.0.0/15.
     *
     * @param addr the IP address
     * @param plen the prefix length, which must be in range 0-32
     * @return the corresponding prefix
     * @throws std::invalid_argument if the prefix length is greater than 32
     */
    static IPv4Prefix make(IPv4Address addr, uint32_t plen) {
        return IPv4Prefix{addr & IPv4Address::toMask(plen), plen};
    };

    /*!
     * Returns the default (0.0.0.0/0) prefix.
     * @return the default prefix
     */
    static constexpr IPv4Prefix Default() {
        return IPv4Prefix{IPv4Address{0,0,0,0}, 0}; }

    /*!
     * Returns the loopback network 127.0.0.0/8.
     * @return the loopback network
     */
    static constexpr IPv4Prefix Loopback() {
        return IPv4Prefix{IPv4Address{127,0,0,0}, 8};
    }

    /*!
     * Returns the multicast address space 224.0.0.0/4.
     * @return the multicast address space
     */
    static constexpr IPv4Prefix Multicast() {
        return IPv4Prefix{IPv4Address{224,0,0,0}, 4};
    }

    /*!
     * Returns the zero network 0.0.0.0/8.
     * @return 0.0.0.0/8
     */
    static constexpr IPv4Prefix Zero() {
        return IPv4Prefix{IPv4Address{0,0,0,0}, 8};
    }

    /*!
     * Returns the IP address of the prefix.
     * @return the IP address of the prefix
     */
    [[nodiscard]]
    constexpr IPv4Address address() const noexcept { return addr_; }

    /*!
     * Returns the length of the prefix.
     * @return the length of the prefix
     */
    [[nodiscard]]
    constexpr uint32_t length() const noexcept { return plen_; }

    /*!
     * Returns true if this IP prefix is numerically less than the right hand side
     * argument. If the IP address portions of both prefixes are equal then the
     * prefix with the numerically smaller length is considered smaller.
     *
     * @param rhs the IP prefix to which this IP prefix is compared
     * @return true if this IP prefix is numerically less than the right hand side
     * argument, false otherwise
     */
    constexpr bool operator < (IPv4Prefix rhs) const noexcept {
        if (addr_ < rhs.addr_) return true;
        if (addr_ > rhs.addr_) return false;
        return plen_ < rhs.plen_;
    }

    /*!
     * Returns true if this IP prefix is numerically less than or equal to the right
     * hand side argument. If the IP address portions of both prefixes are equal then
     * the prefix with the numerically smaller length is considered smaller.
     *
     * @param rhs the IP prefix to which this IP prefix is compared
     * @return true if this IP prefix is numerically less than or equal to the right
     * hand side argument, false otherwise
     */
    constexpr bool operator <= (IPv4Prefix rhs) const noexcept {
        if (addr_ < rhs.addr_) return true;
        if (addr_ > rhs.addr_) return false;
        return plen_ <= rhs.plen_;
    }

    /*!
     * Returns true if this IP prefix is numerically greater than the right hand
     * side argument. If the IP address portions of both prefixes are equal then
     * the prefix with the numerically greater length is considered greater.
     *
     * @param rhs the IP prefix to which this IP prefix is compared
     * @return true if this IP prefix is numerically greater than the right hand
     * side argument, false otherwise
     */
    constexpr bool operator > (IPv4Prefix rhs) const noexcept {
        if (addr_ > rhs.addr_) return true;
        if (addr_ < rhs.addr_) return false;
        return plen_ > rhs.plen_;
    }

    /*!
     * Returns true if this IP prefix is numerically greater than or equal to
     * the right hand side argument. If the IP address portions of both prefixes
     * are equal then the prefix with the numerically greater length is
     * considered greater.
     *
     * @param rhs the IP prefix to which this IP prefix is compared
     * @return true if this IP prefix is numerically greater than or equal to
     * the right hand side argument, false otherwise
     */
    constexpr bool operator >= (IPv4Prefix rhs) const noexcept {
        if (addr_ > rhs.addr_) return true;
        if (addr_ < rhs.addr_) return false;
        return plen_ >= rhs.plen_;
    }

    /*!
     * Returns true if this IP prefix is numerically equal to the right hand
     * side argument. This means that both the address and length portions of
     * both IP prefixes must be equal.
     *
     * @param rhs the IP prefix to which this IP prefix is compared
     * @return true if this IP prefix is numerically equal to the right hand
     * side argument, false otherwise
     */
    constexpr bool operator == (IPv4Prefix rhs) const noexcept {
        return addr_ == rhs.addr_ && plen_ == rhs.plen_;
    }

    /*!
     * Returns true if this IP prefix is numerically not equal to the right hand
     * side argument. The inequality is satisfied if either the address or
     * length portion of the prefixes is unequal.
     *
     * @param rhs the IP prefix to which this IP prefix is compared
     * @return true if this IP prefix is numerically not equal to the right hand
     * side argument, false otherwise
     */
    constexpr bool operator != (IPv4Prefix rhs) const noexcept {
        return addr_ != rhs.addr_ || plen_ != rhs.plen_;
    }

    /*!
     * Returns true of the specified IP address belongs to this prefix.
     * @param addr the IP address
     * @return true if the IP address belong to this prefix, false otherwise
     */
    [[nodiscard]]
    constexpr bool contains(IPv4Address addr) const noexcept {
        return addr_ == (addr & IPv4Address{IPv4Address::maskValue(plen_)});
    }

    /*!
     * Returns true if the specified IP prefix is contained in this prefix,
     * that is its prefix length is strictly greater than that of this
     * prefix and its address belongs to this prefix.
     *
     * @param rhs the IP prefix
     * @return true if this prefix contains the specified prefix, false
     * otherwise
     */
    [[nodiscard]]
    constexpr bool contains(IPv4Prefix rhs) const noexcept {
        if (plen_ >= rhs.plen_) return false;
        return contains(rhs.addr_);
    }

    /*!
     * Returns the number of characters required to format this prefix with
     * the address in the dotted decimal notation followed by a slash followed
     * by the prefix length.
     *
     * @return the number of characters required to format this prefix with
     * the address in the dotted decimal notation followed by a slash followed
     * by the prefix length
     */
    [[nodiscard]]
    constexpr size_t charlen() const noexcept {
        return addr_.charlen() + 1 + (plen_ < 10 ? 1 : 2);
    }

private:
    constexpr IPv4Prefix(IPv4Address addr, uint32_t plen) noexcept
    : addr_{addr}, plen_{plen} {}

    IPv4Address addr_;
    uint32_t plen_;
};

} // namespace pimc


namespace std {

template <> struct hash<pimc::IPv4Prefix> {
    std::size_t operator() (
            pimc::IPv4Prefix prefix) const noexcept{
        std::size_t h1 = std::hash<pimc::IPv4Address>{}(prefix.address());
        std::size_t h2 = std::hash<uint32_t>{}(prefix.length());
        return h1 ^ (h2 << 1);
    }
};

} // namespace std

