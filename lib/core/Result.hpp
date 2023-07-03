#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include <type_traits>
#include <concepts>
#include <utility>
#include <functional>
#include <variant>

#include "pimc/core/TypeUtils.hpp"
#include "pimc/core/OptionalResultHelpers.hpp"

namespace pimc {

// Forward declarations:

template <typename>
class Failure;

template <typename, typename>
class Result;

// Traits:

template <typename T>
struct IsFailure: std::false_type {};

template <typename U>
struct IsFailure<Failure<U>>: std::true_type {};

template <typename T>
struct IsResult: std::false_type {
    using ValueType = T;
    using ErrorType = void;
};

template <typename T, typename E>
struct IsResult<Result<T, E>>: std::true_type {
    using ValueType = T;
    using ErrorType = E;
};

template <typename R>
concept ResultType = IsResult<R>::value and
        not std::same_as<typename IsResult<R>::ErrorType, void>;

template <typename R>
concept ResultTypeValueDefaultConstructible =
        IsResult<R>::value and
        (std::default_initializable<typename IsResult<R>::ValueType> or
         std::same_as<typename IsResult<R>::ValueType, void>) and
        not std::same_as<typename IsResult<R>::ErrorType, void>;

namespace detail {

template <typename EE, typename E>
concept FailureIsValueConstructible =
        std::constructible_from<E, EE&&> and
        not std::same_as<std::decay_t<EE>, std::in_place_t> and
        not std::same_as<std::decay_t<EE>, InPlaceValueType> and
        not std::same_as<std::decay_t<EE>, InPlaceErrorType> and
        not IsFailure<std::decay_t<EE>>::value and
        not IsResult<std::decay_t<EE>>::value;

template <typename EE, typename E>
concept FailureIsExplicitValueConstructible =
        FailureIsValueConstructible<EE, E> and
        not std::convertible_to<EE, E>;

template <typename EE, typename E>
concept FailureIsImplicitValueConstructible =
        FailureIsValueConstructible<EE, E> and
        std::convertible_to<EE, E>;

template <typename EE, typename E>
concept FailureIsValueAssignable =
        not IsResult<std::decay_t<EE>>::value and
        not IsFailure<std::decay_t<EE>>::value and
        std::assignable_from<WrappedValueType<E>&, EE>;

} // namespace detail

/*!
 * A helper object which cant be returned by a computation to produce
 * a Result in the error state with the error value contained in the
 * returned Failure object.
 *
 * @tparam E the type of the error
 */
template <typename E>
class Failure {
    static_assert(
            not IsResult<std::decay_t<E>>::value,
            "Possibly CV-qualified result E type "
            "may not be used as the type argument to Failure");

    static_assert(
            not IsFailure<std::decay_t<E>>::value,
            "Possibly CV-qualified failure E type "
            "may not be usd as the type argument to Failure");

    static_assert(
            not std::is_void_v<std::decay_t<E>>,
            "Possibly CV-qualified void E type "
            "may not be used as the type argument to Failure");

    static_assert(
            not std::is_rvalue_reference_v<E>,
            "rvalue references for E type "
            "may not be used as the type argument to Failure, "
            "only lvalue references are allowed");

public:
    /*!
     * Failure default constructor
     */
    Failure() = default;

    /*!
     * Constructs the failure object in place with the specified arguments.
     *
     * @tparam Ts the types of the arguments
     * @param args the types of the arguments to forward to the failure object
     */
    template <typename ... Ts>
    requires std::constructible_from<E, Ts...>
    constexpr Failure(std::in_place_t, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>)
    : error_{std::forward<Ts>(args)...} {}

    /*!
     * Constructs the failure object in place with the specified initializer
     * list and additional arguments (if any).
     *
     * @tparam U the type of the initializer list element
     * @tparam Ts the types of the additional arguments
     * @param il the initializer list
     * @param args the additional arguments
     */
    template <typename U, typename ... Ts>
    requires std::constructible_from<E, std::initializer_list<U>, Ts...>
    constexpr Failure(std::in_place_t, std::initializer_list<U> il, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>, Ts...>)
    : error_{il, std::forward<Ts>(args)...} {}

    /*!
     * Constructs a Failure object from the specified error.
     *
     * @tparam EE The type of error implicitly convertible to the error type
     * @param error the error
     */
    template <detail::FailureIsImplicitValueConstructible<E> EE>
    constexpr Failure(EE&& error)
    noexcept(std::is_nothrow_constructible_v<E, EE>)
    : error_{std::forward<EE>(error)} {}

    /*!
     * Constructs a Failure object from the specified error.
     *
     * @tparam EE The type of error explicitly convertible to the error type
     * @param error the error
     */
    template <detail::FailureIsExplicitValueConstructible<E> EE>
    constexpr explicit Failure(EE&& error)
    noexcept(std::is_nothrow_constructible_v<E, EE>)
    : error_{std::forward<EE>(error)} {}

    /*!
     * The default copy constructor
     */
    Failure(Failure const&) = default;

    /*!
     * The default move constructor
     */
    Failure(Failure&&) noexcept(std::is_nothrow_move_constructible_v<E>) = default;

    /*!
     * Constructs a Failure object by copying the error from \p other.
     *
     * @tparam EE the error type from which this error type con be constructed
     * @param other the Failure object from which to copy construct this one
     */
    template <typename EE>
    requires std::constructible_from<E, EE const&>
    constexpr Failure(Failure<EE> const& other)
    noexcept(std::is_nothrow_constructible_v<E, EE const&>)
    : error_{other.error()} {}

    /*!
     * Constructs a Failure object by move-converting \p other.
     *
     * @tparam EE the other Failure to move from
     * @param other the Failure object from which to move construct this one
     */
    template <typename EE>
    requires std::constructible_from<E, EE&&>
    constexpr Failure(Failure<EE>&& other)
    noexcept(std::is_nothrow_constructible_v<E, EE&&>)
    : error_{std::move(other).error()} {}

    /*!
     * Assigns the value of \p error to this Failure object.
     *
     * @tparam EE the type of the specified error object. It
     * must be assignable to the error type of this Failure object
     * @param error the error
     * @return a reference to this Failure object
     */
    template <detail::FailureIsValueAssignable<E> EE>
    constexpr auto operator= (EE&& error)
    noexcept(std::is_nothrow_assignable_v<E&, EE> or std::is_lvalue_reference_v<E>)
    -> Failure& {
        error_ = std::forward<EE>(error);
        return *this;
    }

    /*!
     * The default copy assignment operator.
     *
     * @return a reference to this Failure object
     */
    auto operator= (Failure const&) -> Failure& = default;

    /*!
     * The default move assignment operator.
     *
     * @return a reference to this Failure object
     */
    auto operator= (Failure&&) noexcept(std::is_nothrow_move_assignable_v<E>)
    -> Failure& = default;

    /*!
     * Assigns the error of \p other via copy.
     *
     * @tparam EE the type of the error contained in the other Failure
     * @param other the other Failure object
     * @return a reference to this Failure object
     */
    template <typename EE>
    requires std::assignable_from<E&, EE const&>
    constexpr auto operator= (Failure<EE> const& other)
    noexcept(std::is_nothrow_assignable_v<E&, EE const&>)
    -> Failure& {
        error_ = other.error();
        return *this;
    }

    /*!
     * Assigns the error of \p other via move.
     *
     * @tparam EE the type of the error contained in the other Failure
     * @param other the other Failure object
     * @return a reference to this object
     */
    template <typename EE>
    requires std::assignable_from<E&, EE&&>
    constexpr auto operator= (Failure<EE>&& other)
    noexcept(std::is_nothrow_assignable_v<E&, EE&&>)
    -> Failure& {
        error_ = std::move(other).error();
        return *this;
    }

    /*!
     * Returns the underlying error object.
     *
     * @return the underlying error object
     */
    constexpr auto error() & noexcept -> std::add_lvalue_reference_t<E> {
        return error_;
    }

    /*!
     * Returns the underlying error object.
     *
     * @return the underlying error object
     */
    constexpr auto error() && noexcept -> std::add_rvalue_reference_t<E> {
        return static_cast<std::add_rvalue_reference_t<E>>(error_);
    }

    /*!
     * Returns the underlying error object.
     *
     * @return the underlying error object
     */
    constexpr auto error() const & noexcept
        -> std::add_lvalue_reference_t<std::add_const_t<E>> {
        return error_;
    }

