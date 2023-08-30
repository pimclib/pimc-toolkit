#pragma once

#include <cstdint>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

#ifdef __linux__
#include <linux/ip.h>

/*!
 * IPv4 Header View.
 *
 * See <a href="https://datatracker.ietf.org/doc/html/rfc791#section-3.1">Internet Header Format</a>
 */
class IPv4HdrWriter final {
public:
    /*!
     * The size of the IPv4 header without options in bytes.
     */
    inline static constexpr size_t HdrSize{sizeof(iphdr)};

    /*!
     * \brief Construct a header view pointing to the null pointer.
     *
     * \warning This view **must** be assigned a valid pointer prior to
     * being used.
     */
    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrWriter(): ipHdr_{nullptr} {}

    /*!
     * \brief Construct a header writer starting at the address \p p and
     * set the IP version to 2, and the header size to 5 (20 bytes, default
     * value).
     *
     * @param p the address of the IPv4 header to write to
     */
    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrWriter(void* p)
    : ipHdr_{static_cast<iphdr*>(p)} {
        ipHdr_->version = 2;
        ipHdr_->ihl = 5;
    }

    /*!
     * \brief Assign the address \p p to the view.
     *
     * @param p the address of the IPv4 header to write to
     *
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    constexpr auto operator= (void* p) -> IPv4HdrWriter& {
        ipHdr_ = static_cast<iphdr*>(p);
        return *this;
    }

    /*!
     * \brief Sets the effective size of IPv4 header in 32-bit words.
     *
     * \note The constructor sets this value to 5, i.e. 20 bytes, which is
     * the default IPv4 header size, that is the one that contains no
     * options.
     *
     * @param ihlv the header length in 32-bit words
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& ihl(uint8_t ihlv) {
        ipHdr_->ihl = 0xf & ihlv;
        return *this;
    }

    /*!
     * \brief Sets the TOS value.
     *
     * @param tosv the TOS value to set
     * @return a reference this this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& tos(uint8_t tosv) {
        ipHdr_->tos = tosv;
        return *this;
    }

    /*!
     * \brief Sets the total length of the IP datagram including the
     * payload in bytes.
     *
     * \note \p lenv must be in the network byte order
     *
     * @param lenv the length of the IP datagram
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& totalLen(uint16_t lenv) {
        ipHdr_->tot_len = lenv;
        return *this;
    }

    /*!
     * \brief Sets the ID field.
     *
     * \note \p idv must be in the network byte order
     *
     * @param idv the ID of the datagram
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& id(uint16_t idv) {
        ipHdr_->id = idv;
        return *this;
    }

    /*!
     * \brief Sets the 16-bit word in the header which contains the flags
     * and the fragmentation offset.
     *
     * \note \p ffoffv must be in the network byte order.
     *
     * @param ffoffv the 16-bit word containing the flags and fragment offset
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& flagsAndFragOff(uint16_t ffoffv) {
        ipHdr_->frag_off = ffoffv;
        return *this;
    }

    /*!
     * \brief Sets the TTL of the IP datagram.
     *
     * @param ttlv the TTL value of the datagram
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& ttl(uint8_t ttlv) {
        ipHdr_->ttl = ttlv;
        return *this;
    }

    /*!
     * \brief Sets the numeric value of the protocol whose data will reside
     * in the IP datagram payload.
     *
     * @param protocolv the numeric protovol value
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& protocol(uint8_t protocolv) {
        ipHdr_->protocol = protocolv;
        return *this;
    }

    /*!
     * \brief Sets the header checksum.
     *
     * \note \p hcsv must be in in the network byte order.
     *
     * @param hcsv the header checksum
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& hdrChecksum(uint16_t hcsv) {
        ipHdr_->check = hcsv;
        return *this;
    }

    /*!
     * \brief Sets the source IP address.
     *
     * \note \p saddrv must be in the network byte order.
     *
     * @param saddrv the source IP address
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& saddr(uint32_t saddrv) {
        ipHdr_->saddr = saddrv;
        return *this;
    }

    /*!
     * \brief Sets the destination IP address.
     *
     * \note \p daddrv must be in the network byte order.
     *
     * @param daddrv the destination IP address
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& daddr(uint32_t daddrv) {
        ipHdr_->daddr = daddrv;
        return *this;
    }

private:
    iphdr* ipHdr_;
};


#endif

#ifdef __APPLE__

#include <netinet/ip.h>

/*!
 * IPv4 Header View.
 *
 * See <a href="https://datatracker.ietf.org/doc/html/rfc791#section-3.1">Internet Header Format</a>
 */
class IPv4HdrWriter final {
public:
    /*!
     * The size of the IPv4 header without options in bytes.
     */
    inline static constexpr size_t HdrSize{sizeof(ip)};

