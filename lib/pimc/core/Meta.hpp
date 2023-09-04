#pragma once

#include <cstddef>

namespace pimc {

/*!
 * \brief A metafunction which returns the specified type
 * from the type list.
 *
 * @tparam Idx the index of the type to return
 * @tparam Ts the type list
 */
template <size_t Idx, typename ... Ts>
struct TypeAt {};

template <size_t Idx, typename T, typename ... Ts>
struct TypeAt<Idx, T, Ts...> {
    using type = typename TypeAt<Idx - 1, Ts...>::type;
};

template <typename T, typename ... Ts>
struct TypeAt<0, T, Ts...> {
    using type = T;
};

template <size_t Idx, typename ... Ts>
using TypeAt_t = typename TypeAt<Idx, Ts...>::type;

} // namespace pimc