    /*!
     * Returns the underlying error object.
     *
     * @return the underlying error object
     */
    constexpr auto error() const && noexcept
        -> std::add_rvalue_reference_t<std::add_const_t<E>> {
        return static_cast<std::add_rvalue_reference_t<std::add_const_t<E>>>(error_);
    }

private:
    using ErrorType = WrappedValueType<E>;

private:
    ErrorType error_;
};

/*!
 * The equality relation between two failure object containing comparable
 * error objects.
 *
 * @tparam E1 the type of error contained in the first failure object
 * @tparam E2 the type of error contained in the second failure object
 * @param lhs the left failure
 * @param rhs the right failure
 * @return true if the two errors are equal, false otherwise
 */
template <typename E1, typename E2>
constexpr auto operator == (Failure<E1> const& lhs, Failure<E2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E1, E2> {
    return lhs.error() == rhs.error();
}

/*!
 * The inequality relation between two failure object containing comparable
 * error objects.
 *
 * @tparam E1 the type of error contained in the first failure object
 * @tparam E2 the type of error contained in the second failure object
 * @param lhs the left failure
 * @param rhs the right failure
 * @return true if the two errors are unequal, false otherwise
 */
template <typename E1, typename E2>
constexpr auto operator != (Failure<E1> const& lhs, Failure<E2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E1, E2> {
    return lhs.error() != rhs.error();
}

/*!
 * The less than relation between two failure object containing comparable
 * error objects.
 *
 * @tparam E1 the type of error contained in the first failure object
 * @tparam E2 the type of error contained in the second failure object
 * @param lhs the left failure
 * @param rhs the right failure
 * @return true if the error contained in the first failure object is
 * less than the error contained in the second one
 */
template <typename E1, typename E2>
constexpr auto operator < (Failure<E1> const& lhs, Failure<E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    return lhs.error() < rhs.error();
}

/*!
 * The less than or equal relation between two failure object containing
 * comparable error objects.
 *
 * @tparam E1 the type of error contained in the first failure object
 * @tparam E2 the type of error contained in the second failure object
 * @param lhs the left failure
 * @param rhs the right failure
 * @return true if the error contained in the first failure object is
 * less than or equalt to the error contained in the second one
 */
template <typename E1, typename E2>
constexpr auto operator <= (Failure<E1> const& lhs, Failure<E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    return lhs.error() <= rhs.error();
}

/*!
 * The greater than relation between two failure object containing comparable
 * error objects.
 *
 * @tparam E1 the type of error contained in the first failure object
 * @tparam E2 the type of error contained in the second failure object
 * @param lhs the left failure
 * @param rhs the right failure
 * @return true if the error contained in the first failure object is
 * greater than the error contained in the second one
 */
template <typename E1, typename E2>
constexpr auto operator > (Failure<E1> const& lhs, Failure<E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    return lhs.error() > rhs.error();
}

/*!
 * The greater than or equal relation between two failure object containing
 * comparable error objects.
 *
 * @tparam E1 the type of error contained in the first failure object
 * @tparam E2 the type of error contained in the second failure object
 * @param lhs the left failure
 * @param rhs the right failure
 * @return true if the error contained in the first failure object is
 * greater than or equal to the error contained in the second one
 */
template <typename E1, typename E2>
constexpr auto operator >= (Failure<E1> const& lhs, Failure<E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    return lhs.error() >= rhs.error();
}

/*!
 * A factory function which creates a new failure object by forwarding
 * the specified error.
 *
 * @tparam E the type of the error
 * @param e the forwarding reference to an error object
 * @return a failure object
 */
template <typename E>
[[nodiscard]]
inline constexpr auto fail(E&& e)
noexcept(std::is_nothrow_constructible_v<std::decay_t<E>, E>)
-> Failure<std::decay_t<E>> {
    return Failure<std::decay_t<E>>{std::forward<E>(e)};
}

/*!
 * A factory function which creates a failure from a reference_wrapper
 * over an error.
 *
 * @tparam E the type of the error
 * @param e the reference_wrapper over the error value
 * @return a failure containing an error reference
 */
template <typename E>
[[nodiscard]]
inline constexpr auto fail(std::reference_wrapper<E> e) noexcept -> Failure<E&> {
    return Failure<E&>{e.get()};
}

/*!
 * A factory function which constructs a failure object containing
 * the error constructed with the specified arguments.
 *
 * @tparam E the error type
 * @tparam Ts the types of arguments to the error constructor
 * @param args the arguments to the error constructor
 * @return a failure object containing the constructed error
 */
template <typename E, typename ... Ts>
requires std::constructible_from<E, Ts...>
[[nodiscard]]
constexpr auto fail(Ts&& ... args)
noexcept(std::is_nothrow_constructible_v<E, Ts...>)
-> Failure<E> {
    return Failure<E>{std::in_place, std::forward<Ts>(args)...};
}

/*!
 * A factory function which constructs a failure object containing
 * the error constructed from an initializer list followed by optional
 * arguments.
 *
 * @tparam E the error type
 * @tparam U the type of the elements of the initializer list
 * @tparam Ts the types of the optional arguments
 * @param il the initializer list
 * @param args the optional arguments
 * @return a failure object containing the constructed error
 */
template <typename E, typename U, typename ...Ts>
requires std::constructible_from<E, std::initializer_list<U>, Ts...>
[[nodiscard]]
constexpr auto fail(std::initializer_list<U> il, Ts&& ... args)
noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>, Ts...>)
-> Failure<E> {
    return Failure<E>{std::in_place, il, std::forward<Ts>(args)...};
}

/*!
 * Swaps the two failure values.
 *
 * @tparam E the type of the contained error
 * @param lhs the left failure
 * @param rhs the right failure
 */
template <typename E>
auto swap(Failure<E>& lhs, Failure<E>& rhs)
noexcept(std::is_nothrow_swappable_v<E>) -> void {
    using std::swap;
    swap(lhs.error(), rhs.error());
}

namespace detail {

template <typename V, typename E,
          bool TriviallyDestructible = std::is_trivially_destructible_v<V> and
                                       std::is_trivially_destructible_v<E>>
struct ResultAlt {
    using ValueType = WrappedValueType<V>;
    using ErrorType = WrappedValueType<E>;

    union {
        ValueType value_;
        ErrorType error_;
        std::monostate empty_;
    };

    bool hasValue_;

    explicit constexpr ResultAlt(std::monostate) noexcept
    : empty_{} {}

    template <typename ... Ts>
    constexpr ResultAlt(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<V, Ts...>)
    : value_{std::forward<Ts>(args)...}, hasValue_{true} {}

    template <typename ... Ts>
    constexpr ResultAlt(InPlaceErrorType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>)
    : error_{std::forward<Ts>(args)...}, hasValue_{false} {}

    ResultAlt(ResultAlt const&) = default;
    ResultAlt(ResultAlt&&) = default;

    auto destroy() const noexcept -> void {}

    auto operator= (ResultAlt const&) -> ResultAlt& = default;
    auto operator= (ResultAlt&&) -> ResultAlt& = default;
};

template <typename V, typename E>
struct ResultAlt<V, E, false> {
    using ValueType = WrappedValueType<V>;
    using ErrorType = WrappedValueType<E>;

    union {
        ValueType value_;
        ErrorType error_;
        std::monostate empty_;
    };

    bool hasValue_;

    explicit constexpr ResultAlt(std::monostate) noexcept: empty_{} {}

    template <typename ... Ts>
    constexpr ResultAlt(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<V, Ts...>)
    : value_{std::forward<Ts>(args)...}, hasValue_{true} {}

    template <typename ... Ts>
    constexpr ResultAlt(InPlaceErrorType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>)
    : error_{std::forward<Ts>(args)...}, hasValue_{false} {}

    ResultAlt(ResultAlt const&) = default;
    ResultAlt(ResultAlt&&) = default;

    auto destroy()
    noexcept(std::is_nothrow_destructible_v<V> and std::is_nothrow_destructible_v<E>)
    -> void {
        if (hasValue_) {
            value_.~ValueType();
            hasValue_ = false;
        }
        else error_.~ErrorType();
    }

    ~ResultAlt()
    noexcept(std::is_nothrow_destructible_v<V> and std::is_nothrow_destructible_v<E>) {
        destroy();
    }

    auto operator= (ResultAlt const&) -> ResultAlt& = default;
    auto operator= (ResultAlt&&) -> ResultAlt& = default;
};

template <typename T, typename E>
struct ResultStorage {
    using StorageType = ResultAlt<T, E>;
    StorageType storage_;

    explicit ResultStorage(std::monostate) noexcept
    : storage_{std::monostate{}} {}

    template <typename ... Ts>
    constexpr ResultStorage(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, Ts...>)
    : storage_{InPlaceValue, std::forward<Ts>(args)...} {}

    template <typename ... Ts>
    constexpr ResultStorage(InPlaceErrorType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>)
    : storage_{InPlaceError, std::forward<Ts>(args)...} {}

    /*
     * Internal use function. It assumes there is no contained value and error
     * at the time of construction.
     */
    template <typename ... Ts>
    auto constructValue(Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, Ts...>) -> void {
        using ValueType = typename StorageType::ValueType;
        auto* p = static_cast<void*>(std::addressof(storage_.value_));
        std::construct_at(reinterpret_cast<ValueType*>(p), std::forward<Ts>(args)...);
        storage_.hasValue_ = true;
    }

    /*
     * Internal use function. It assumes there is contained value and error
     * at the time of construction.
     */
    template <typename ... Ts>
    auto constructError(Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>) -> void {
        using ErrorType = typename StorageType::ErrorType;
        auto* p = static_cast<void*>(std::addressof(storage_.error_));
        std::construct_at(reinterpret_cast<ErrorType*>(p), std::forward<Ts>(args)...);
        storage_.hasValue_ = false;
    }

    template <typename R>
    auto constructErrorFromResult(R&& r) -> void {
        if (r.storage_.hasValue_) constructValue();
        else constructError(std::forward<R>(r).storage_.error_);
    }

    template <typename R>
    auto constructFromResult(R&& r) -> void {
        if (r.storage_.hasValue_) {
            using ValueType = typename StorageType::ValueType;
            auto* p = static_cast<void*>(std::addressof(storage_.value_));
            auto* vp = reinterpret_cast<ValueType*>(p);

            if constexpr (std::is_lvalue_reference_v<T>)
                std::construct_at(vp, std::forward<R>(r).storage_.value_.get());
            else std::construct_at(vp, std::forward<R>(r).storage_.value_);
            storage_.hasValue_ = true;
        } else {
            using ErrorType = typename StorageType::ErrorType ;
            auto* p = static_cast<void*>(std::addressof(storage_.error_));
            auto* ep = reinterpret_cast<ErrorType*>(p);

            if constexpr (std::is_lvalue_reference_v<E>)
                std::construct_at(ep, std::forward<R>(r).storage_.error_.get());
            else std::construct_at(ep, std::forward<R>(r).storage_.error_);
            storage_.hasValue_ = false;
        }
    }

    template <typename V>
    auto assignValue(V&& v)
    noexcept(std::is_nothrow_assignable_v<T&, V>) -> void {
        if (not storage_.hasValue_) {
            // This destroys the error
            storage_.destroy();
            constructValue(std::forward<V>(v));
        } else storage_.value_ = std::forward<V>(v);
    }

    template <typename X>
    auto assignError(X&& x)
    noexcept(std::is_nothrow_assignable_v<E&, X>) -> void {
        if (storage_.hasValue_) {
            storage_.destroy();
            constructError(std::forward<X>(x));
        } else storage_.error_ = std::forward<X>(x);
    }

    template <typename R>
    auto assignFromResult(R&& r) -> void {
        if (storage_.hasValue_ != r.storage_.hasValue_) {
            storage_.destroy();
            constructFromResult(std::forward<R>(r));
        } else if (storage_.hasValue_) {
            if constexpr (std::is_lvalue_reference_v<T>)
                storage_.value_ = std::forward<R>(r).storage_.value_.get();
            else storage_.value_ = std::forward<R>(r).storage_.value_;
        } else {
            if constexpr (std::is_lvalue_reference_v<E>)
                storage_.error_ = std::forward<R>(r).storage_.error_.get();
            else storage_.error_ = std::forward<R>(r).storage_.error_;
        }
    }

    ResultStorage(ResultStorage const&) noexcept
    requires AllTriviallyCopyConstructible<T, E> = default;

    ResultStorage(ResultStorage const& rhs)
    noexcept(std::is_nothrow_copy_constructible_v<T> and
             std::is_nothrow_copy_constructible_v<E>)
    requires SomeNotTriviallyCopyAssignable<T, E>
    : storage_{std::monostate{}} {
        constructFromResult(rhs);
    }

    ResultStorage(ResultStorage const&)
    requires SomeNotCopyConstructible<T, E> = delete;

    ResultStorage(ResultStorage&&) noexcept
    requires AllTriviallyMoveConstructible<T, E> = default;

    ResultStorage(ResultStorage&& rhs)
    noexcept(std::is_nothrow_move_constructible_v<T> and
             std::is_nothrow_move_constructible_v<E>)
    requires SomeNotTriviallyMoveAssignable<T, E>
    : storage_{std::monostate{}} {
        constructFromResult(std::move(rhs));
    }

    ResultStorage(ResultStorage&&)
    requires SomeNotMoveConstructible<T, E> = delete;

    auto operator= (ResultStorage const&)
    noexcept -> ResultStorage&
    requires AllTriviallyCopyConstructible<T, E> and
             AllTriviallyCopyAssignable<T, E> and
             AllTriviallyDestructible<T, E>
    = default;

    auto operator= (ResultStorage const& rhs) -> ResultStorage&
    requires SomeNotCopyConstructible<T, E> or
             SomeNotCopyAssignable<T, E> or
             SomeNotDestructible<T, E>
    = delete;

    auto operator= (ResultStorage const& rhs)
    noexcept(std::is_nothrow_copy_constructible_v<T> and
             std::is_nothrow_copy_constructible_v<E> and
             std::is_nothrow_copy_assignable_v<T> and
             std::is_nothrow_copy_assignable_v<E>) -> ResultStorage&
    requires AllCopyConstructible<T, E> and (
             (SomeNotTriviallyCopyAssignable<T, E> and AllDestructible<T, E>) or
             (AllTriviallyCopyAssignable<T, E> and SomeNotTriviallyDestructible<T, E>)) {
        assignFromResult(rhs);
        return *this;
    }

    auto operator= (ResultStorage&&)
    noexcept -> ResultStorage&
    requires AllTriviallyMoveConstructible<T, E> and
             AllTriviallyMoveAssignable<T, E> and
             AllTriviallyDestructible<T, E>
    = default;

    auto operator= (ResultStorage&& rhs) -> ResultStorage&
    requires SomeNotMoveConstructible<T, E> or
             SomeNotMoveAssignable<T, E> or
             SomeNotDestructible<T, E>
    = delete;

    auto operator= (ResultStorage&& rhs)
    noexcept(std::is_nothrow_move_constructible_v<T> and
             std::is_nothrow_move_constructible_v<E> and
             std::is_nothrow_move_assignable_v<T> and
             std::is_nothrow_move_assignable_v<E>) -> ResultStorage&
    requires AllMoveConstructible<T, E> and (
             (SomeNotTriviallyMoveAssignable<T, E> and AllDestructible<T, E>) or
             (AllTriviallyMoveAssignable<T, E> and SomeNotTriviallyDestructible<T, E>)) {
        assignFromResult(std::move(rhs));
        return *this;
    }
};

// Concepts:

template <typename T1, typename E1, typename T2, typename E2>
concept ConvertibleResult =
        // T1 constructible from Result<T2, E2>
        std::constructible_from<T1, Result<T2, E2>&> or
        std::constructible_from<T1, Result<T2, E2> const&> or
        std::constructible_from<T1, Result<T2, E2>&&> or
        std::constructible_from<T1, Result<T2, E2> const&&> or
        // E1 constructible from Result<T2, E2>
        std::constructible_from<E1, Result<T2, E2>&> or
        std::constructible_from<E1, Result<T2, E2> const&> or
        std::constructible_from<E1, Result<T2,E2> &&> or
        std::constructible_from<E1, Result<T2, E2> const&&> or
        // Result<T2, E2> convertible to T1
        std::convertible_to<Result<T2,E2>&, T1> or
        std::convertible_to<Result<T2, E2> const&, T1> or
        std::convertible_to<Result<T2,E2>&&, T1> or
        std::convertible_to<Result<T2, E2> const&&, T1> or
        // Result<T2, E2> convertible to E2
        std::convertible_to<Result<T2,E2>&, E1> or
        std::convertible_to<Result<T2, E2> const&, E1> or
        std::convertible_to<Result<T2,E2>&&, E1> or
        std::convertible_to<Result<T2, E2> const&&, E1>;

template <typename T1, typename E1, typename T2, typename E2>
concept CopyConvertibleResult =
        not ConvertibleResult<T1, E1, T2, E2> and
        std::constructible_from<T1, T2 const&> and
        std::constructible_from<E1, E2 const&>;

template <typename T1, typename E1, typename T2, typename E2>
concept ImplicitlyCopyConvertibleResult =
        CopyConvertibleResult<T1, E1, T2, E2> and
        std::convertible_to<T2 const&, T1> and
        std::convertible_to<E2 const&, E1>;

template <typename T1, typename E1, typename T2, typename E2>
concept ExplicitlyCopyConvertibleResult =
        CopyConvertibleResult<T1, E1, T2, E2> and (
                not std::convertible_to<T2 const&, T1> or
                not std::convertible_to<E2 const&, E1>
        );

template <typename T1, typename E1, typename T2, typename E2>
concept MoveConvertibleResult =
        not ConvertibleResult<T1, E1, T2, E2> and
        std::constructible_from<T1, T2&&> and
        std::constructible_from<E1, E2&&>;

template <typename T1, typename E1, typename T2, typename E2>
concept ImplicitlyMoveConvertibleResult =
        MoveConvertibleResult<T1, E1, T2, E2> and
        std::convertible_to<T2&&, T1> and
        std::convertible_to<E2&&, E1>;

template <typename T1, typename E1, typename T2, typename E2>
concept ExplicitlyMoveConvertibleResult =
        MoveConvertibleResult<T1, E1, T2, E2> and (
                not std::convertible_to<T2&&, T1> or
                not std::convertible_to<E2&&, E1>
        );

template <typename TT, typename T>
concept ResultValueConvertibleTo =
        std::constructible_from<T, TT&&> and
        not std::same_as<std::decay_t<TT>, std::in_place_t> and
        not std::same_as<std::decay_t<TT>, InPlaceValueType> and
        not std::same_as<std::decay_t<TT>, InPlaceErrorType> and
        not IsResult<std::decay_t<TT>>::value;

template <typename TT, typename T>
concept ImplicitlyResultValueConvertibleTo =
        ResultValueConvertibleTo<TT, T> and
        std::convertible_to<TT&&, T>;

template <typename TT, typename T>
concept ExplicitlyResultValueConvertibleTo =
        ResultValueConvertibleTo<TT, T> and
        not std::convertible_to<TT&&, T>;

template <typename T1, typename E1, typename T2, typename E2>
concept ConvertAssignableResult =
        ConvertibleResult<T1, E1, T2, E2> and
        std::assignable_from<T1&, Result<T2,E2>&> and
        std::assignable_from<T1&, Result<T2,E2> const&> and
        std::assignable_from<T1&, Result<T2,E2>&&> and
        std::assignable_from<T1&, Result<T2,E2> const&&> and
        std::assignable_from<E1&, Result<T2,E2>&> and
        std::assignable_from<E1&, Result<T2,E2> const&> and
        std::assignable_from<E1&, Result<T2,E2>&&> and
        std::assignable_from<E1&, Result<T2,E2> const&&>;

template <typename T1, typename E1, typename T2, typename E2>
concept CopyConvertAssignableResult =
        not ConvertAssignableResult<T1, E1, T2, E2> and
        std::constructible_from<T1, T2 const&> and
        std::assignable_from<WrappedValueType<T1>&, T2 const&> and
        std::constructible_from<E1, E2 const&> and
        std::assignable_from<WrappedValueType<E1>&, E2 const&>;

template <typename T1, typename E1, typename T2, typename E2>
concept MoveConvertAssignableResult =
        not ConvertAssignableResult<T1, E1, T2, E2> and
        std::constructible_from<T1, T2&&> and
        std::assignable_from<T1&, T2&&> and
        std::constructible_from<E1, E2&&> and
        std::assignable_from<E1&, E2&&>;

template <typename TT, typename T>
concept ResultValueAssignableTo =
        not IsResult<std::decay_t<TT>>::value and
        not IsFailure<std::decay_t<TT>>::value and
        std::constructible_from<T, TT> and
        std::assignable_from<WrappedValueType<T>&, TT> and (
                not std::same_as<std::decay_t<T>, std::decay_t<TT>> or
                not std::is_scalar_v<T>
        );

template <typename E, typename EE>
concept ErrorAssignable =
        std::constructible_from<E, EE> and
        std::assignable_from<WrappedValueType<E>&, EE>;

template <typename F, typename E>
concept ErrorMapper =
        std::invocable<F, E> and
        not std::same_as<void, std::invoke_result_t<F, E>>;

} // namespace detail

/*!
 * A result object, which at any given time contains either a valid result of
 * a computation or an error indicating that the computation failed.
 *
 * @tparam T the type of the result
 * @tparam E the type of the error
 */
template <typename T, typename E>
class Result {
    static_assert(
            not std::is_abstract_v<T>,
            "Value type may not be abstract");

    static_assert(
            not std::is_same_v<std::decay_t<T>, std::in_place_t>,
            "Value type may not be in_place_t type");

    static_assert(
            not std::is_same_v<std::decay_t<T>, InPlaceValueType>,
            "Value type may not be pimc::InPlaceValueType type");

    static_assert(
            not std::is_same_v<std::decay_t<T>, InPlaceErrorType>,
            "Value type may not be pimc::InPlaceErrorType type");

    static_assert(
            not IsResult<std::decay_t<T>>::value,
            "Value type may not be Result type");

    static_assert(
            not IsFailure<std::decay_t<T>>::value,
            "Value type may not be Failure type");

    static_assert(
            not std::is_rvalue_reference_v<T>,
            "Value type may not be an rvalue reference, "
            "only lvalue references are allowed");

    static_assert(
            not std::is_abstract_v<E>,
            "Error type may not be abstract");

    static_assert(
            not std::is_void_v<std::decay_t<E>>,
            "Error type may not be void");

    static_assert(
            not std::is_same_v<std::decay_t<E>, std::in_place_t>,
            "Error type may not be in_place_t type");

    static_assert(
            not std::is_same_v<std::decay_t<E>, InPlaceValueType>,
            "Error type may not be pimc::InPlaceValueType type");

    static_assert(
            not std::is_same_v<std::decay_t<E>, InPlaceErrorType>,
            "Error type may not be pimc::InPlaceErrorType type");

    static_assert(
            not IsResult<std::decay_t<E>>::value,
            "Error type may not be Result type");

    static_assert(
            not IsFailure<std::decay_t<E>>::value,
            "Error type may not be Failure type");

    static_assert(
            not std::is_rvalue_reference_v<E>,
            "Error type may not be an rvalue reference, "
            "only lvalue references are allowed");

    template <typename T2, typename E2>
    friend class Result;

public:

    using ValueType = T;
    using ErrorType = E;
    using FailureType = Failure<E>;

public:

    /*!
     * \brief Constructs a Result with the default value and in the value state.
     *
     * Usage:
     *
     * ```cpp
     * auto r = pimc::Result<int, std::string>{};
     * ```
     *
     * @tparam U the value type
     */
    template <typename U=T>
    constexpr Result()
    noexcept(std::is_nothrow_constructible_v<U>)
    requires std::constructible_from<U>
    : value_{InPlaceValue} {}

    /*!
     * \brief Copy constructs a Result from \p rhs.
     *
     * If \p rhs contains a value, the constructed copy will contain a
     * a copy of the value.
     *
     * If \p rhs contains an error, the constructed copy will contain a
     * copy of the error.
     *
     * @param rhs the Result to copy
     */
    constexpr Result(Result const& rhs) = default;

    /*!
     * Move constructs a Result from \p rhs.
     *
     * If \p rhs contains a value, move initializes the value of the constructed
     * Result from the value contained in \p rhs, but does not make \p rhs empty.
     *
     * If \p rhs contains an error, the constructed copy will contain the
     * error moved from \p rhs.
     *
     * @param rhs the Result to move
     */
    constexpr Result(Result&& rhs) = default;

    /*!
     * \brief Converting copy constructor.
     *
     * Usage:
     *
     * ```cpp
     * auto const r1 = pimc::Result<int, int>{1};
     * auto const r2 = pimc::Result<long, long>{r1};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result to convert
     */
    template <typename T2, typename E2>
    /* implicit */ Result(Result<T2, E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2 const&> and
             std::is_nothrow_constructible_v<E, E2 const&>)
    requires detail::ImplicitlyCopyConvertibleResult<T, E, T2, E2>
    : value_{std::monostate{}} {
        value_.constructFromResult(rhs.value_);
    }

    /*!
     * \brief Converting copy constructor.
     *
     * Usage:
     *
     * ```cpp
     * auto const r1 = pimc::Result<int, int>{1};
     * auto const r2 = pimc::Result<long, long>{r1};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result to convert
     */
    template <typename T2, typename E2>
    explicit Result(Result<T2, E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<T, const T2&> and
             std::is_nothrow_constructible_v<E, const E2&>)
    requires detail::ExplicitlyCopyConvertibleResult<T, E, T2, E2>
    : value_{std::monostate{}} {
        value_.constructFromResult(rhs.value_);
    }

    /*!
     * \brief Converting move constructor.
     *
     * Example:
     *
     * ```cpp
     * class A {};
     * class B: public A {};
     *
     * auto r1 = pimc::Result<std::unique_ptr<B>, int>{std::make_unique<B>()};
     * auto r2 = pimc::Result<std::unique_ptr<A>, long>{std::move(r1)};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result to move convert
     */
    template <typename T2, typename E2>
    /* implicit */ Result(Result<T2, E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2&&> and
             std::is_nothrow_constructible_v<E, E2&&>)
    requires detail::ImplicitlyMoveConvertibleResult<T, E, T2, E2>
    : value_{std::monostate{}} {
        value_.constructFromResult(std::move(rhs).value_);
    }

    /*!
     * \brief Converting move constructor.
     *
     * Example:
     *
     * ```cpp
     * class A {};
     * class B: public A {};
     *
     * auto r1 = pimc::Result<std::unique_ptr<B>, int>{std::make_unique<B>()};
     * auto r2 = pimc::Result<std::unique_ptr<A>, long>{std::move(r1)};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result to move convert
     */
    template <typename T2, typename E2>
    explicit Result(Result<T2, E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2&&> and
             std::is_nothrow_constructible_v<E, E2&&>)
    requires detail::ExplicitlyMoveConvertibleResult<T, E, T2, E2>
            : value_{std::monostate{}} {
        value_.constructFromResult(std::move(rhs).value_);
    }

    /*!
     * \brief Constructs a Result object that contains a value.
     *
     * The value is constructed from the specified arguments.
     *
     * Example:
     *
     * ```cpp
     * auto r = pimc::Result<std::string, int>{pimc::InPlaceValue, "abc"};
     * ```
     *
     * @tparam Ts the types of the arguments to the value constructor
     * @param args the arguments to the value constructor
     */
    template <typename ... Ts>
    constexpr explicit Result(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, Ts...>)
    : value_{InPlaceValue, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Result which contains a value.
     *
     * The value is constructed from `std::initializer_list<U>` and \p args.
     *
     * Example:
     *
     * ```cpp
     * pimc::Result<std::string, int> r{pimc::InPlaceValue, {'a', 'b', 'c'}};
     * ```
     *
     * @tparam U the type of the value in the initializer list
     * @tparam Ts the types of the optional arguments that follow the initializer list
     * @param il the initializer list
     * @param args the optional arguments supplied after the initializer list
     */
    template <typename U, typename ... Ts>
    constexpr explicit Result(
            InPlaceValueType, std::initializer_list<U> il, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Ts...>)
    requires std::constructible_from<T, std::initializer_list<U>, Ts...>
    : value_{InPlaceValue, il, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a result that contains an error.
     *
     * The error is constructed from the specified arguments.
     *
     * Example:
     *
     * ```cpp
     * auto r = pimc::Result<int, std::string>{pimc::InPlaceError, "error1"};
     * ```
     *
     *
     * @tparam Ts the types of the arguments to the error constructor
     * @param args the arguments to the error constructor
     */
    template <typename ... Ts>
    constexpr explicit Result(InPlaceErrorType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>)
    : value_{InPlaceError, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Result which contains an error.
     *
     * The error is constructed from `std::initializer_list<U>` and \p args.
     *
     * Example:
     *
     * ```cpp
     * pimc::Result<std::string, int> r{
     *         pimc::InPlaceEror, {'e', 'r', 'r', 'o', 'r', '1'}};
     * ```
     *
     * @tparam U the type of the value in the initializer list
     * @tparam Ts the types of the optional arguments that follow the initializer list
     * @param il the initializer list
     * @param args the optional arguments supplied after the initializer list
     */
    template <typename U, typename ... Ts>
    constexpr explicit Result(
            InPlaceErrorType, std::initializer_list<U> il, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>, Ts...>)
    requires std::constructible_from<E, std::initializer_list<U>, Ts...>
    : value_{InPlaceError, il, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Result in the error state and whose error is copy
     * constructed from the error contained in the specified Failure object.
     *
     * Example:
     *
     * ```cpp
     * pimc::Failure<bool> const f{true};
     * pimc::Result<int, bool> r = f;
     *
     * auto getResult() -> pimc::Result<int, std::string> {
     *     Failure<std::string> const f{"error1"s};
     *     return f;
     * }
     * ```
     * @tparam E2
     * @param f
     */
    template <typename E2>
    constexpr /* implicit */ Result(Failure<E2> const& f)
    noexcept(std::is_nothrow_constructible_v<E, E2 const&>)
    requires std::constructible_from<E, E2 const&>
    : value_{InPlaceError, f.error()} {}

    /*!
     * \brief Constructs a Result in the error state and whose error is copy
     * constructed from the error contained in the specified Failure object.
     *
     * Example:
     *
     * ```cpp
     * pimc::Result<int, bool> r = pimc::fail(true);
     *
     * auto getResult() -> pimc::Result<int, std::string> {
     *     return pimc::fail("error1"s);
     * }
     * ```
     * @tparam E2
     * @param f
     */
    template <typename E2>
    constexpr /* implicit */ Result(Failure<E2>&& f)
    noexcept(std::is_nothrow_constructible_v<E, E&&>)
    requires std::constructible_from<E, E2&&>
    : value_{InPlaceError, std::move(f).error()} {}

    /*!
     * \brief Constructs a Result that contains a value.
     *
     * The value of the constructed result is constructed from \p value.
     *
     * Example:
     *
     * ```cpp
     * pimc Result<int, bool> r = 100;
     *
     * auto getValue() -> pimc::Result<std::string, int> {
     *     char const* msg = "message";
     *     return msg;
     * }
     * ```
     *
     * @tparam U the type of the argument to this constructor
     * @param value the value convertible to the value contained in the result.
     */
    template <detail::ImplicitlyResultValueConvertibleTo<T> U>
    constexpr /* implicit */ Result(U&& value)
    noexcept(std::is_nothrow_constructible_v<T, U>)
            : value_{InPlaceValue, std::forward<U>(value)} {}

    /*!
     * \brief Constructs a Result that contains a value.
     *
     * The value of the constructed result is constructed from \p value.
     *
     * Example:
     *
     * ```cpp
     * pimc Result<int, bool> r = 100;
     *
     * auto getValue() -> pimc::Result<std::string, int> {
     *     char const* msg = "message";
     *     return msg;
     * }
     * ```
     *
     * @tparam U the type of the argument to this constructor
     * @param value the value convertible to the value contained in the result.
     */
    template <detail::ExplicitlyResultValueConvertibleTo<T> U>
    constexpr explicit Result(U&& value)
    noexcept(std::is_nothrow_constructible_v<T, U>)
    : value_{InPlaceValue, std::forward<U>(value)} {}

    /*!
     * The copy assignment operator.
     *
     * @param rhs the Result to copy assign
     * @return a reference to this Result
     */
    auto operator= (Result const& rhs) -> Result& = default;

    /*!
     * The move assignment operator.
     *
     * @param rhs the Result to move assign
     * @return a reference to this Result
     */
    auto operator= (Result&& rhs)
    noexcept(std::is_nothrow_assignable_v<T&, T&&> and
             std::is_nothrow_assignable_v<E&, E&&>)
    -> Result& = default;

    /*!
     * \brief Copy constructs the state of \p rhs.
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result whose state is copied into this Result
     * @return a reference to this Result
     */
    template <typename T2, typename E2>
    auto operator= (Result<T2, E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2 const&> and
             std::is_nothrow_constructible_v<E, E2 const&> and
             std::is_nothrow_assignable_v<T&, T2 const&> and
             std::is_nothrow_assignable_v<E&, E2 const&>) -> Result&
    requires detail::CopyConvertAssignableResult<T, E, T2, E2> {
        value_.assignFromResult(rhs.value_);
        return *this;
    }

    /*!
     * \brief Move constructs the state of \p rhs.
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result whose state is moved into this Result
     * @return a reference to this Result
     */
    template <typename T2, typename E2>
    auto operator= (Result<T2, E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2 const&> and
             std::is_nothrow_constructible_v<E, E2 const&> and
             std::is_nothrow_assignable_v<T&, T2&&> and
             std::is_nothrow_assignable_v<E&, E2&&>) -> Result&
    requires detail::MoveConvertAssignableResult<T, E, T2, E2> {
        value_.assignFromResult(std::move(rhs).value_);
        return *this;
    }

    /*!
     * \brief Perfect-forwarding value assignment operator.
     *
     * @tparam U the value type convertible to the value type of this Result
     * @param value the value to perfect forward to the value of this Result
     * @return a reference to this Result
     */
    template <detail::ResultValueAssignableTo<T> U>
    auto operator= (U&& value)
    noexcept(std::is_nothrow_constructible_v<T, U> and
             std::is_nothrow_assignable_v<T&, U>) -> Result& {
        value_.assignValue(std::forward<U>(value));
        return *this;
    }

    /*!
     * \brief Error copy assignment operator.
     *
     * @tparam EE the error type of the Failure object
     * @param rhs the Failure object whose error is copied into this Result
     * @return a reference to this Result
     */
    template <typename EE>
    auto operator= (Failure<EE> const& rhs)
    noexcept(std::is_nothrow_constructible_v<E, EE const&> and
             std::is_nothrow_assignable_v<E&, EE const&>) -> Result&
    requires detail::ErrorAssignable<E, EE const&> {
        value_.assignError(rhs.error());
        return *this;
    }

    /*!
     * \brief Error move assignment operator.
     *
     * @tparam EE the error type of the Failure object
     * @param rhs the Failure object whose error is moved into this Result
     * @return a reference to this Result
     */
    template <typename EE>
    auto operator= (Failure<EE>&& rhs)
    noexcept(std::is_nothrow_constructible_v<E, EE&&> and
             std::is_nothrow_assignable_v<E&, EE&&>) -> Result&
    requires detail::ErrorAssignable<E, EE&&> {
        value_.assignError(std::move(rhs).error());
        return *this;
    }

    /*!
     * \brief Returns an lvalue reference to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Result contains a value.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Result<std::string, int> r{"xyz"s};
     *
     * std::cout << "Value: '" << (*r).c_str << "'" << std::endl;
     * ```
     *
     * @return an lvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto operator*() & noexcept -> std::add_lvalue_reference_t<T> {
        return value_.storage_.value_;
    }

    /*!
     * \brief Returns an rvalue reference to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Result contains a value.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain a value.
     *
     * @return an rvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto operator*() && noexcept -> std::add_rvalue_reference_t<T> {
        return static_cast<std::add_rvalue_reference_t<T>>(value_.storage_.value_);
    }

    /*!
     * \brief Returns a const lvalue reference to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Result contains a value.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain a value.
     *
     * @return a const lvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto operator*() const& noexcept
    -> std::add_lvalue_reference_t<std::add_const_t<T>> {
        return value_.storage_.value_;
    }

    /*!
     * \brief Returns a const rvalue reference to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Result contains a value.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain a value.
     *
     * @return a const rvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto operator*() const&& noexcept
    -> std::add_rvalue_reference_t<std::add_const_t<T>> {
        return static_cast<
            std::add_rvalue_reference_t<std::add_const_t<T>>>(value_.storage_.value_);
    }

    /*!
     * \brief Retrieves a pointer to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Result contains a value.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Result<std::string, int> r{"xyz"s};
     *
     * std::cout << "Value: '" << r->c_str() << "'" << std::endl;
     * ```
     *
     * @return a pointer to the contained value
     */
    [[nodiscard]]
    constexpr auto operator->()
    noexcept -> std::remove_reference_t<T>* {
        return std::addressof(**this);
    }

    /*!
     * \brief Retrieves a const pointer to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Result contains a value.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Result<std::string, int> r{"xyz"s};
     *
     * std::cout << "Value: '" << r->c_str() << "'" << std::endl;
     * ```
     *
     * @return a const pointer to the contained value
     */
    [[nodiscard]]
    constexpr auto operator->()
    const noexcept -> std::remove_reference_t<std::add_const_t<T>>* {
        return std::addressof(**this);
    }

    /*!
     * \brief Returns `true` if the Result contains a value.
     *
     * @return `true` if the Result contains a value, false otherwise
     */
    [[nodiscard]]
    constexpr auto hasValue() const noexcept -> bool {
        return value_.storage_.hasValue_;
    }

    /*!
     * \brief Returns `true` if the Result contains an error.
     *
     * @return `true` if the Result contains an error, false otherwise
     */
    [[nodiscard]]
    constexpr auto hasError() const noexcept -> bool {
        return not value_.storage_.hasValue_;
    }

    /*!
     * \brief Returns `true` if the Result contains a value.
     *
     * @return `true` if the Result contains a value, false otherwise
     */
    [[nodiscard]]
    constexpr explicit operator bool() const noexcept {
        return hasValue();
    }

    /*!
     * \brief Returns an lvalue reference to the contained value.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Result<std::string, int> r{"xyz"s};
     *
     * std::cout << "Value: '" << r.value() << "'" << std::endl;
     * ```
     *
     * @return an lvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto value() & noexcept -> std::add_lvalue_reference_t<T> {
        return value_.storage_.value_;
    }

    /*!
     * \brief Returns an rvalue reference to the contained value.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain a value.
     *
     * @return an rvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto value() && noexcept -> std::add_rvalue_reference_t<T> {
        return static_cast<std::add_rvalue_reference_t<T>>(value_.storage_.value_);
    }

    /*!
     * \brief Returns a const lvalue reference to the contained value.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain a value.
     *
     * @return a const lvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto value() const& noexcept
    -> std::add_lvalue_reference_t<std::add_const_t<T>> {
        return value_.storage_.value_;
    }

    /*!
     * \brief Returns a const rvalue reference to the contained value.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain a value.
     *
     * @return a const rvalue reference to the contained value
     */
    [[nodiscard]]
    constexpr auto value() const&& noexcept
    -> std::add_rvalue_reference_t<std::add_const_t<T>> {
        return static_cast<
            std::add_rvalue_reference_t<std::add_const_t<T>>>(value_.storage_.value_);
    }

    /*!
     * \brief Returns an lvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * Example:
     *
     * ```cpp
     * Result<int, std::string> r{pimc::fail("error1"s)};
     *
     * std::cout << "Error: '" << r.error() << "'" << std::endl;
     * ```
     *
     * @return an lvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() & noexcept -> std::add_lvalue_reference_t<E> {
        return value_.storage_.error_;
    }

    /*!
     * \brief Returns an rvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * @return an rvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() && noexcept -> std::add_rvalue_reference_t<E> {
        return static_cast<std::add_rvalue_reference_t<E>>(value_.storage_.error_);
    }

    /*!
     * \brief Returns a cost lvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * @return a const lvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() const& noexcept
    -> std::add_lvalue_reference_t<std::add_const_t<E>> {
        return value_.storage_.error_;
    }

    /*!
     * \brief Returns a const rvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * @return a const rvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() const&& noexcept
    -> std::add_rvalue_reference_t<std::add_const_t<E>> {
        return static_cast<
            std::add_rvalue_reference_t<std::add_const_t<E>>>(value_.storage_.error_);
    }

    /*!
     * \brief Returns a copy of the contained value if this Result holds a value
     * otherwise returns \p defaultValue.
     *
     * Example:
     *
     * ```cpp
     * auto r1 = pimc::Result<int, int>{100};
     * assert(r1.valueOr(0) == 100);
     *
     * auto r2 = pimc::Result<int, int>{pimc::fail(100)};
     * assert(r2.valueOr(0) == 0);
     * ```
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a copy of the contained value if this Result holds a value otherwise
     * \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto valueOr(U&& defaultValue) const & -> std::remove_reference_t<T> {
        return hasValue() ? value_.storage_.value_ : std::forward<U>(defaultValue);
    }

    /*!
     * Returns a moved out value of the Result if it holds a value, otherwise
     * returns \p defaultValue.
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a moved out value of the Result if it holds a value, otherwise
     * return \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto valueOr(U&& defaultValue) && -> std::remove_reference_t<T> {
        return hasValue() ?
               std::move(value_.storage_.value_) : std::forward<U>(defaultValue);
    }

    /*!
     * \brief Returns a copy of the contained error if this Result holds an error
     * otherwise returns \p defaultValue.
     *
     * Example:
     *
     * ```cpp
     * auto r1 = pimc::Result<int, int>{100};
     * assert(r1.errorOr(0) == 0);
     *
     * auto r2 = pimc::Result<int, int>{pimc::fail(100)};
     * assert(r2.errorOr(0) == 100);
     * ```
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a copy of the contained error if this Result holds an error otherwise
     * \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto errorOr(U&& defaultValue) const& -> std::remove_reference_t<E> {
        return not hasValue() ? value_.storage_.error_ : std::forward<U>(defaultValue);
    }

    /*!
     * Returns a moved out error of the Result if it holds an error, otherwise
     * returns \p defaultValue.
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a moved out error of the Result if it holds an error, otherwise
     * return \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto errorOr(U&& defaultValue) && -> std::remove_reference_t<E> {
        return not hasValue() ?
               std::move(value_.storage_.error_) : std::forward<U>(defaultValue);
    }

    /*!
     * \brief Invokes the function \p f with a const reference to the value
     * contained in this Result as the argument and returns the return value
     * of the function.
     *
     * If this Result contains an error, returns a Result containing the
     * same error.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained value
     * @return a return value of the function (which is a Result) if this
     * Result contains a value, otherwise a Result containing the same error
     * as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMap(F&& f) const & -> std::invoke_result_t<F, const T&>
    requires requires(F&& fn, T const& v) { { fn(v) } -> ResultType; } {
        using ReturnType = std::invoke_result_t<F, T const&>;

        return hasValue()
            ? std::invoke(std::forward<F>(f), value_.storage_.value_)
            : ReturnType{InPlaceError, value_.storage_.error_};
    }

    /*!
     * \brief Invokes the function \p f with an rvalue reference to the
     * value contained in this Result as the argument and returns the return
     * value of the function.
     *
     * If this Result contains an error, returns a Result containing the
     * same error.
     *
     * \note If this Result contains a value, it's moved to the argument of
     * the function; if this Result contains an error, it's moved to the
     * returned Result.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained value
     * @return a return value of the function (which is a Result) if this
     * Result contains a value, otherwise a Result containing the same error
     * as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMap(F&& f) && -> std::invoke_result_t<F, T&&>
    requires requires(F&& fn, T&& v) { { fn(v) } -> ResultType; } {
        using ReturnType = std::invoke_result_t<F, T&&>;

        return hasValue()
            ? std::invoke(std::forward<F>(f), std::move(value_.storage_.value_))
            : ReturnType{InPlaceError, std::move(value_.storage_.error_)};
    }

    /*!
     * \brief Invokes the function \p f with a const reference to the value
     * contained in this Result and returns a Result containing the result of
     * the function as the value.
     *
     * If this Result contains an error, the function is not invoked and a Result
     * containing the same error is returned.
     *
     * @tparam F the type of the function \p f
     * @param f the function to invoke on the value contained in this Result
     * @return a Result containing the return value of the function if this Result
     * contains a value, otherwise a Result containing the same error
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto map(F&& f) const & -> Result<std::invoke_result_t<F, T const&>, E>
    requires std::invocable<F, T const&> {
        using ReturnValueType = std::invoke_result_t<F, T const&>;
        using ReturnType = Result<ReturnValueType, E>;

        if constexpr (not std::is_void_v<ReturnValueType>) {
            return hasValue()
                ? ReturnType{
                    InPlaceValue,
                    std::invoke(std::forward<F>(f), value_.storage_.value_)}
                : ReturnType{InPlaceError, value_.storage_.error_};
        } else {
            if (hasValue()) {
                std::invoke(std::forward<F>(f), value_.storage_.value_);
                return ReturnType{};
            } else return ReturnType{InPlaceError, value_.storage_.error_};
        }
    }

    /*!
     * \brief Invokes the function \p f with an rvalue reference to the value
     * contained in this Result and returns a Result containing the result of
     * the function as the value.
     *
     * If this Result contains an error, the function is not invoked and a Result
     * containing the same error is returned.
     *
     * \note If this Result contains a value it's moved to the argument of the
     * function \p f, otherwise the error contained in this Result is moved to
     * the returned Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to invoke on the value contained in this Result
     * @return a Result containing the return value of the function if this Result
     * contains a value, otherwise a Result containing the same error
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto map(F&& f) && -> Result<std::invoke_result_t<F, T&&>, E>
    requires std::invocable<F, T&&> {
        using ReturnValueType = std::invoke_result_t<F, T&&>;
        using ReturnType = Result<ReturnValueType, E>;

        if constexpr (not std::is_void_v<ReturnValueType>) {
            return hasValue()
                   ? ReturnType{
                        InPlaceValue,
                        std::invoke(
                                std::forward<F>(f), std::move(value_.storage_.value_))}
                   : ReturnType{InPlaceError, std::move(value_.storage_.error_)};
        } else {
            if (hasValue()) {
                std::invoke(std::forward<F>(f), std::move(value_.storage_.value_));
                return ReturnType{};
            } else return ReturnType{InPlaceError, std::move(value_.storage_.error_)};
        }
    }

    /*!
     * \brief Invokes the function \b f with a const reference to the error
     * contained in this Result as the argument to the function and returns
     * the return value of the function.
     *
     * If this Result contains a value, returns a Result containing the same
     * value.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained error
     * @return a return value of the function (which is a Result) if this
     * Result contains an error, otherwise a Result containing the same value
     * as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMapError(F&& f) const & -> std::invoke_result_t<F, E const&>
    requires requires(F&& fn, E const& e) { { f(e) } -> ResultType; } {
        using ReturnType = std::invoke_result_t<F, E const&>;

        return hasError()
            ? std::invoke(std::forward<F>(f), value_.storage_.error_)
            : ReturnType{InPlaceValue, value_.storage_.value_};
    }

    /*!
     * \brief Invokes the function \b f with an rvalue reference to the error
     * contained in this Result as the argument to the function and returns
     * the return value of the function.
     *
     * If this Result contains a value, returns a Result containing the same
     * value.
     *
     * \note If this Result contains an error, it's moved to the argument of
     * the function; if this Result contains a value, it's moved to the returned
     * Result.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained error
     * @return a return value of the function (which is a Result) if this
     * Result contains an error, otherwise a Result containing the same value
     * as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMapError(F&& f) && -> std::invoke_result_t<F, E&&>
    requires requires(F&& fn, E&& e) { { fn(std::move(e)) } -> ResultType; } {
        using ReturnType = std::invoke_result_t<F, E&&>;

        return hasError()
               ? std::invoke(std::forward<F>(f), std::move(value_.storage_.error_))
               : ReturnType{InPlaceValue, std::move(value_.storage_.value_)};
    }

    /*!
     * \brief Invokes the function \b f with a const reference of the error
     * contained in the Result and returns a Result whose error is constructed
     * from the return value of the function.
     *
     * If this Result contains a value, the function \b f is not invoked, and
     * a Result containing the same value as this Result is returned.
     *
     * @tparam F the type of the function \b f
     * @param f the function to invoke with the error contained in this Result
     * @return a Result containing the return value of the function invoked
     * with the error contained in this Result if this Result is in the error
     * state, otherwise a Result containing the same value as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto mapError(F&& f) const &
    -> Result<T, std::invoke_result_t<F, E const&>>
    requires detail::ErrorMapper<F, E const &> {
        using ReturnType = Result<T, std::invoke_result_t<F, E const&>>;

        return hasError()
            ? ReturnType{
                InPlaceError,
                std::invoke(std::forward<F>(f), value_.storage_.error_)}
            : ReturnType{InPlaceValue, value_.storage_.value_};
    }

    /*!
     * \brief Invokes the function \b f with an rvalue reference of the error
     * contained in the Result and returns a Result whose error is constructed
     * from the return value of the function.
     *
     * \note If this Result contains an error it's moved to the argument of the
     * function \p f, otherwise the value contained in this Result is moved to
     * the returned Result.
     *
     * If this Result contains a value, the function \b f is not invoked, and
     * a Result containing the same value as this Result is returned.
     *
     * @tparam F the type of the function \b f
     * @param f the function to invoke with the error contained in this Result
     * @return a Result containing the return value of the function invoked
     * with the error contained in this Result if this Result is in the error
     * state, otherwise a Result containing the same value as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto mapError(F&& f) &&
    -> Result<T, std::invoke_result_t<F, E&&>>
    requires detail::ErrorMapper<F, E&&> {
        using ReturnType = Result<T, std::invoke_result_t<F, E&&>>;

        return hasError()
            ? ReturnType{
                InPlaceError,
                std::invoke(
                        std::forward<F>(f), std::move(value_.storage_.error_))}
            : ReturnType{InPlaceValue, std::move(value_.storage_.value_)};
    }

private:
    detail::ResultStorage<T, E> value_;
};

/*!
 * A specialization of the Result where the type of the valid result is `void`.
 *
 * @tparam E the type of the error
 */
template <typename E>
class Result<void, E> {

    static_assert(
            not std::is_abstract_v<E>,
            "Error type may not be abstract");

    static_assert(
            not std::is_void_v<std::decay_t<E>>,
            "Error type may not be void");

    static_assert(
            not std::is_same_v<std::decay_t<E>, std::in_place_t>,
            "Error type may not be in_place_t type");

    static_assert(
            not std::is_same_v<std::decay_t<E>, InPlaceValueType>,
            "Error type may not be pimc::InPlaceValueType type");

    static_assert(
            not std::is_same_v<std::decay_t<E>, InPlaceErrorType>,
            "Error type may not be pimc::InPlaceErrorType type");

    static_assert(
            not IsResult<std::decay_t<E>>::value,
            "Error type may not be Result type");

    static_assert(
            not IsFailure<std::decay_t<E>>::value,
            "Error type may not be Failure type");

    static_assert(
            not std::is_rvalue_reference_v<E>,
            "Error type may not be an rvalue reference, "
            "only lvalue references are allowed");

    template <typename T2, typename E2>
    friend class Result;

public:
    using ValueType = void;
    using ErrorType = E;
    using FailureType = Failure<E>;

    /*!
     * \brief Constructs a Result in the value state.
     *
     * Example:
     *
     * ```cpp
     * auto r = pimc::Result<void, int>{};
     * ```
     */
    constexpr Result() noexcept: value_{InPlaceValue} {}

    /*!
     * \brief Copy constructs a Result
     *
     * @param rhs the Result from which to copy construct
     */
    constexpr Result(Result const& rhs) = default;

    /*!
     * \brief Move constructs a Result
     *
     * @param rhs the Result from which to move construct
     */
    constexpr Result(Result&& rhs)
    noexcept(std::is_nothrow_move_constructible_v<E>) = default;

    /*!
     * \brief Converting copy constructor.
     *
     * If \p rhs contains a value, constructs a Result in the value state
     * ignoring the value contained in \p rhs.
     *
     * If \p rhs contains an error, constructs a Result in the error state
     * whose error is a copy of the error contained in \p rhs.
     *
     * @tparam U the value type of the \p rhs
     * @tparam E2 the error type of the \p rhs
     * @param rhs the Result to copy
     */
    template <typename U, typename E2>
    explicit Result(Result<U, E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E const&>)
    requires std::constructible_from<E, E2 const&>
    : value_{std::monostate{}} {
        value_.constructErrorFromResult(rhs.value_);
    }

    /*!
     * \brief Converting move constructor.
     *
     * If \p rhs contains an error, constructs a Result in the error state
     * whose error is move constructed from the error contained in \p rhs.
     *
     * @tparam U the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Result to move from
     */
    template <typename U, typename E2>
    explicit Result(Result<U, E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2&&>)
    requires std::constructible_from<E, E2&&>
    : value_{std::monostate{}} {
        value_.constructErrorFromResult(std::move(rhs).value_);
    }

    /*!
     * Constructs a Result in the value state.
     */
    constexpr explicit Result(InPlaceValueType) noexcept
    : value_{InPlaceValue} {}

    /*!
     * \brief Constructs a Result which contains an error constructed from
     * the specified arguments.
     *
     * @tparam Ts the types of the arguments to the constructor of the error
     * @param args the arguments to the constructor of the error
     */
    template <typename ... Ts>
    constexpr explicit Result(InPlaceErrorType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, Ts...>)
    requires std::constructible_from<E, Ts...>
    : value_{InPlaceError, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Result which contains an error constructed from the
     * specified initializer list and additional arguments.
     *
     * @tparam U the type of the elements of the initializer list
     * @tparam Ts the types of the additional arguments
     * @param il the initializer list
     * @param args the additional arguments
     */
    template <typename U, typename ... Ts>
    constexpr explicit Result(
            InPlaceErrorType, std::initializer_list<U> il, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>, Ts...>)
    requires std::constructible_from<E, std::initializer_list<U>&, Ts...>
    : value_{InPlaceError, il, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Result in the error state containing the error
     * contained in the Failure \p rhs.
     *
     * @tparam E2 the type of the error contained in \p rhs
     * @param rhs the Failure from whose error to construct the Result
     */
    template <typename E2>
    constexpr /* implicit */ Result(Failure<E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2 const&>)
    requires std::constructible_from<E, E2 const&>
    : value_{InPlaceError, rhs.error()} {}

    /*!
     * \brief Constructs a Result in the error state containing the moved
     * out error from the Failure \p rhs.
     *
     * @tparam E2 the type of the error contained in \p rhs
     * @param rhs the Failure whose moved out error is used to construct
     * the error contained in this Result
     */
    template <typename E2>
    constexpr /* implicit */ Result(Failure<E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2&&>)
    requires std::constructible_from<E2, E2&&>
    : value_{InPlaceError, std::move(rhs).error()} {}

    /*!
     * \brief The copy assignment operator
     *
     * @param rhs the Result from which to copy assign this Result
     * @return a reference to this Result
     */
    auto operator= (Result const& rhs)
    noexcept(std::is_nothrow_copy_assignable_v<E>) -> Result& = default;

    /*!
     * \brief The move assignment operator
     *
     * @param rhs the Result from which to move assign this Result
     * @return a reference to this Result
     */
    auto operator= (Result&& rhs)
    noexcept(std::is_nothrow_move_assignable_v<E>) -> Result& = default;

    /*!
     * \brief Converting copy assignment operator.
     *
     * If both this Result and \p rhs contain an error, the error from \p rhs
     * is copy assigned to the error contained in this Result.
     *
     * If \p rhs contains an error, but this Result is in the value state,
     * then the error in this result is copy constructed from the error
     * contained in \p rhs.
     *
     * If \p rhs is in value state, but this Result is in the error state,
     * the error is destroyed and this Result in transitioned to the value
     * state.
     *
     * @tparam E2 the type of the error in \p rhs
     * @param rhs the Result whose state and error to copy
     * @return a reference to this Result
     */
    template <typename E2>
    auto operator= (Result<void, E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2 const&> and
             std::is_nothrow_assignable_v<E&, E2 const&>) -> Result&
    requires std::constructible_from<E, E2 const&> and
             std::assignable_from<WrappedValueType<E>&, E2 const&> {
        value_.assignFromResult(rhs.value_);
        return *this;
    }

    /*!
     * \brief Converting move assignment operator.
     *
     * If both this Result and \p rhs contain an error, the error from \p rhs
     * is move assigned to the error contained in this Result.
     *
     * If \p rhs contains an error, but this Result is in the value state,
     * then the error in this result is move constructed from the error
     * contained in \p rhs.
     *
     * If \p rhs is in value state, but this Result is in the error state,
     * the error is destroyed and this Result in transitioned to the value
     * state.
     *
     * @tparam E2 the type of the error in \p rhs
     * @param rhs the Result whose error state to copy and error to move
     * @return a reference to this Result
     */
    template <typename E2>
    auto operator= (Result<void, E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2&&> and
             std::is_nothrow_assignable_v<E&, E2&&>) -> Result&
    requires std::constructible_from<E, E2&&> and
             std::assignable_from<WrappedValueType<E>&, E&&> {
        value_.assignFromResult(std::move(rhs).value_);
        return *this;
    }

    /*!
     * \brief Assignment operator from const reference to a Failure.
     *
     * If this Result contains a value, it's transitioned to the error state
     * and its error is copy constructed from the error contained in the
     * Failure \p rhs. Otherwise, its error is copy assigned from the error
     * contained in the Failure \p rhs.
     *
     * @tparam E2 the type of the error in the Failure \p rhs
     * @param rhs the Failure whose error to copy
     * @return a reference to this Result
     */
    template <typename E2>
    auto operator= (Failure<E2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2 const&> and
             std::is_nothrow_assignable_v<E&, E2 const&>) -> Result&
    requires detail::ErrorAssignable<E, E2 const&> {
        value_.assignError(rhs.error());
        return *this;
    }

    /*!
     * \brief Assignment operator from rvalue reference to a Failure.
     *
     * If this Result contains a value, it's transitioned to the error state
     * and its error is move constructed from the error contained in the
     * Failure \p rhs. Otherwise, its error is move assigned from the error
     * contained in the Failure \p rhs.
     *
     * @tparam E2 the type of the error in the Failure \p rhs
     * @param rhs the Failure whose error to move
     * @return a reference to this Result
     */
    template <typename E2>
    auto operator= (Failure<E2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<E, E2&&> and
             std::is_nothrow_assignable_v<E&, E2&&>) -> Result&
    requires detail::ErrorAssignable<E, E2&&> {
        value_.assignError(std::move(rhs).error());
        return *this;
    }

    /*!
     * \brief bool cast operator.
     *
     * Returns `true` if this Result does not contain an error.
     *
     * @return `true` if this Result does not contain an error, `false`
     * otherwise
     */
    [[nodiscard]]
    constexpr explicit operator bool() const noexcept {
        return value_.storage_.hasValue_;
    }

    /*!
     * Returns `true` if this Result does not contain an error.
     *
     * @return `true` if this Result does not contain an error, `false`
     * otherwise
     */
    [[nodiscard]]
    constexpr auto hasValue() const noexcept -> bool {
        return value_.storage_.hasValue_;
    }

    /*!
     * Returns `true` if this Result contains an error.
     *
     * @return `true` if this Result contains an error, `false`
     * otherwise
     */
    [[nodiscard]]
    constexpr auto hasError() const noexcept -> bool {
        return not value_.storage_.hasValue_;
    }

    auto value() const & -> void {}

    auto value() && -> void {}

    /*!
     * \brief Returns an lvalue reference to the contained error.
     *
     * \warning The behavior of this operator is undefined if the Result does
     * not contain an error.
     *
     * Example:
     *
     * ```cpp
     * Result<int, std::string> r{pimc::fail("error1"s)};
     *
     * std::cout << "Error: '" << r.error() << "'" << std::endl;
     * ```
     *
     * @return an lvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() & noexcept -> std::add_lvalue_reference_t<E> {
        return value_.storage_.error_;
    }

    /*!
     * \brief Returns an rvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * @return an rvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() && noexcept -> std::add_rvalue_reference_t<E> {
        return static_cast<std::add_rvalue_reference_t<E>>(value_.storage_.error_);
    }

    /*!
     * \brief Returns a cost lvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * @return a const lvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() const& noexcept
    -> std::add_lvalue_reference_t<std::add_const_t<E>> {
        return value_.storage_.error_;
    }

    /*!
     * \brief Returns a const rvalue reference to the contained error.
     *
     * \warning The behavior of this function is undefined if the Result does
     * not contain an error.
     *
     * @return a const rvalue reference to the contained error
     */
    [[nodiscard]]
    constexpr auto error() const&& noexcept
    -> std::add_rvalue_reference_t<std::add_const_t<E>> {
        return static_cast<
            std::add_rvalue_reference_t<std::add_const_t<E>>>(value_.storage_.error_);
    }

    /*!
     * \brief Returns a copy of the contained error if this Result holds an error
     * otherwise returns \p defaultValue.
     *
     * Example:
     *
     * ```cpp
     * auto r1 = pimc::Result<int, int>{100};
     * assert(r1.errorOr(0) == 0);
     *
     * auto r2 = pimc::Result<int, int>{pimc::fail(100)};
     * assert(r2.errorOr(0) == 100);
     * ```
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a copy of the contained error if this Result holds an error otherwise
     * \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto errorOr(U&& defaultValue) const& -> std::remove_reference_t<E> {
        return not hasValue() ? value_.storage_.error_ : std::forward<U>(defaultValue);
    }

    /*!
     * Returns a moved out error of the Result if it holds an error, otherwise
     * returns \p defaultValue.
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a moved out error of the Result if it holds an error, otherwise
     * return \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto errorOr(U&& defaultValue) && -> std::remove_reference_t<E> {
        return not hasValue() ?
               std::move(value_.storage_.error_) : std::forward<U>(defaultValue);
    }

    /*!
     * \brief Invokes the function \p f with no arguments and returns the
     * return value of the function.
     *
     * If this Result contains an error, returns a Result containing the
     * same error.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained value
     * @return a return value of the function (which is a Result) if this
     * Result does not contain an error, otherwise a Result containing the
     * same error as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMap(F&& f) const & -> std::invoke_result_t<F>
    requires requires(F&& fn) { { fn() } -> ResultType; } {
        using ReturnType = std::invoke_result_t<F>;

        return hasValue()
               ? std::invoke(std::forward<F>(f))
               : ReturnType{InPlaceError, value_.storage_.error_};
    }

    /*!
     * \brief Invokes the function \p f with no arguments and returns the
     * return value of the function.
     *
     * If this Result contains an error, returns a Result containing the
     * same error.
     *
     * \note If this Result contains an error it's moved to the returned
     * Result.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained value
     * @return a return value of the function (which is a Result) if this
     * Result does not contain an error, otherwise a Result containing the
     * same error as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMap(F&& f) && -> std::invoke_result_t<F>
    requires requires(F&& fn) { { fn() } -> ResultType; } {
        using ReturnType = std::invoke_result_t<F>;

        return hasValue()
               ? std::invoke(std::forward<F>(f))
               : ReturnType{InPlaceError, std::move(value_.storage_.error_)};
    }

    /*!
     * \brief Invokes the function \p f with no arguments and returns a Result
     * containing the result of the function as the value.
     *
     * If this Result contains an error, the function is not invoked and a Result
     * containing the same error is returned.
     *
     * @tparam F the type of the function \p f
     * @param f the function to invoke
     * @return a Result containing the return value of the function if this Result
     * does not contain an error, otherwise a Result containing the same error
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto map(F&& f) const & -> Result<std::invoke_result_t<F>, E>
    requires std::invocable<F> {
        using ReturnValueType = std::invoke_result_t<F>;
        using ReturnType = Result<ReturnValueType, E>;

        if constexpr (not std::is_void_v<ReturnValueType>) {
            return hasValue()
                   ? ReturnType{InPlaceValue, std::invoke(std::forward<F>(f))}
                   : ReturnType{InPlaceError, value_.storage_.error_};
        } else {
            if (hasValue()) {
                std::invoke(std::forward<F>(f));
                return ReturnType{};
            } else return ReturnType{InPlaceError, value_.storage_.error_};
        }
    }

    /*!
     * \brief Invokes the function \p f with no arguments and returns a Result
     * containing the result of the function as the value.
     *
     * If this Result contains an error, the function is not invoked and a Result
     * containing the same error is returned.
     *
     * \note If this Result contains an error it is moved to the returned Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to invoke
     * @return a Result containing the return value of the function if this Result
     * does not contain an error, otherwise a Result containing the same error
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto map(F&& f) && -> Result<std::invoke_result_t<F>, E>
    requires std::invocable<F> {
        using ReturnValueType = std::invoke_result_t<F>;
        using ReturnType = Result<ReturnValueType, E>;

        if constexpr (not std::is_void_v<ReturnValueType>) {
            return hasValue()
                   ? ReturnType{InPlaceValue, std::invoke(std::forward<F>(f))}
                   : ReturnType{InPlaceError, std::move(value_.storage_.error_)};
        } else {
            if (hasValue()) {
                std::invoke(std::forward<F>(f));
                return ReturnType{};
            } else return ReturnType{InPlaceError, std::move(value_.storage_.error_)};
        }
    }

    /*!
     * \brief Invokes the function \b f with a const reference to the error
     * contained in this Result as the argument to the function and returns
     * the return value of the function.
     *
     * If this Result contains a value, returns a Result a default constructed
     * value.
     *
     * \note The function \p f must return a Result whosse value type is
     * default constructible.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained error
     * @return a return value of the function (which is a Result) if this
     * Result contains an error, otherwise a Result containing a default
     * constructed value
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMapError(F&& f) const & -> std::invoke_result_t<F, E const&>
    requires requires(F&& fn, E const& e) { { fn(e) } -> ResultTypeValueDefaultConstructible; } {
        using ReturnType = std::invoke_result_t<F, E const&>;

        return hasError()
               ? std::invoke(std::forward<F>(f), value_.storage_.error_)
               : ReturnType{InPlaceValue};
    }

    /*!
     * \brief Invokes the function \b f with an rvalue reference to the error
     * contained in this Result as the argument to the function and returns
     * the return value of the function.
     *
     * If this Result contains a value, returns a Result containing a default
     * constructed value.
     *
     * \note If this Result contains an error, it's moved to the argument of
     * the function.
     *
     * \note The function \p f must return a Result.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained error
     * @return a return value of the function (which is a Result) if this
     * Result contains an error, otherwise a Result containing a default
     * constructed value
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMapError(F&& f) && -> std::invoke_result_t<F, E&&>
    requires requires(F&& fn, E&& e) {
        { fn(std::move(e)) } -> ResultTypeValueDefaultConstructible; } {
        using ReturnType = std::invoke_result_t<F, E&&>;

        return hasError()
               ? std::invoke(std::forward<F>(f), std::move(value_.storage_.error_))
               : ReturnType{InPlaceValue};
    }

    /*!
     * \brief Invokes the function \b f with a const reference of the error
     * contained in the Result and returns a Result whose error is constructed
     * from the return value of the function.
     *
     * If this Result does not contain an error, the function \b f is not invoked,
     * and a Result in the value state is returned.
     *
     * @tparam F the type of the function \b f
     * @param f the function to invoke with the error contained in this Result
     * @return a Result containing the return value of the function invoked
     * with the error contained in this Result if this Result is in the error
     * state, otherwise a Result in the value state
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto mapError(F&& f) const &
    -> Result<void, std::invoke_result_t<F, E const&>>
    requires detail::ErrorMapper<F, E const &> {
        using ReturnType = Result<void, std::invoke_result_t<F, E const&>>;

        return hasError()
               ? ReturnType{
                        InPlaceError,
                        std::invoke(std::forward<F>(f), value_.storage_.error_)}
               : ReturnType{InPlaceValue};
    }

    /*!
     * \brief Invokes the function \b f with an rvalue reference of the error
     * contained in the Result and returns a Result whose error is constructed
     * from the return value of the function.
     *
     * \note If this Result contains an error it's moved to the argument of the
     * function \p f.
     *
     * If this Result does not contain an error, the function \b f is not invoked,
     * and a Result in the value state is returned.
     *
     * @tparam F the type of the function \b f
     * @param f the function to invoke with the error contained in this Result
     * @return a Result containing the return value of the function invoked
     * with the error contained in this Result if this Result is in the error
     * state, otherwise a Result containing the same value as this Result
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto mapError(F&& f) &&
    -> Result<void, std::invoke_result_t<F, E&&>>
    requires detail::ErrorMapper<F, E&&> {
        using ReturnType = Result<void, std::invoke_result_t<F, E&&>>;

        return hasError()
               ? ReturnType{
                        InPlaceError,
                        std::invoke(
                                std::forward<F>(f), std::move(value_.storage_.error_))}
               : ReturnType{InPlaceValue};
    }

private:
    detail::ResultStorage<std::monostate, E> value_;
};

//
// Generic comparison operations
//

/*!
 * The equality relation between two result objects containing comparable value
 * and error objects. The following rules apply:
 *   - If one result contains a value but the other result contains an error,
 *     these results are not equal
 *   - If both results contain either values or errors then they are equal if
 *     the values or errors are equal respectively
 *
 * @tparam T1 the type of value in the first result object
 * @tparam E1 the type of error in the first result object
 * @tparam T2 the type of value in the second result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the two results are equal, `false` otherwise
 */
template <typename T1, typename E1, typename T2, typename E2>
inline constexpr auto operator== (Result<T1, E1> const& lhs, Result<T2, E2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<T1, T2> and std::equality_comparable_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() == rhs.value();

        return lhs.error() == rhs.error();
    }

    return false;
}

/*!
 * The inequality relation between two result objects containing comparable value
 * and error objects. The following rules apply:
 *   - If one result contains a value but the other result contains an error,
 *     these objects are not equal
 *   - If both results contain either values or errors then they are not equal if
 *     the values or errors are not equal respectively
 *
 * @tparam T1 the type of value in the first result object
 * @tparam E1 the type of error in the first result object
 * @tparam T2 the type of value in the second result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the two results are not equal, `false` otherwise
 */
template <typename T1, typename E1, typename T2, typename E2>
inline constexpr auto operator!= (Result<T1, E1> const& lhs, Result<T2, E2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<T1, T2> and std::equality_comparable_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() != rhs.value();

        return lhs.error() != rhs.error();
    }

    return true;
}

/*!
 * The less than relation between the two result objects. The following rules
 * apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `true`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `false`
 *   - If both results contain either values or errors, then the result
 *     of this operator is the result of the less than operator applied to
 *     the values or errors respectively
 *
 * @tparam T1 the type of value in the first result object
 * @tparam E1 the type of error in the first result object
 * @tparam T2 the type of value in the second result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is less than the right result, `false` otherwise
 */
template <typename T1, typename E1, typename T2, typename E2>
inline constexpr auto operator< (Result<T1, E1> const& lhs, Result<T2, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> and std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() < rhs.value();

        return lhs.error() < rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) < static_cast<int>(rhs.hasValue());
}

/*!
 * The less than or equal relation between the two result objects. The following
 * rules apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `true`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `false`
 *   - If both results contain either values or errors, then the result
 *     of this operator is the result of the less than or equal operator applied
 *     to the values or errors respectively
 *
 * @tparam T1 the type of value in the first result object
 * @tparam E1 the type of error in the first result object
 * @tparam T2 the type of value in the second result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is less than or equal to the right
 * result, `false` otherwise
 */
template <typename T1, typename E1, typename T2, typename E2>
inline constexpr auto operator<= (Result<T1, E1> const& lhs, Result<T2, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> and std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() <= rhs.value();

        return lhs.error() <= rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) <= static_cast<int>(rhs.hasValue());
}

/*!
 * The greater than relation between the two result objects. The following rules
 * apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `false`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `true`
 *   - If both results contain either values or errors, then the result
 *     of this operator is the result of the greater than operator applied to
 *     the values or errors respectively
 *
 * @tparam T1 the type of value in the first result object
 * @tparam E1 the type of error in the first result object
 * @tparam T2 the type of value in the second result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is greater than the right result, `false` otherwise
 */
template <typename T1, typename E1, typename T2, typename E2>
inline constexpr auto operator> (Result<T1, E1> const& lhs, Result<T2, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> and std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() > rhs.value();

        return lhs.error() > rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) > static_cast<int>(rhs.hasValue());
}

/*!
 * The greater than or equal relation between the two result objects. The
 * following rules apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `false`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `true`
 *   - If both results contain either values or errors, then the result
 *     of this operator is the result of the greater than or equal operator
 *     applied to the values or errors respectively
 *
 * @tparam T1 the type of value in the first result object
 * @tparam E1 the type of error in the first result object
 * @tparam T2 the type of value in the second result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is greater than or equal to the right
 * result, `false` otherwise
 */
template <typename T1, typename E1, typename T2, typename E2>
inline constexpr auto operator>= (Result<T1, E1> const& lhs, Result<T2, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> and std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() >= rhs.value();

        return lhs.error() >= rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) >= static_cast<int>(rhs.hasValue());
}

//
// Result<void, E> comparisons
//

/*!
 * The equality relation between two result objects whose values are void and
 * whose errors are comparable. The following rules apply:
 *   - If one result contains a value but the other contains an error, these
 *     results are not equal
 *   - If both results contain values they are equal
 *   - If both results contain errors then they are equal if the errors are equal
 *
 * @tparam E1 the type of error in the first result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the two results are equal, `false` otherwise
 */
template <typename E1, typename E2>
inline constexpr auto operator== (Result<void, E1> const& lhs, Result<void, E2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return true;

        return lhs.error() == rhs.error();
    }

    return false;
}

/*!
 * The inequality relation between two result objects whose values are void and
 * whose errors are comparable. The following rules apply:
 *   - If one result contains a value but the other contains an error, these
 *     results are not equal
 *   - If both results contain values they are equal
 *   - If both results contain errors then they are not equal if the errors are
 *     not equal
 *
 * @tparam E1 the type of error in the first result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the two results are not equal, `false` otherwise
 */
template <typename E1, typename E2>
inline constexpr auto operator!= (Result<void, E1> const& lhs, Result<void, E2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return false;

        return lhs.error() != rhs.error();
    }

    return true;
}

/*!
 * The less than relation between the two result objects whose values are void and
 * whose errors are comparable. The following rules
 * apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `true`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `false`
 *   - If both results contain values this operator returns `false`
 *   - If both results contain errors, then the result of this operator is the
 *     result of the less than operator applied to the errors
 *
 * @tparam E1 the type of error in the first result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is less than the right result, `false` otherwise
 */
template <typename E1, typename E2>
inline constexpr auto operator< (Result<void, E1> const& lhs, Result<void, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return false;

        return lhs.error() < rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) < static_cast<int>(rhs.hasValue());
}

/*!
 * The less than or equal relation between the two result objects whose values
 * are void and whose errors are comparable. The following rules
 * apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `true`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `false`
 *   - If both results contain values this operator returns `true`
 *   - If both results contain errors, then the result of this operator is the
 *     result of the less than or equal to operator applied to the errors
 *
 * @tparam E1 the type of error in the first result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is less than the right result, `false` otherwise
 */
template <typename E1, typename E2>
inline constexpr auto operator<= (Result<void, E1> const& lhs, Result<void, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return true;

        return lhs.error() <= rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) <= static_cast<int>(rhs.hasValue());
}

