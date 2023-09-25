#pragma once

#include <cstddef>
#include <cstdint>

#include "pimc/core/CompilerUtils.hpp"

namespace pimc {

template <typename PW>
concept PacketWriterObject = requires(PW pw, void* pktData) {
    PW{pktData};
};

/*!
 * An object that facilitates writing structured packet data
 * to a contiguous region of memory.
 */
class PacketWriter final {
    template <typename T>
    friend constexpr auto next(PacketWriter& pw) -> T*;

    template <PacketWriterObject T>
    friend constexpr auto next(PacketWriter& pw, size_t sz) -> T;

public:
    /*!
     * \brief Constructs a PacketWriter object for the memory region
     * starting at \p m
     * @param m the start of packet memory region
     */
    constexpr explicit PacketWriter(void* m)
            : start_{static_cast<uint8_t*>(m)}
            , end_{static_cast<uint8_t*>(m)} {}

    /*!
     * \brief Returns the size of the data written to the packet memory.
     *
     * @return the size of the accumulated data
     */
    [[nodiscard]]
    constexpr size_t size() const {
        return static_cast<size_t>(end_ - start_);
    }

    /*!
     * \brief Returns a new PacketWriter whose start is at the
     * current position in the data.
     *
     * @return a new PacketWriter for the current position
     */
    [[nodiscard]]
    PacketWriter mark() {
        return PacketWriter(end_);
    }

    /*!
     * \brief Returns a pointer to the start of the data.
     *
     * @return a pointer to the start of the data
     */
    [[nodiscard]]
    void const* data() const {
        return static_cast<void const*>(start_);
    }

private:
    uint8_t* start_;
    uint8_t* end_;
};

/*!
 * Allocates the object of the specified type T in the memory
 * of the packet and returns a pointer to the object of this type.
 *
 * @tparam T the type of the object to write to the packet memory
 * @param pw the PacketWriter into which the object is being written
 * @return a pointer of type T
 */
template <typename T>
[[nodiscard]]
PIMC_ALWAYS_INLINE
constexpr auto next(PacketWriter& pw) -> T* {
    auto* p = static_cast<void*>(pw.end_);
    pw.end_ += sizeof(T);
    return static_cast<T*>(p);
}

/*!
 * Allocates the object of the specified type T and size in the memory
 * of the packet and returns a pointer to the object of this type.
 *
 * @tparam T an object that satisfies the PacketWriterObject concept
 * @param pw the PacketWriter into which the object is being written
 * @param sz the size of the allocated object
 * @return a pointer of type T
 */
template <PacketWriterObject PWO>
[[nodiscard]]
PIMC_ALWAYS_INLINE
constexpr auto next(PacketWriter& pw, size_t sz) -> PWO {
    auto* p = pw.end_;
    pw.end_ += sz;
    return PWO{static_cast<void*>(p)};
}

} // namespace pimc
