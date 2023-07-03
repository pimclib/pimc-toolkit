#pragma once

#include <type_traits>

namespace pimc {

template <typename T>
struct IsReferenceWrapper : std::false_type {};

template <typename U>
struct IsReferenceWrapper<std::reference_wrapper<U>> : std::true_type {};

template <typename T>
constexpr bool IsReferenceWrapper_v = IsReferenceWrapper<T>::value;

/*!
 * The tag type to indicate that a value object should be constructed
 * in place.
 */
struct InPlaceValueType {
    explicit InPlaceValueType() = default;
};

/// An instance of the InPlaceValueType.
inline constexpr InPlaceValueType InPlaceValue{};

/*!
 * The tag type to indicate that an empty object should be constructed
 * in place.
 */
struct InPlaceEmptyType {
    explicit InPlaceEmptyType() = default;
};

/// An instance of the InPlaceEmptyType.
inline constexpr InPlaceEmptyType InPlaceEmpty{};

/*!
 * The tag type to indicate that an error object should be constructed
 * in place.
 */
struct InPlaceErrorType {
    explicit InPlaceErrorType() = default;
};

/// An instance of the InPlaceErrorType.
inline constexpr InPlaceErrorType InPlaceError{};

template <typename T>
using WrappedValueType = std::conditional_t<
        std::is_lvalue_reference_v<T>,
        std::reference_wrapper<std::remove_reference_t<T>>,
        std::remove_const_t<T>
>;

} // namespace pimc