/*!
 * The greater than relation between the two result objects whose values are
 * void and whose errors are comparable. The following rules
 * apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `false`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `true`
 *   - If both results contain values this operator returns `false`
 *   - If both results contain errors, then the result of this operator is the
 *     result of the greater than operator applied to the errors
 *
 * @tparam E1 the type of error in the first result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is greater than the right result, `false`
 * otherwise
 */
template <typename E1, typename E2>
inline constexpr auto operator> (Result<void, E1> const& lhs, Result<void, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return false;

        return lhs.error() > rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) > static_cast<int>(rhs.hasValue());
}

/*!
 * The greater than or equal relation between the two result objects whose
 * values are void and whose errors are comparable. The following rules
 * apply:
 *   - If the left result has an error and the right result has a value,
 *     this operator returns `false`
 *   - If the left result has a value and the right result has an error,
 *     this operator returns `true`
 *   - If both results contain values this operator returns `true`
 *   - If both results contain errors, then the result of this operator is the
 *     result of the greater than or equal operator applied to the errors
 *
 * @tparam E1 the type of error in the first result object
 * @tparam E2 the type of error in the second result object
 * @param lhs the left result
 * @param rhs the right result
 * @return `true` if the left result is greater than or equal to the right
 * result, `false` otherwise
 */
