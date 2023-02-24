#pragma once

#include <string>
#include <string_view>

namespace pimc {

/*!
 * \brief A utility class that represents a text line made of a single
 * character.
 *
 * This object is useful as separator line or indent among other things.
 *
 * @tparam C the character of the line
 */
template <char C>
class SCLine final {
public:
    /*!
     * Constructs a text line of size \p sz
     * @param sz the length of the text line
     */
    explicit SCLine(size_t sz): s_(sz, C), sv_{s_} {}

    /*!
     * Returns a string view of the line.
     *
     * @return a string view of the line
     */
    auto operator() () const -> std::string_view { return sv_; }

    /*!
     * \brief Returns a section of the line of the size \p sz.
     *
     * \note If \p sz is larger than the length of the line, the returned
     * string view will have the same length as the line.
     *
     * @param sz the size of a section of the line
     * @return a section of the line of size \p sz
     */
    auto operator() (size_t sz) const -> std::string_view {
        return sv_.substr(0, sz);
    }

private:
    std::string s_;
    std::string_view sv_;
};

} // namespace pimc