    /*!
     * \brief Construct a header view pointing to the null pointer.
     *
     * \warning This view **must** be assigned a valid pointer prior to
     * being used.
     */
    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrWriter() : ipHdr_{nullptr} {}

    /*!
     * \brief Construct a header writer starting at the address \p p and
     * set the IP version to 2, and the header size to 5 (20 bytes, default
     * value).
     *
     * @param p the address of the IPv4 header to write to
     */
    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrWriter(void* p)
            : ipHdr_{static_cast<ip*>(p)} {
        ipHdr_->ip_v = 2u;
        ipHdr_->ip_hl = 5u;
    }

    /*!
     * \brief Assign the address \p p to the view.
     *
     * @param p the address of the IPv4 header to write to
     *
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    constexpr auto operator= (void* p) -> IPv4HdrWriter& {
        ipHdr_ = static_cast<ip*>(p);
        return *this;
    }

    /*!
     * \brief Sets the effective size of IPv4 header in 32-bit words.
     *
     * \note The constructor sets this value to 5, i.e. 20 bytes, which is
     * the default IPv4 header size, that is the one that contains no
     * options.
     *
     * @param ihlv the header length in 32-bit words
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& ihl(uint8_t ihlv) {
        ipHdr_->ip_v = 0xf & ihlv;
        return *this;
    }

    /*!
     * \brief Sets the TOS value.
     *
     * @param tosv the TOS value to set
     * @return a reference this this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& tos(uint8_t tosv) {
        ipHdr_->ip_tos = tosv;
        return *this;
    }

    /*!
     * \brief Sets the total length of the IP datagram including the
     * payload in bytes.
     *
     * \note \p lenv must be in the network byte order
     *
     * @param lenv the length of the IP datagram
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& totalLen(uint16_t lenv) {
        ipHdr_->ip_len = lenv;
        return *this;
    }

    /*!
     * \brief Sets the ID field.
     *
     * \note \p idv must be in the network byte order
     *
     * @param idv the ID of the datagram
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& id(uint16_t idv) {
        ipHdr_->ip_id = idv;
        return *this;
    }

    /*!
     * \brief Sets the 16-bit word in the header which contains the flags
     * and the fragmentation offset.
     *
     * \note \p ffoffv must be in the network byte order.
     *
     * @param ffoffv the 16-bit word containing the flags and fragment offset
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& flagsAndFragOff(uint16_t ffoffv) {
        ipHdr_->ip_off = ffoffv;
        return *this;
    }

    /*!
     * \brief Sets the TTL of the IP datagram.
     *
     * @param ttlv the TTL value of the datagram
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& ttl(uint8_t ttlv) {
        ipHdr_->ip_ttl = ttlv;
        return *this;
    }

    /*!
     * \brief Sets the numeric value of the protocol whose data will reside
     * in the IP datagram payload.
     *
     * @param protocolv the numeric protovol value
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& protocol(uint8_t protocolv) {
        ipHdr_->ip_p = protocolv;
        return *this;
    }

    /*!
     * \brief Sets the header checksum.
     *
     * \note \p hcsv must be in in the network byte order.
     *
     * @param hcsv the header checksum
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& hdrChecksum(uint16_t hcsv) {
        ipHdr_->ip_sum = hcsv;
        return *this;
    }

    /*!
     * \brief Sets the source IP address.
     *
     * \note \p saddrv must be in the network byte order.
     *
     * @param saddrv the source IP address
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& saddr(uint32_t saddrv) {
        ipHdr_->ip_src.s_addr = saddrv;
        return *this;
    }

    /*!
     * \brief Sets the destination IP address.
     *
     * \note \p daddrv must be in the network byte order.
     *
     * @param daddrv the destination IP address
     * @return a reference to this writer
     */
    PIMC_ALWAYS_INLINE
    IPv4HdrWriter& daddr(uint32_t daddrv) {
        ipHdr_->ip_dst.s_addr = daddrv;
        return *this;
    }


private:
    ip *ipHdr_;
};

#endif

} // namespace pimc
