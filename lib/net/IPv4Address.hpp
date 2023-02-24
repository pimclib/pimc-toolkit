#pragma once

#include <cstdint>
#include <stdexcept>
#include <arpa/inet.h>

namespace pimc::net {

/*!
 * An IPv4 address class.
 */
class IPv4Address final {
    static constexpr uint32_t DefaultAddressValue{0};
    static constexpr uint32_t LocalBroadcastAddressValue{0xFFFFFFFF};
public:
    /*!
     * Constructs an IPv4 address from individual octets. The octets are specified
     * in the most significant first order.
     *
     * @param o1 the first (most significant) octet
     * @param o2 the second octet
     * @param o3 the third octet
     * @param o4 the fourth (least significant) octet
     */
    constexpr explicit IPv4Address(
            uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4) noexcept :
            addr_{
                (static_cast<uint32_t>(o1) << 24u) +
                (static_cast<uint32_t>(o2) << 16u) +
                (static_cast<uint32_t>(o3) << 8u) +
                 static_cast<uint32_t>(o4)}  {}

    /*!
     * Constructs the default IP address (0.0.0.0)
     */
    constexpr IPv4Address() noexcept: addr_(DefaultAddressValue) {}

    /*!
     * Constructs an IP address from the specified 32-bit unsigned value.
     * @param address the IP address value
     */
    constexpr explicit IPv4Address(uint32_t address)
    noexcept : addr_(address) {}

    /*!
     * Returns the 32-bit unsigned value of the IP address.
     */
    [[nodiscard]]
    constexpr uint32_t value() const { return addr_; }

    /*!
     * A factory method that creates an IP address from the 32-bit unsigned
     * value in the big endian (network) byte order.
     *
     * @param netlong the IP address value in the network byte order
     * @return the corresponding IP address
     */
    static IPv4Address from_nl(uint32_t netlong) {
        return IPv4Address(ntohl(netlong));
    }

    /*!
     * Returns an unsigned value whose bit pattern corresponds to the
     * subnet mask with the specified prefix length. If the prefix length
     * is greater than 32. Applying this function to prefix length greater
     * than 32 results in undefined behavior.
     *
     * @param plen the prefix length; must be in the range 0-32
     * @return the 32-bit unsigned value of the corresponding subnet mask
     */
    constexpr static uint32_t maskValue(uint32_t plen) {
        if (plen == 0) return 0;
        return LocalBroadcastAddressValue - ((1u << (32u - plen)) - 1u);
    }

    /*!
     * A factory method which converts a prefix length into an IP address of the
     * corresponding subnet mask, e.g. /11 becomes 255.224.0.0
     * @param plen the unsigned integer in the range 0-32 inclusive
     * @return an IP address of the corresponding subnet mask
     * @throws std::invalid_argument exception of the specified prefix length is
     * greater than 32
     */
    static IPv4Address toMask(uint32_t plen) {
        if (plen > 32)
            throw std::invalid_argument{"illegal IPv4 prefix length"};
        return IPv4Address{maskValue(plen)};
    };

    /*!
     * Converts this IP address into a 32-bit value in the network byte order
     * @return
     */
    [[nodiscard]]
    uint32_t to_nl() const noexcept { return htonl(addr_); }

    /*!
     * Returns the first (most significant) octet of the IP address.
     * @return the first octet of the IP address
     */
    [[nodiscard]]
    constexpr uint8_t oct1() const noexcept {
        return static_cast<uint8_t>(addr_ >> 24u);
    }

    /*!
     * Returns the second octet of the IP address
     * @return the second octet of the IP address
     */
    [[nodiscard]]
    constexpr uint8_t oct2() const noexcept {
        return static_cast<uint8_t>((addr_ >> 16u) & 0xFFu);
    }

    /*!
     * Returns the third octet of the IP address
     * @return the third octet of the IP address
     */
    [[nodiscard]]
    constexpr uint8_t oct3() const noexcept {
        return static_cast<uint8_t>((addr_ >> 8u) & 0xFFu);
    }

