#pragma once

#include <cstdint>

namespace pimc {

/**
 * \brief Compute the 16-bit unsigned IP checksum over the data pointed
 * to by \p pktData.
 *
 * \note The returned value is in the network byte order.
 *
 * @param pktData a pointer to packet data
 * @param length the length of the data
 * @return a 16-bit unsigned IP checksum value in the network byte order
 */
uint16_t ipChecksumNs(void const* pktData, std::size_t length);

} // namespace pimc
