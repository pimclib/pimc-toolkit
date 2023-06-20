#pragma once

#include <cstdint>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

#ifdef __linux__
#include <linux/ip.h>

/*!
 * IPv4 Header View.
 *
 * See <a href="https://www.rfc-editor.org/rfc/rfc791#section-3.1">Internet Header Format</a>
 */
class IPv4HdrView final {
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
    constexpr explicit IPv4HdrView(): ipHdr_{nullptr} {}

    /*!
     * \brief Construct a header view starting at the address \p p.
     *
     * @param p the address of the IPv4 header to view
     */
    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrView(void const* p)
    : ipHdr_{static_cast<iphdr const*>(p)} {}

    /*!
     * \brief Assign the address \p p to the view.
     *
     * @param p the address of the IPv4 header to view
     *
     * @return a reference to this view
     */
    PIMC_ALWAYS_INLINE
    constexpr auto operator= (void const* p) -> IPv4HdrView& {
        ipHdr_ = static_cast<iphdr const*>(p);
        return *this;
    }

    /*!
     * \brief Returns the effective size of IPv4 header in 32-bit words.
     *
     * If the header contains any options, they are taken into account in
     * this value. The minimum correct value of this field is 5.
     *
     * @return the size of the IPv4 header in 32-bit words
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ihl() const { return ipHdr_->ihl; }

    /*!
     * \brief Returns the effective size of IPv4 header in bytes.
     *
     * If the header* contains any options, they are taken into account in
     * this value.
     *
     * \note The returned value is computed from IHL and it is in the
     * host byte order.
     *
     * @return the size of the IPv4 header in bytes
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t headerSizeBytes() const {
        return static_cast<uint16_t>(static_cast<uint16_t>(ihl()) << 2u);
    }

    /*!
     * \brief Returns the IPv4 version number value.
     *
     * If the pointer assigned to this view points to a valid IPv4 header,
     * the returned value will always be 4. If the returned value is not 4,
     * that means the pointer does not point at a valid header.
     *
     * @return the IPv4 version number value
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t version() const { return ipHdr_->version; }

    /*!
     * \brief Returns the TOS value.
     *
     * @return the TOS value
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t tos() const { return ipHdr_->tos; }

    /*!
     * \brief Returns the total length of the IP datagram including the
     * payload in bytes.
     *
     * \note The returned value is in the network byte order
     *
     * @return the size of the IP datagram
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t totalLen() const { return ipHdr_->tot_len; }

    /*!
     * \brief Returns the ID field.
     *
     * @return the ID field
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t id() const { return ipHdr_->id; }

    /*!
     * \brief Returns a byte containing the flags and the fragmentation
     * offset.
     *
     * \note The returned value is in the network byte order.
     *
     * @return a byte containing the flags and the fragmentation offset
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t flagsAndFragOff() const { return ipHdr_->frag_off; }

    /*!
     * \brief Returns the TTL of the IP datagram.
     *
     * @return the TTL of the IP datagram
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ttl() const { return ipHdr_->ttl; }

    /*!
     * \brief Returns the number of the protocol whose data is in the IP
     * datagram payload.
     *
     * @return the protocol number
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t protocol() const { return ipHdr_->protocol; }

    /*!
     * \brief Returns the header checksum.
     *
     * \note The returned value is in the network byte order.
     *
     * @return the header checksum
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t hdrChecksum() const { return ipHdr_->check; }

    /*!
     * \brief Returns the source IP address.
     *
     * \note The returned value is in the network byte order.
     *
     * @return the source IP address
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t saddr() const { return ipHdr_->saddr; }

    /*!
     * \brief Returns the destination IP address.
     *
     * \note The returned value is in the network byte order.
     *
     * @return the destination IP address.
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t daddr() const { return ipHdr_->daddr; }

private:
    iphdr const* ipHdr_;
};


#endif

#ifdef __APPLE__

#include <netinet/ip.h>

/*!
 * IPv4 Header View.
 *
 * See <a href="https://www.rfc-editor.org/rfc/rfc791#section-3.1">Internet Header Format</a>
 */
class IPv4HdrView final {
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
    constexpr explicit IPv4HdrView() : ipHdr_{nullptr} {}

