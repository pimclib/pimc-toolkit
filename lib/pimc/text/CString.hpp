#pragma once

#include <ranges>

namespace pimc {

/*!
 * A sentinel for a 0-terminated strings.
 */
struct cssentinel {};

inline bool operator== (char const* p, cssentinel) {
    return *p == static_cast<char>(0);
}

inline bool operator== (wchar_t const* p, cssentinel) {
    return *p == static_cast<wchar_t>(0);
}

namespace views {

template <typename Char>
class basic_cstr: public std::ranges::view_interface<basic_cstr<Char>> {
public:
    explicit constexpr basic_cstr(Char const* p): p_{p} {}
    constexpr auto begin() const { return p_; }
    constexpr auto end() const { return cssentinel{}; }
private:
    Char const* p_;
};

using cstr = basic_cstr<char>;

} // namespace view

} // namespace pimc
