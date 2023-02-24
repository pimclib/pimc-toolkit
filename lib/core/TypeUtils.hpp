#pragma once

#include <cstdint>
#include <type_traits>

namespace pimc {

/*!
 * A meta-function which checks if the specified type is one of the types
 * in the specified template variable argument list.
 *
 * @tparam T the type to check
 * @tparam Ts the types against which to check
 */
template <typename T, typename ... Ts>
struct IsOneOf: std::false_type {};

template <typename T, typename ... Ts>
struct IsOneOf<T, T, Ts...>: std::true_type {};

template <typename T, typename U, typename ... Ts>
struct IsOneOf<T, U, Ts...>: IsOneOf<T, Ts...> {};

/*!
 * A meta-function which returns true if the specified type is one of the
 * types in the specified template variable argument list, false otherwise.
 *
 * @tparam T the type to check
 * @tparam Ts the types against which to check
 */
template <typename T, typename ... Ts>
inline constexpr bool IsOneOf_v = IsOneOf<T, Ts...>::value;

/*!
 * A concept which returns true if the specified type is one of the types
 * in the type list.
 *
 * @tparam T the type to check
 * @tparam Ts the list of types against which the the is checked
 */
template <typename T, typename ... Ts>
concept OneOf = IsOneOf<T, Ts...>::value;

/*!
 * A metafunction which returns true if the specified type is one of the
 * unsigned integer types, false otherwise.
 *
 * @tparam Unsigned the type to check
 */
template <typename Unsigned>
struct IsUInt: IsOneOf<Unsigned, uint8_t, uint16_t, uint32_t, uint64_t, unsigned> {};

/*!
 * A metafunction which returns true if the specified type is one of the
 * unsigned integer types, false otherwise.
 *
 * @tparam Unsigned the type to check
 */
template <typename Unsigned>
inline constexpr bool IsUInt_v = IsUInt<Unsigned>::value;

/*!
 * A concept which returns true if the specified type is one of the unsigned
 * integer types.
 *
 * @tparam Unsigned the type to check
 */
template <typename Unsigned>
concept UInt = IsUInt<Unsigned>::value;

/*!
 * A metafunction which returns true if the specified type is one of the
 * signed integer types.
 *
 * @tparam Integer the type to check
 */
template <typename Integer>
struct IsSInt: IsOneOf<Integer, int8_t, int16_t, int32_t, int64_t, int> {};

/*!
 * A metafunction which returns true if the specified type is one of the
 * signed integer types.
 *
 * @tparam Integer the type to check
 */
template <typename Integer>
inline constexpr bool IsSInt_v = IsSInt<Integer>::value;

/*!
 * A concept which returns true if the specified type is one of the signed
 * integer types.
 *
 * @tparam Integer the type to check
 */
template <typename Integer>
concept SInt = IsSInt<Integer>::value;

/*!
 * A concept which indicates if all of the types are destructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllDestructible =
        (std::is_destructible_v<Ts> and ...);

/*!
 * A concept which indicates if all of the types are trivially destructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllTriviallyDestructible =
        (std::is_trivially_destructible_v<Ts> and ...);

/*!
 * A concept that indicates that all types are copy constructible but at least
 * one is non-trivially copy constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotTriviallyDestructible =
        (std::is_destructible_v<Ts> and ...) and
        ((not std::is_trivially_destructible_v<Ts>) or ...);

/*!
 * A concept which indicates that at least one of the types is not copy
 * constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotDestructible =
        ((not std::is_destructible_v<Ts>) or ...);

/*!
 * A concept which indicates if all of the types are copy constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllCopyConstructible =
        (std::is_copy_constructible_v<Ts> and ...);

/*!
 * A concept which indicates if all of the types are trivially copy constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllTriviallyCopyConstructible =
        (std::is_trivially_copy_constructible_v<Ts> and ...);

/*!
 * A concept that indicates that all types are copy constructible but at least
 * one is non-trivially copy constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotTriviallyCopyConstructible =
        (std::is_copy_constructible_v<Ts> and ...) and
        ((not std::is_trivially_copy_constructible_v<Ts>) or ...);

/*!
 * A concept which indicates that at least one of the types is not copy
 * constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotCopyConstructible =
        ((not std::is_copy_constructible_v<Ts>) or ...);

/*!
 * A concept which indicates if all of the types are move constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllMoveConstructible =
        (std::is_move_constructible_v<Ts> and ...);

/*!
 * A concept which indicates if all of the types are trivially move constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllTriviallyMoveConstructible =
        (std::is_trivially_move_constructible_v<Ts> and ...);

/*!
 * A concept that indicates that all types are move constructible but at least
 * one is non-trivially move constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotTriviallyMoveConstructible =
        (std::is_move_constructible_v<Ts> and ...) and
        ((not std::is_trivially_move_constructible_v<Ts>) or ...);

/*!
 * A concept which indicates that at least one of the types is not move
 * constructible.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotMoveConstructible =
        ((not std::is_move_constructible_v<Ts>) or ...);

/*!
 * A concept which indicates if all of the types are trivially copy assignable.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllTriviallyCopyAssignable =
        (std::is_trivially_copy_assignable_v<Ts> and ...);

/*!
 * A concept that indicates that all types are copy assignable but at least
 * one is non-trivially copy assignable.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotTriviallyCopyAssignable =
(std::is_copy_assignable_v<Ts> and ...) and
        ((not std::is_trivially_copy_assignable_v<Ts>) or ...);

/*!
 * A concept which indicates that at least one of the types is not copy
 * assignable.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotCopyAssignable =
        ((not std::is_copy_assignable_v<Ts>) or ...);

/*!
 * A concept which indicates if all of the types are trivially move assignable.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept AllTriviallyMoveAssignable =
        (std::is_trivially_move_assignable_v<Ts> and ...);

/*!
 * A concept that indicates that all types are move assignable but at least
 * one is non-trivially move assignable.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotTriviallyMoveAssignable =
        (std::is_move_assignable_v<Ts> and ...) and
        ((not std::is_trivially_move_assignable_v<Ts>) or ...);

/*!
 * A concept which indicates that at least one of the types is not move
 * assignable.
 *
 * @tparam Ts the types to check
 */
template <typename ... Ts>
concept SomeNotMoveAssignable =
        ((not std::is_move_assignable_v<Ts>) or ...);

} // namespace pimc