template <typename E1, typename E2>
inline constexpr auto operator>= (Result<void, E1> const& lhs, Result<void, E2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E1, E2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return true;

        return lhs.error() >= rhs.error();
    }

    return static_cast<int>(lhs.hasValue()) >= static_cast<int>(rhs.hasValue());
}



//
// Comparison of Result<T, E> with a value
//

/*!
 * The equality relation between a result and a value comparable to the value of
 * the result. The following rules apply:
 *   - If the result contains an error, this operator returns `false`
 *   - If the result contains a value, this operator returns `true` if the
 *     value of the result is equal to the specified value, false otherwise
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam V the type of the \p rhs
 * @param lhs the result
 * @param rhs the value
 * @return `true` if the result contains a value and that value is equal to \p rhs,
 * `false` otherwise
 */
template <typename T, typename E, typename V>
inline constexpr auto operator== (Result<T, E> const& lhs, V const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::equality_comparable_with<T, V>) {
    return lhs.hasValue() and lhs.value() == rhs;
}

/*!
 * The equality relation between a value and a result whose value is comparable
 * to the value on the left. The following rules apply:
 *   - If the result contains an error, this operator returns `false`
 *   - If the result contains a value, this operator returns `true` if the
 *     value of the result is equal to the specified value, false otherwise
 *
 * @tparam V the type of the \p lhs
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the value
 * @param rhs the result
 * @return `true` if the result contains a value and that value is equal to \p lhs,
 * `false` otherwise
 */