    /*!
     * Returns the fourth (least significant) octet of the IP address
     * @return the fourth octet of the IP address
     */
    [[nodiscard]]
    constexpr uint8_t oct4() const noexcept {
        return static_cast<uint8_t>(addr_ & 0xFFu);
    }

    /*!
     * Returns true if this IP address is multicast, i.e. in the range 224.0.0.0/4
     * @return true if this IP address is multicast, false otherwise
     */
    [[nodiscard]]
    constexpr bool isMcast() const noexcept {
        return (addr_ >> 28u) == 14u;
    }

    /*!
     * Returns true if this IP address is equal to 255.255.255.255
     * @return true if this IP address is the local broadcast address,
     * false otherwise
     */
    [[nodiscard]]
    constexpr bool isLocalBroadcast() const noexcept {
        return addr_ == LocalBroadcastAddressValue;
    }

    /*!
     * Returns true if this IP address is equal to 0.0.0.0
     * @return true if this IP address is default, false otherwise
     */
    [[nodiscard]]
    constexpr bool isDefault() const noexcept {
        return addr_ == DefaultAddressValue;
    }

    /*!
     * Returns true if this IP address is in the range 127.0.0.0/8
     * @return true if this IP address is loopback, false otherwise
     */
    [[nodiscard]]
    constexpr bool isLoopback() const noexcept {
        return (addr_ >> 24u) == 127u;
    }

    /*!
     * Returns true if this IP address can be used as a subnet mask, i.e. it has a
     * contiguous set of set bits at the beginning followed by a contiguous set of
     * cleared bits, where either set can be empty. For example 255.255.224.0.
     *
     * @return true if this address can be used a subnet mask, false otherwise
     */
    [[nodiscard]]
    constexpr bool isMask() const noexcept {
        if (addr_ == DefaultAddressValue) return true;
        uint32_t m = LocalBroadcastAddressValue - addr_;
        return ((m + 1) & m) == 0;
    }

    /*!
     * If this IP address is a subnet mask, converts it to the prefix length,
     * otherwise throws the std::logic_error exception.
     *
     * @return the prefix length corresponding to the subnet mask, e.g. 21 for the
     * address 255.255.248.0
     * @throws std::logic_exception if this IP address is not a subnet mask
     */
    [[nodiscard]]
    uint32_t toMask() const {
        if (addr_ == LocalBroadcastAddressValue) return 32;
        if (! isMask())
            throw std::logic_error("address is not mask");

        uint32_t pl{16};
        uint32_t m{LocalBroadcastAddressValue - addr_};
        uint32_t s{16};
        while (s > 1u) {
            uint32_t mn = m >> s;
            uint32_t sn = s >> 1u;
            if (mn > 0) {
                pl -= sn;
                m = mn;
            } else pl += sn;

            s = sn;
        }

        if (m == 1u) return pl;

        return pl - 1u;
    }

    /*!
     * Returns true if this IP address is numerically less than the right hand side
     * argument
     * @param rhs the IP address to which this IP address is compared
     * @return true if this IP address is numerically less than the right hand side
     * argument, false otherwise
     */
    constexpr bool operator < (IPv4Address rhs) const noexcept {
        return addr_ < rhs.addr_;
    }

    /*!
     * Returns true if this IP address is numerically less than or equal to the right
     * hand side argument
     * @param rhs the IP address to which this IP address is compared
     * @return true if this IP address is numerically less than or equal to the right
     * hand side argument, false otherwise
     */
    constexpr bool operator <= (IPv4Address rhs) const noexcept {
        return addr_ <= rhs.addr_;
    }

    /*!
     * Returns true if this IP address is numerically greater than the right hand side
     * argument
     * @param rhs the IP address to which this IP address is compared
     * @return true if this IP address is numerically greater than the right hand side
     * argument, false otherwise
     */
    constexpr bool operator > (IPv4Address rhs) const noexcept {
        return addr_ > rhs.addr_;
    }

