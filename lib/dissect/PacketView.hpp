#pragma once

#include <cstdint>
#include <concepts>
#include <functional>

namespace pimc {

/*!
 * An object that provides an incremental *view* over a raw packet data.
 */
class PacketView final {
public:
    /*!
     * Constructs a view over the data passed as the pointer \p data
     * and whose size is \p len.
     *
     * @param data the pointer to the start of the packet data
     * @param len the size of the packet data
     */
    constexpr PacketView(uint8_t const* data, std::size_t len)
    : data_{data}, len_{len}, taken_{0u} {}

    /*!
     * Attempts to measure the \p sz bytes at the current position in
     * the view and if there is enough data remaining calls the \p viewer
     * passing the pointer to the data to it and returns `true`. If there
     * isn't enough data remaining, this function does not call the
     * \p viewer and it returns `false`.
     *
     * @tparam Viewer the type of the viewer function
     * @param sz the desired size to data at the current position to view
     * @param viewer the function the performs the viewing
     * @return `true` if there is enough data and thus the viewer was called,
     * `false` otherwise
     */
    template <typename Viewer>
    requires std::regular_invocable<Viewer, void const*>
    bool take(std::size_t sz, Viewer&& viewer) {
        if (taken_ + sz <= len_) {
            std::invoke(
                    std::forward<Viewer>(viewer),
                    static_cast<void const*>(data_ + taken_));
            taken_ += sz;
            return true;
        }

        return false;
    }

    /*!
     * Skips the specified number of bytes if there is enough remaining
     * bytes in the view and return true, otherwise return false.
     *
     * @param sz the desired number of bytes to skip
     * @return `true` if there is enough data, `false` otherwise
     */
    bool skip(std::size_t sz) {
        if (taken_ + sz <= len_) {
            taken_ += sz;
            return true;
        }
        return false;
    }

    /*!
     * Returns the original size of the data.
     *
     * @return the original size of the data
     */
    [[nodiscard]]
    std::size_t len() const { return len_; }

    /*!
     * Returns the currently taken size of the data.
     *
     * @return the currently taken size of the data
     */
    [[nodiscard]]
    std::size_t taken() const { return taken_; }

    /*!
     * Returns the size of the remaining data.
     *
     * @return the size of the remaining data
     */
    [[nodiscard]]
    std::size_t remaining() const { return len_ - taken_; }

private:
    uint8_t const* data_;
    std::size_t len_;
    std::size_t taken_;
};

/*!
 * \brief An object that provides an incremental reverse *view* over a
 * raw packet data.
 *
 * This views walks the packet data backwards, assuming that the pointer
 * to the data points at the byte past the last byte of the data.
 */
class ReversePacketView final {
public:
    /*!
     * Constructs a view over the data passed as the pointer \p data
     * and whose size is \p len.
     *
     * @param data the pointer to the start of the packet data
     * @param len the size of the packet data
     */
    constexpr ReversePacketView(uint8_t const* data, std::size_t len)
            : data_{data}, len_{len}, taken_{0u} {}

    /*!
     * Attempts to measure the \p sz bytes at the current position in
     * the view and if there is enough data remaining calls the \p viewer
     * passing the pointer to the data to it and returns `true`. If there
     * isn't enough data remaining, this function does not call the
     * \p viewer and it returns `false`.
     *
     * @tparam Viewer the type of the viewer function
     * @param sz the desired size to data at the current position to view
     * @param viewer the function the performs the viewing
     * @return `true` if there is enough data and thus the viewer was called,
     * `false` otherwise
     */
    template <typename Viewer>
    requires std::regular_invocable<Viewer, void const*>
    bool take(std::size_t sz, Viewer&& viewer) {
        if (taken_ + sz <= len_) {
            std::invoke(
                    std::forward<Viewer>(viewer),
                    static_cast<void const*>(data_ - taken_ - sz));
            taken_ += sz;
            return true;
        }

        return false;
    }

    /*!
     * Skips the specified number of bytes if there is enough remaining
     * bytes in the view and return true, otherwise return false.
     *
     * @param sz the desired number of bytes to skip
     * @return `true` if there is enough data, `false` otherwise
     */
    bool skip(std::size_t sz) {
        if (taken_ + sz <= len_) {
            taken_ += sz;
            return true;
        }
        return false;
    }

    /*!
     * Returns the original size of the data.
     *
     * @return the original size of the data
     */
    [[nodiscard]]
    std::size_t len() const { return len_; }

    /*!
     * Returns the currently taken size of the data.
     *
     * @return the currently taken size of the data
     */
    [[nodiscard]]
    std::size_t taken() const { return taken_; }

    /*!
     * Returns the size of the remaining data.
     *
     * @return the size of the remaining data
     */
    [[nodiscard]]
    std::size_t remaining() const { return len_ - taken_; }

private:
    uint8_t const* data_;
    std::size_t len_;
    std::size_t taken_;
};

} // namespace pimc