    /*!
     * \brief Construct a header view starting at the address \p p.
     *
     * @param p the address of the IPv4 header to view
     */
    PIMC_ALWAYS_INLINE
    constexpr explicit IPv4HdrView(void const *p)
    : ipHdr_{static_cast<ip const *>(p)} {}

    /*!
     * \brief Assign the address \p p to the view.
     *
     * @param p the address of the IPv4 header to view
     *
     * @return a reference to this view
     */
    PIMC_ALWAYS_INLINE
    constexpr auto operator=(void const *p) -> IPv4HdrView & {
        ipHdr_ = static_cast<ip const *>(p);
        return *this;
    }

    /*!
     * \brief Returns the effective size of IPv4 header in 32-bit words.
     *
     * If the header contains any options, they are taken into account in
     * this value. The minimum correct value of this field is 5.
     *
     * @return the size of the IPv4 header in 32-bit words
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ihl() const { return ipHdr_->ip_hl; }

    /*!
     * \brief Returns the effective size of IPv4 header in bytes.
     *
     * If the header* contains any options, they are taken into account in
     * this value.
     *
     * \note The returned value is computed from IHL and it is in the
     * host byte order.
     *
     * @return the size of the IPv4 header in bytes
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t headerSizeBytes() const {
        return static_cast<uint16_t>(static_cast<uint16_t>(ihl()) << 2u);
    }

    /*!
     * \brief Returns the IPv4 version number value.
     *
     * If the pointer assigned to this view points to a valid IPv4 header,
     * the returned value will always be 4. If the returned value is not 4,
     * that means the pointer does not point at a valid header.
     *
     * @return the IPv4 version number value
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t version() const { return ipHdr_->ip_v; }

    /*!
     * \brief Returns the TOS value.
     *
     * @return the TOS value
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t tos() const { return ipHdr_->ip_tos; }

    /*!
     * \brief Returns the total length of the IP datagram including the
     * payload in bytes.
     *
     * \note The returned value is in the network byte order
     *
     * @return the size of the IP datagram
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t totalLen() const { return ipHdr_->ip_len; }

    /*!
     * \brief Returns the ID field.
     *
     * @return the ID field
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t id() const { return ipHdr_->ip_id; }

    /*!
     * \brief Returns a byte containing the flags and the fragmentation
     * offset.
     *
     * \note The returned value is in the network byte order.
     *
     * @return a byte containing the flags and the fragmentation offset
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t flagsAndFragOff() const { return ipHdr_->ip_off; }

    /*!
     * \brief Returns the TTL of the IP datagram.
     *
     * @return the TTL of the IP datagram
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t ttl() const { return ipHdr_->ip_ttl; }

    /*!
     * \brief Returns the number of the protocol whose data is in the IP
     * datagram payload.
     *
     * @return the protocol number
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint8_t protocol() const { return ipHdr_->ip_p; }

    /*!
     * \brief Returns the header checksum.
     *
     * \note The returned value is in the network byte order.
     *
     * @return the header checksum
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint16_t hdrChecksum() const { return ipHdr_->ip_sum; }

    /*!
     * \brief Returns the source IP address.
     *
     * \note The returned value is in the network byte order.
     *
     * @return the source IP address
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t saddr() const { return ipHdr_->ip_src.s_addr; }

    /*!
     * \brief Returns the destination IP address.
     *
     * \note The returned value is in the network byte order.
     *
     * @return the destination IP address.
     */
    [[nodiscard]]
    PIMC_ALWAYS_INLINE
    uint32_t daddr() const { return ipHdr_->ip_dst.s_addr; }

private:
    ip const *ipHdr_;
};

#endif

} // namespace pimc