template <typename V, typename T, typename E>
inline constexpr auto operator== (V const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::equality_comparable_with<T, V>) {
    return rhs.hasValue() and lhs == rhs.value();
}

/*!
 * The inequality relation between a result and a value comparable to the value of
 * the result. The following rules apply:
 *   - If the result contains an error, this operator returns `true`
 *   - If the result contains a value, this operator returns `true` if the
 *     value of the result is not equal to the specified value, false otherwise
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam V the type of the \p rhs
 * @param lhs the result
 * @param rhs the value
 * @return `true` if the result contains an error or if it contains a value then
 * this value is not equal to \p rhs, `false` otherwise
 */
template <typename T, typename E, typename V>
inline constexpr auto operator!= (Result<T, E> const& lhs, V const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::equality_comparable_with<T, V>) {
    return lhs.hasError() or lhs.value() != rhs;
}

/*!
 * The inequality relation between a result and a value comparable to the value of
 * the result. The following rules apply:
 *   - If the result contains an error, this operator returns `true`
 *   - If the result contains a value, this operator returns `true` if the
 *     value of the result is not equal to the specified value, false otherwise
 *
 * @tparam V the type of the \p lhs
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the value
 * @param rhs the result
 * @return `true` if the result contains an error or if it contains a value then
 * this value is not equal to \p lhs, `false` otherwise
 */
