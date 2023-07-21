#pragma once

#include <cstddef>
#include <cstdint>

namespace pimc {

/*!
 * An object that facilitates writing structured packet data
 * to a contiguous region of memory.
 */
class PacketWriter final {
    template <typename T>
    friend constexpr auto next(PacketWriter& pw) -> T*;
public:
    /*!
     * Constructs a PacketWriter object for the memory region starting
     * at \p m
     * @param m the start of packet memory region
     */
    constexpr explicit PacketWriter(void* m)
            : start_{static_cast<uint8_t*>(m)}
            , end_{static_cast<uint8_t*>(m)} {}

    /*!
     * Returns the size of the data written to the packet memory.
     *
     * @return the size of the accumulated data
     */
    [[nodiscard]]
    constexpr size_t size() const {
        return static_cast<size_t>(end_ - start_);
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
constexpr auto next(PacketWriter& pw) -> T* {
    auto* p = static_cast<void*>(pw.end_);
    pw.end_ += sizeof(T);
    return static_cast<T*>(p);
}

} // namespace pimc
