#pragma once

#include <cstdint>
#include <memory>

namespace pimc {

/*!
 * An object that keeps track of the location of an item in the
 * source file. The location is the filename and the line where
 * the object appears.
 */
class Location final {
public:

    /*!
     * \brief A factory method which creates an initial location,
     * which contains the filename and -1 for the line.
     *
     * @tparam T the type of object from which a std::string can
     * be constructed
     * @param v the name of the source file
     * @return a Location with the specified source and the line
     * equal to -1
     */
    template <typename T>
    requires requires(T&& v) { std::string{std::forward<T>(v)}; }
    inline static Location forSource(T&& v) {
        return Location{std::make_shared<std::string>(std::forward<T>, -1l)};
    }

    /*!
     * \brief Returns the name of the source
     * @return the name of the source
     */
    [[nodiscard]]
    std::string const& source() const { return *source_; }

    /*!
     * \brief Returns the line of the source
     * @return the line of the source
     */
    [[nodiscard]]
    int64_t line() const { return line_; }

    /*!
     * \brief Creates a new location with the same source file name and
     * the specified \p line
     *
     * @param line the line number in the source
     * @return a new Location with the same source file name and the
     * specified \p line
     */
    Location at(int64_t line) {
        return Location{source_, line};
    }
private:
    Location(std::shared_ptr<std::string>& source, int64_t line)
    : source_{source}, line_{line} {}
private:
    std::shared_ptr<std::string> source_;
    int64_t line_;
};

} // namespace pimc