template <typename V, typename T, typename E>
inline constexpr auto operator!= (V const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::equality_comparable_with<T, V>) {
    return rhs.hasError() or lhs != rhs.value();
}

/*!
 * The less than relation between a result and a value comparable to the value
 * of the result. The following rules apply:
 *   - If the result has an error this operator returns `true`
 *   - If the result has a value, then the result of this operator is the
 *     result of the less than operator applied to the value contained in
 *     the result and \p rhs
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam V the type of the \p rhs
 * @param lhs the result
 * @param rhs the value
 * @return `true` if the result contains an error or if its value is less
 * than \p rhs, `false` otherwise
 */
template <typename T, typename E, typename V>
inline constexpr auto operator< (Result<T, E> const& lhs, V const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return lhs.hasError() or lhs.value() < rhs;
}

/*!
 * The less than relation between a value and a result whose value is comparable
 * to the value on the left. The following rules apply:
 *   - If the result has an error this operator returns `false`
 *   - If the result has a value, then the result of this operator is the
 *     result of the less than operator applied to \p lhs and the value contained
 *     in the result
 *
 * @tparam V the type of the \p lhs
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the value
 * @param rhs the result
 * @return `true` if the result contains a value and this value is greater
 * than \p lhs, `false` otherwise
 */
template <typename V, typename T, typename E>
inline constexpr auto operator< (V const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return rhs.hasValue() and lhs < rhs.value();
}

