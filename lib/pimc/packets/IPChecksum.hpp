#pragma once

#include <cstdint>

namespace pimc {

/**
 * \brief Compute the 16-bit unsigned IP checksum over the data pointed
 * to by \p pktData.
 *
 * @param pktData a pointer to packet data
 * @param length the length of the data
 * @return a 16-bit unsigned IP checksum
 */
uint16_t ipChecksum(void *pktData, std::size_t length);

} // namespace pimc
