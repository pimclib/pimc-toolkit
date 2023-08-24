#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

namespace pimc {

uint16_t ipChecksum(void *pktData, size_t length) {
    auto data = static_cast<char *>(pktData);
    uint64_t acc = 0xffff;

    // Handle any partial 32-bit word at the start of the data
    unsigned long offset = reinterpret_cast<unsigned long>(data) & 3ul;
    if (offset != 0) {
        size_t count = 4 - offset;
        if (count > length) count = length;
        uint32_t word = 0;
        memcpy(reinterpret_cast<char *>(&word) + offset, data, count);
        acc += ntohl(word);
        data += count;
        length -= count;
    }

    // Handle any complete 32-bit words
    char *end = data + (length & ~3ul);
    while (data != end) {
        uint32_t word;
        memcpy(&word, data, 4);
        acc += ntohl(word);
        data += 4;
    }

    // Handle any partial 32-bit word at the end of the data
    length &= 3;
    if (length != 0) {
        uint32_t word = 0;
        memcpy(&word, data, length);
        acc += ntohl(word);
    }

    // Handle deferred carries
    acc = (acc & 0xffffffff) + (acc >> 32);
    while (acc >> 16)
        acc = (acc & 0xffff) + (acc >> 16);

    // If the data began at an odd byte address then reverse the byte order
    if (offset & 1)
        acc = ((acc & 0xff00) >> 8) | ((acc & 0x00ff) << 8);

    return htons(static_cast<uint16_t>(~acc));
}

} // namespace pimc