    /*!
     * Returns true if this IP address is numerically greater than or euqal to the right
     * hand side argument
     * @param rhs the IP address to which this IP address is compared
     * @return true if this IP address is numerically greater than or equal to the right
     * hand side argument, false otherwise
     */
    constexpr bool operator >= (IPv4Address rhs) const noexcept {
        return addr_ >= rhs.addr_;
    }

    /*!
     * Returns true if this IP address is euqal to the right hand side argument
     * @param rhs the IP address to which this IP address is compared
     * @return true if this IP address is numerically equal to the right hand side argument,
     * false otherwise
     */
    constexpr bool operator == (IPv4Address rhs) const noexcept {
        return addr_ == rhs.addr_;
    }

    /*!
     * Returns true if this IP address is not euqal to the right hand side argument
     * @param rhs the IP address to which this IP address is compared
     * @return true if this IP address is numerically not equal to the right hand side
     * argument, false otherwise
     */
    constexpr bool operator != (IPv4Address rhs) const noexcept {
        return addr_ != rhs.addr_;
    }

    /*!
     * Performs the bitwise and operation on the values of this IP address and the right
     * hand side argument and returns the resulting value as an IP address.
     *
     * @param rhs the right hand side IP address
     * @return an IP address whose value is equal to the result of the bitwise and
     * operation applied to the values of this IP address and the right hand side one
     */
    constexpr IPv4Address operator& (IPv4Address rhs) const noexcept {
        return IPv4Address(addr_ & rhs.addr_);
    }

    /*!
     * Performs the bitwise exclusive or operation on the values of this IP address and
     * the right hand side argument and returns the resulting value as an IP address.
     *
     * @param rhs the right hand side IP address
     * @return an IP address whose value is equal to the result of the bitwise exclusive
     * or operation applied to the values of this IP address and the right hand side one
     */
    constexpr IPv4Address operator^ (IPv4Address rhs) const noexcept {
        return IPv4Address(addr_ ^ rhs.addr_);
    }

    /*!
     * Performs the bitwise or operation on the values of this IP address and the right
     * hand side argument and returns the resulting value as an IP address.
     *
     * @param rhs the right hand side IP address
     * @return an IP address whose value is equal to the result of the bitwise or
     * operation applied to the values of this IP address and the right hand side one
     */
    constexpr IPv4Address operator| (IPv4Address rhs) const noexcept {
        return IPv4Address(addr_ | rhs.addr_);
    }

    /*!
     * Returns an IP address whose value is equal to the bitwise inverted value of this
     * IP address
     * @return an IP address whose value is equal to the bitwise inverted value of this
     * IP address
     */
    constexpr IPv4Address operator~ () const noexcept {
        return IPv4Address{~addr_};
    }

    /*!
     * Returns the number of characters required to format this address in
     * the dotted decimal notation.
     *
     * @return the number of characters required to format this address in
     * the dotted decimal notation
     */
    [[nodiscard]]
    constexpr size_t charlen() const noexcept {
        return octlen<1>() + octlen<2>() + octlen<3>() + octlen<4>() + 3u;
    }

private:
    template <size_t N>
    [[nodiscard]]
    constexpr size_t octlen() const noexcept {
        uint8_t oct;
        if constexpr (N == 1) oct = oct1();
        else if constexpr (N == 2) oct = oct2();
        else if constexpr (N == 3) oct = oct3();
        else if constexpr (N == 4) oct = oct4();
        else oct = 0;
        if (oct < 10) return 1;
        if (oct < 100) return 2;
        return 3;
    }

private:
    uint32_t addr_;
};

} // namespace pimc::net

namespace std {

template <> struct hash<pimc::net::IPv4Address> {
    std::size_t operator() (
            pimc::net::IPv4Address address) const noexcept{
        return std::hash<uint32_t>{}(address.value());
    }
};

} // namespace std

