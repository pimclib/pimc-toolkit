#pragma once

#include <type_traits>
#include <tuple>

namespace pimc {

template <typename>
struct IsStdTuple: std::false_type {};

template <typename ... Ts>
struct IsStdTuple<std::tuple<Ts...>>: std::true_type {};

/*!
 * A concept which returns true if it's argument is std::tuple, false
 * otherwise.
 *
 * @tparam T the type to check if it's the std::tuple
 */
template <typename T>
concept StdTuple = IsStdTuple<T>::value;

} // namespace pimc