/*!
 * The less than or equal relation between a result and a value comparable
 * to the value of the result. The following rules apply:
 *   - If the result has an error this operator returns `true`
 *   - If the result has a value, then the result of this operator is the
 *     result of the less than or equal operator applied to the value contained
 *     in the result and \p rhs
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam V the type of the \p rhs
 * @param lhs the result
 * @param rhs the value
 * @return `true` if the result contains an error or if its value is less
 * than or equal to \p rhs, `false` otherwise
 */
template <typename T, typename E, typename V>
inline constexpr auto operator<= (Result<T, E> const& lhs, V const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return lhs.hasError() or lhs.value() <= rhs;
}

/*!
 * The less than or equal relation between a value and a result whose value is
 * comparable to the value on the left. The following rules apply:
 *   - If the result has an error this operator returns `false`
 *   - If the result has a value, then the result of this operator is the
 *     result of the less than or equal operator applied to \p lhs and the value
 *     contained in the result
 *
 * @tparam V the type of the \p lhs
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the value
 * @param rhs the result
 * @return `true` if the result contains a value and this value is greater
 * than or equal to \p lhs, `false` otherwise
 */
template <typename V, typename T, typename E>
inline constexpr auto operator<= (V const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return rhs.hasValue() and lhs <= rhs.value();
}

/*!
 * The greater than relation between a result and a value comparable to the value
 * of the result. The following rules apply:
 *   - If the result has an error this operator returns `false`
 *   - If the result has a value, then the result of this operator is the
 *     result of the greater than operator applied to the value contained in
 *     the result and \p rhs
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam V the type of the \p rhs
 * @param lhs the result
 * @param rhs the value
 * @return `true` if the result contains a value and this value is greater
 * than \p rhs, `false` otherwise
 */
template <typename T, typename E, typename V>
inline constexpr auto operator> (Result<T, E> const& lhs, V const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return lhs.hasValue() and lhs.value() > rhs;
}

/*!
 * The greater than relation between a value and a result whose value is
 * comparable to the value on the left. The following rules apply:
 *   - If the result has an error this operator returns `true`
 *   - If the result has a value, then the result of this operator is the
 *     result of the greater than operator applied to \p lhs and the value
 *     contained in the result
 *
 * @tparam V the type of the \p lhs
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the value
 * @param rhs the result
 * @return `true` if the result contains an error or its value is less
 * than \p lhs, `false` otherwise
 */
template <typename V, typename T, typename E>
inline constexpr auto operator> (V const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return rhs.hasError() or lhs > rhs.value();
}

/*!
 * The greater than or equal relation between a result and a value comparable
 * to the value of the result. The following rules apply:
 *   - If the result has an error this operator returns `false`
 *   - If the result has a value, then the result of this operator is the
 *     result of the greater than or equal operator applied to the value
 *     contained in the result and \p rhs
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam V the type of the \p rhs
 * @param lhs the result
 * @param rhs the value
 * @return `true` if the result contains a value and this value is greater
 * than or equal to \p rhs, `false` otherwise
 */
template <typename T, typename E, typename V>
inline constexpr auto operator>= (Result<T, E> const& lhs, V const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return lhs.hasValue() and lhs.value() >= rhs;
}

/*!
 * The greater than or equal relation between a value and a result whose
 * value is comparable to the value on the left. The following rules apply:
 *   - If the result has an error this operator returns `true`
 *   - If the result has a value, then the result of this operator is the
 *     result of the greater than or equal operator applied to \p lhs and
 *     the value contained in the result
 *
 * @tparam V the type of the \p lhs
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the value
 * @param rhs the result
 * @return `true` if the result contains an error or its value is less
 * than or equal to \p lhs, `false` otherwise
 */
template <typename V, typename T, typename E>
inline constexpr auto operator>= (V const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires (not std::same_as<T, void> and
          not ResultType<V> and
          std::totally_ordered_with<T, V>) {
    return rhs.hasError() or lhs >= rhs.value();
}

//
// Comparison of Result<T, E> with Failure<X>
//

/*!
 * The equality relation between a result and a failure, where the error of
 * the result is comparable to the value in the failure:
 *   - If the result contains a value, this operator returns `false`
 *   - If the result contains an error, this operator returns `true` if the
 *     error of the result is equal to value of the failure
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam X the type of the value of the failure
 * @param lhs the result
 * @param rhs the failure
 * @return `true` if the result contains an error and that error is equal to
 * the value in the failure
 */
template <typename T, typename E, typename X>
inline constexpr auto operator== (Result<T, E> const& lhs, Failure<X> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E, X> {
    return lhs.hasError() and lhs.error() == rhs.error();
}

/*!
 * The equality relation between a failure and a result, where the value of
 * the failure is comparable to the error in the result:
 *   - If the result contains a value, this operator returns `false`
 *   - If the result contains an error, this operator returns `true` if the
 *     error of the result is equal to value of the failure
 *
 * @tparam X the type of the value of the failure
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the failure
 * @param rhs the result
 * @return `true` if the result contains an error and that error is equal to
 * the value in the failure
 */
template <typename X, typename T, typename E>
inline constexpr auto operator== (Failure<X> const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E, X> {
    return rhs.hasError() and lhs.error() == rhs.error();
}

/*!
 * The inequality relation between a result and a failure, where the error of
 * the result is comparable to the value in the failure:
 *   - If the result contains a value, this operator returns `true`
 *   - If the result contains an error, this operator returns `true` if the
 *     error of the result is not equal to value of the failure
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam X the type of the value of the failure
 * @param lhs the result
 * @param rhs the failure
 * @return `true` if the result contains a value or if its error is not equal to
 * the value in the failure
 */
template <typename T, typename E, typename X>
inline constexpr auto operator!= (Result<T, E> const& lhs, Failure<X> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E, X> {
    return lhs.hasValue() or lhs.error() != rhs.error();
}

/*!
 * The inequality relation between a failure and a result, where the value of
 * the failure is comparable to the error in the result:
 *   - If the result contains a value, this operator returns `true`
 *   - If the result contains an error, this operator returns `true` if the
 *     error of the result is not equal to value of the failure
 *
 * @tparam X the type of the value of the failure
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the failure
 * @param rhs the result
 * @return `true` if the result contains a value or if its error is not equal to
 * the value in the failure
 */
template <typename X, typename T, typename E>
inline constexpr auto operator!= (Failure<X> const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<E, X> {
    return rhs.hasValue() or lhs.error() != rhs.error();
}

/*!
 * The less than relation between a result and a failure, where the error of
 * the result is comparable to the value of the failure. The following rules
 * apply:
 *   - If the result has a value this operator returns `false`
 *   - If the result has an error, then the result of this operator is the
 *     result of the less than operator applied to the error contained in
 *     the result and the value contained in the failure
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam X the type of the value of the failure
 * @param lhs the result
 * @param rhs the failure
 * @return `true` if the result contains an error and this error is less
 * than the value of the failure, `false` otherwise
 */
template <typename T, typename E, typename X>
inline constexpr auto operator< (Result<T, E> const& lhs, Failure<X> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return lhs.hasError() and lhs.error() < rhs.error();
}

/*!
 * The less than relation between a failure and a result, where the value of
 * the failure is comparable to the error of the result. The following rules
 * apply:
 *   - If the result has a value this operator returns `true`
 *   - If the result has an error, then the result of this operator is the
 *     result of the less than operator applied to the value contained in
 *     the failure and the error contained in the result
 *
 * @tparam X the type of the value of the failure
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the failure
 * @param rhs the result
 * @return `true` if the result contains a value or if its error is greater
 * than the value of the failure, `false` otherwise
 */
template <typename X, typename T, typename E>
inline constexpr auto operator< (Failure<X> const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return rhs.hasValue() or lhs.error() < rhs.error();
}

/*!
 * The less than or equal relation between a result and a failure, where the
 * error of the result is comparable to the value of the failure. The following
 * rules apply:
 *   - If the result has a value this operator returns `false`
 *   - If the result has an error, then the result of this operator is the
 *     result of the less than or equal operator applied to the error contained
 *     in the result and the value contained in the failure
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam X the type of the value of the failure
 * @param lhs the result
 * @param rhs the failure
 * @return `true` if the result contains an error and this error is less
 * than or equal to the value of the failure, `false` otherwise
 */
template <typename T, typename E, typename X>
inline constexpr auto operator<= (Result<T, E> const& lhs, Failure<X> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return lhs.hasError() and lhs.error() <= rhs.error();
}

/*!
 * The less than or equal relation between a failure and a result, where the
 * value of the failure is comparable to the error of the result. The following
 * rules apply:
 *   - If the result has a value this operator returns `true`
 *   - If the result has an error, then the result of this operator is the
 *     result of the less than or equal operator applied to the value contained
 *     in the failure and the error contained in the result
 *
 * @tparam X the type of the value of the failure
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the failure
 * @param rhs the result
 * @return `true` if the result contains a value or if its error is greater
 * than or equal to the value of the failure, `false` otherwise
 */
template <typename X, typename T, typename E>
inline constexpr auto operator<= (Failure<X> const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return rhs.hasValue() or lhs.error() <= rhs.error();
}

/*!
 * The greater than relation between a result and a failure, where the error of
 * the result is comparable to the value of the failure. The following rules
 * apply:
 *   - If the result has a value this operator returns `true`
 *   - If the result has an error, then the result of this operator is the
 *     result of the greater than operator applied to the error contained in
 *     the result and the value contained in the failure
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam X the type of the value of the failure
 * @param lhs the result
 * @param rhs the failure
 * @return `true` if the result contains a value or if its error is greater
 * than the value of the failure, `false` otherwise
 */
template <typename T, typename E, typename X>
inline constexpr auto operator> (Result<T, E> const& lhs, Failure<X> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return lhs.hasValue() or lhs.error() > rhs.error();
}

/*!
 * The greater than relation between a failure and a result, where the value of
 * the failure is comparable to the error of the result. The following rules
 * apply:
 *   - If the result has a value this operator returns `false`
 *   - If the result has an error, then the result of this operator is the
 *     result of the greater than operator applied to the value contained in
 *     the failure and the error contained in the result
 *
 * @tparam X the type of the value of the failure
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the failure
 * @param rhs the result
 * @return `true` if the result contains an error and this error is less
 * than the value of the failure, `false` otherwise
 */
template <typename X, typename T, typename E>
inline constexpr auto operator> (Failure<X> const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return rhs.hasError() and lhs.error() > rhs.error();
}

/*!
 * The greater than or equal relation between a result and a failure, where the
 * error of the result is comparable to the value of the failure. The following
 * rules apply:
 *   - If the result has a value this operator returns `true`
 *   - If the result has an error, then the result of this operator is the
 *     result of the greater than or equal operator applied to the error contained
 *     in the result and the value contained in the failure
 *
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @tparam X the type of the value of the failure
 * @param lhs the result
 * @param rhs the failure
 * @return `true` if the result contains a value or if its error is greater
 * than or equal to the value of the failure, `false` otherwise
 */
template <typename T, typename E, typename X>
inline constexpr auto operator>= (Result<T, E> const& lhs, Failure<X> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return lhs.hasValue() or lhs.error() >= rhs.error();
}

/*!
 * The greater than or equal relation between a failure and a result, where the
 * value of the failure is comparable to the error of the result. The following
 * rules apply:
 *   - If the result has a value this operator returns `false`
 *   - If the result has an error, then the result of this operator is the
 *     result of the greater than or equal operator applied to the value contained
 *     in the failure and the error contained in the result
 *
 * @tparam X the type of the value of the failure
 * @tparam T the type of value of the result
 * @tparam E the type of error of the result
 * @param lhs the failure
 * @param rhs the result
 * @return `true` if the result contains an error and this error is less
 * than or equal to the value of the failure, `false` otherwise
 */
template <typename X, typename T, typename E>
inline constexpr auto operator>= (Failure<X> const& lhs, Result<T, E> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<E, X> {
    return rhs.hasError() and lhs.error() >= rhs.error();
}

//
// swap
//

/*!
 * Swaps the two result objects.
 * @tparam T the value type of each result object
 * @tparam E the error type of each result object
 * @param lhs the left result
 * @param rhs the right result
 */
template <typename T, typename E>
inline auto swap(Result<T, E>& lhs, Result<T, E>& rhs)
noexcept(std::is_nothrow_move_constructible_v<Result<T, E>> and
         std::is_nothrow_move_assignable_v<Result<T, E>> and
         std::is_nothrow_swappable_v<T> and
         std::is_nothrow_swappable_v<E>) -> void {
    using std::swap;

    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue()) swap(lhs.value(), rhs.value());
        else swap(lhs.error(), rhs.error());
    } else {
        auto tmp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(tmp);
    }
}

/*!
 * Swaps the two result object, where the value type of each result is `void`.
 * @tparam E the error type of each result object
 * @param lhs the left result
 * @param rhs the right result
 */
template <typename E>
inline auto swap(Result<void, E>& lhs, Result<void, E>& rhs)
noexcept(std::is_nothrow_move_constructible_v<Result<void, E>> and
         std::is_nothrow_move_assignable_v<Result<void, E>> and
         std::is_nothrow_swappable_v<E>) -> void {
    using std::swap;

    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasError()) swap(lhs.error(), rhs.error());
        // and do nothing if both Results are in the value state
    } else {
        auto tmp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(tmp);
    }
}

} // namespace pimc

#pragma GCC diagnostic pop
