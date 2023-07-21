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

template<typename>
class Optional;

// traits

template <typename T>
struct IsOptional: std::false_type {};

template <typename U>
struct IsOptional<Optional<U>>: std::true_type {
    using ValueType = U;
};

template <typename V>
concept OptionalType = IsOptional<V>::value and
        not std::same_as<typename IsOptional<V>::ValueType, void>;

namespace detail {

template <typename V,
        bool TriviallyDestructible = std::is_trivially_destructible_v<V>>
struct OptionalAlt {
    using ValueType = WrappedValueType<V>;

    union {
        ValueType value_;
        std::monostate empty_;
    };

    bool hasValue_;

    explicit constexpr OptionalAlt(std::monostate) noexcept
    : empty_{} {}

    template <typename ... Ts>
    constexpr OptionalAlt(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<V, Ts...>)
    : value_{std::forward<Ts>(args)...}, hasValue_{true} {}

    constexpr OptionalAlt(InPlaceEmptyType)
    : empty_{}, hasValue_{false} {}

    OptionalAlt(OptionalAlt const&) = default;
    OptionalAlt(OptionalAlt&&) = default;

    auto destroy() const noexcept -> void {}

    auto operator= (OptionalAlt const&) -> OptionalAlt& = default;
    auto operator= (OptionalAlt&&) -> OptionalAlt& = default;
};

template <typename V>
struct OptionalAlt<V, false> {
    using ValueType = WrappedValueType<V>;

    union {
        ValueType value_;
        std::monostate empty_;
    };

    bool hasValue_;

    explicit constexpr OptionalAlt(std::monostate) noexcept
    : empty_{} {}

    template <typename ... Ts>
    constexpr OptionalAlt(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<V, Ts...>)
            : value_{std::forward<Ts>(args)...}, hasValue_{true} {}

    constexpr OptionalAlt(InPlaceEmptyType)
            : empty_{}, hasValue_{false} {}

    OptionalAlt(OptionalAlt const&) = default;
    OptionalAlt(OptionalAlt&&) = default;

    auto destroy()
    noexcept(std::is_nothrow_destructible_v<V>)
    -> void {
        if (hasValue_) {
            value_.~ValueType();
            hasValue_ = false;
        }
    }

    ~OptionalAlt()
    noexcept(std::is_nothrow_destructible_v<V>) {
        destroy();
    }

    auto operator= (OptionalAlt const&) -> OptionalAlt& = default;
    auto operator= (OptionalAlt&&) -> OptionalAlt& = default;
};

template <typename T>
struct OptionalStorage {
    using StorageType = OptionalAlt<T>;
    StorageType storage_;

    explicit constexpr OptionalStorage(std::monostate) noexcept
    : storage_{std::monostate{}} {}

    template <typename ... Ts>
    constexpr OptionalStorage(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, Ts...>)
    : storage_{InPlaceValue, std::forward<Ts>(args)...} {}

    constexpr OptionalStorage(InPlaceEmptyType) noexcept
    : storage_{InPlaceEmpty} {}

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

    template <typename R>
    auto constructFromOptional(R&& r) -> void {
        if (r.storage_.hasValue_) {
            using ValueType = typename StorageType::ValueType;
            auto* p = static_cast<void*>(std::addressof(storage_.value_));
            auto* vp = reinterpret_cast<ValueType*>(p);

            if constexpr (std::is_lvalue_reference_v<T>)
                std::construct_at(vp, std::forward<R>(r).storage_.value_.get());
            else std::construct_at(vp, std::forward<R>(r).storage_.value_);
            storage_.hasValue_ = true;
        } else storage_.hasValue_ = false;
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

    template <typename R>
    auto assignFromOptional(R&& r) -> void {
        if (storage_.hasValue_ != r.storage_.hasValue_) {
            storage_.destroy();
            constructFromOptional(std::forward<R>(r));
        } else if (storage_.hasValue_) {
            if constexpr (std::is_lvalue_reference_v<T>)
                storage_.value_ = std::forward<R>(r).storage_.value_.get();
            else storage_.value_ = std::forward<R>(r).storage_.value_;
        }
    }

    OptionalStorage(OptionalStorage const&) noexcept
    requires std::is_trivially_copy_constructible_v<T> = default;

    OptionalStorage(OptionalStorage const& rhs)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
    requires (std::is_copy_constructible_v<T>
              and not std::is_trivially_copy_constructible_v<T>)
            : storage_{std::monostate{}} {
        constructFromOptional(rhs);
    }

    OptionalStorage(OptionalStorage const&)
    requires (not std::is_copy_constructible_v<T>) = delete;

    OptionalStorage(OptionalStorage&&) noexcept
    requires std::is_trivially_move_constructible_v<T> = default;

    OptionalStorage(OptionalStorage&& rhs)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    requires (std::is_move_constructible_v<T>
              and not std::is_trivially_move_constructible_v<T>)
            : storage_{std::monostate{}} {
        constructFromOptional(std::move(rhs));
    }

    OptionalStorage(OptionalStorage&&)
    requires (not std::is_move_constructible_v<T>) = delete;

    auto operator= (OptionalStorage const&)
    noexcept -> OptionalStorage&
    requires std::is_trivially_copy_constructible_v<T> and
             std::is_trivially_copy_assignable_v<T> and
             std::is_trivially_destructible_v<T>
    = default;

    auto operator= (OptionalStorage const& rhs) -> OptionalStorage&
    requires (not std::is_copy_constructible_v<T>) or
             (not std::is_copy_assignable_v<T>) or
             (not std::is_destructible_v<T>)
    = delete;

    auto operator= (OptionalStorage const& rhs)
    noexcept(std::is_nothrow_copy_constructible_v<T> and
             std::is_nothrow_copy_assignable_v<T>) -> OptionalStorage&
    requires std::is_copy_constructible_v<T> and (
             (std::is_copy_assignable_v<T> and std::is_destructible_v<T>
                    and (not std::is_trivially_copy_assignable_v<T>)) or
             (std::is_trivially_copy_assignable_v<T> and std::is_destructible_v<T>
                     and (not std::is_trivially_destructible_v<T>))) {
        assignFromOptional(rhs);
        return *this;
    }

    auto operator= (OptionalStorage&&)
    noexcept -> OptionalStorage&
    requires std::is_trivially_move_constructible_v<T> and
             std::is_trivially_move_assignable_v<T> and
             std::is_trivially_destructible_v<T>
    = default;

    auto operator= (OptionalStorage const& rhs) -> OptionalStorage&
    requires (not std::is_move_constructible_v<T>) or
             (not std::is_move_assignable_v<T>) or
             (not std::is_destructible_v<T>)
    = delete;

    auto operator= (OptionalStorage&& rhs)
    noexcept(std::is_nothrow_move_constructible_v<T> and
             std::is_nothrow_move_assignable_v<T>) -> OptionalStorage&
    requires std::is_move_constructible_v<T> and (
            (std::is_move_assignable_v<T> and std::is_destructible_v<T>
             and (not std::is_trivially_move_assignable_v<T>)) or
            (std::is_trivially_move_assignable_v<T> and std::is_destructible_v<T>
             and (not std::is_trivially_destructible_v<T>))) {
        assignFromOptional(std::move(rhs));
        return *this;
    }
};

template <typename T1, typename T2>
concept ConvertibleOptional =
        // T1 constructible from Optional<T2, E2>
        std::constructible_from<T1, Optional<T2>&> or
        std::constructible_from<T1, Optional<T2> const&> or
        std::constructible_from<T1, Optional<T2>&&> or
        std::constructible_from<T1, Optional<T2> const&&> or
        // Optional<T2> convertible to T1
        std::convertible_to<Optional<T2>&, T1> or
        std::convertible_to<Optional<T2> const&, T1> or
        std::convertible_to<Optional<T2>&&, T1> or
        std::convertible_to<Optional<T2> const&&, T1>;

template <typename T1, typename T2>
concept CopyConvertibleOptional =
        not ConvertibleOptional<T1, T2> and
        std::constructible_from<T1, T2 const&>;

template <typename T1, typename T2>
concept ImplicitlyCopyConvertibleOptional =
        CopyConvertibleOptional<T1, T2> and
        std::convertible_to<T2 const&, T1>;

template <typename T1, typename T2>
concept ExplicitlyCopyConvertibleOptional =
        CopyConvertibleOptional<T1, T2> and
        (not std::convertible_to<T2 const&, T1>);

template <typename T1, typename T2>
concept MoveConvertibleOptional =
        not ConvertibleOptional<T1, T2> and
        std::constructible_from<T1, T2&&>;

template <typename T1, typename T2>
concept ImplicitlyMoveConvertibleOptional =
        MoveConvertibleOptional<T1, T2> and
        std::convertible_to<T2&&, T1>;

template <typename T1, typename T2>
concept ExplicitlyMoveConvertibleOptional =
        MoveConvertibleOptional<T1, T2> and
        (not std::convertible_to<T2&&, T1>);

template <typename TT, typename T>
concept OptionalValueConvertibleTo =
        std::constructible_from<T, TT&&> and
        not std::same_as<std::decay_t<TT>, std::in_place_t> and
        not std::same_as<std::decay_t<TT>, InPlaceValueType> and
        not std::same_as<std::decay_t<TT>, InPlaceErrorType> and
        not IsOptional<std::decay_t<TT>>::value;

template <typename TT, typename T>
concept ImplicitlyOptionalValueConvertibleTo =
        OptionalValueConvertibleTo<TT, T> and
        std::convertible_to<TT&&, T>;

template <typename TT, typename T>
concept ExplicitlyOptionalValueConvertibleTo =
        OptionalValueConvertibleTo<TT, T> and
        not std::convertible_to<TT&&, T>;

template <typename T1, typename T2>
concept ConvertAssignableOptional =
        ConvertibleOptional<T1, T2> and
        std::assignable_from<T1&, Optional<T2>&> and
        std::assignable_from<T1&, Optional<T2> const&> and
        std::assignable_from<T1&, Optional<T2>&&> and
        std::assignable_from<T1&, Optional<T2> const&&>;

template <typename T1, typename T2>
concept CopyConvertAssignableOptional =
not ConvertAssignableOptional<T1, T2> and
        std::constructible_from<T1, T2 const&> and
        std::assignable_from<WrappedValueType<T1>&, T2 const&>;

template <typename T1, typename T2>
concept MoveConvertAssignableOptional =
not ConvertAssignableOptional<T1, T2> and
        std::constructible_from<T1, T2&&> and
        std::assignable_from<T1&, T2&&>;

template <typename TT, typename T>
concept OptionalValueAssignableTo =
not IsOptional<std::decay_t<TT>>::value and
        std::constructible_from<T, TT> and
        std::assignable_from<WrappedValueType<T>&, TT> and (
                not std::same_as<std::decay_t<T>, std::decay_t<TT>> or
                not std::is_scalar_v<T>
        );

template <typename F, typename T>
concept OptionalMapper =
        std::invocable<F, T> and
        not std::same_as<void, std::invoke_result_t<F, T>>;
} // namespace detail

/*!
 * A result object, which at any given time contains either a valid result of
 * a computation or an error indicating that the computation failed.
 *
 * @tparam T the type of the result
 * @tparam E the type of the error
 */
template <typename T>
class Optional {
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
            not IsOptional<std::decay_t<T>>::value,
            "Value type may not be Optional type");

    static_assert(
            not std::is_rvalue_reference_v<T>,
            "Value type may not be an rvalue reference, "
            "only lvalue references are allowed");

    template <typename T2>
    friend class Optional;

public:
    using ValueType = T;

public:

    /*!
     * \brief Constructs an empty optional value.
     *
     * Usage:
     *
     * ```cpp
     * auto r = pimc::Optional<int>{};
     * ```
     *
     * @tparam U the value type
     */
    constexpr Optional() noexcept: value_{InPlaceEmpty} {}

    /*!
     * \brief Copy constructs a Optional from \p rhs.
     *
     * If \p rhs contains a value, the constructed copy will contain a
     * a copy of the value.
     *
     * If \p rhs contains an error, the constructed copy will contain a
     * copy of the error.
     *
     * @param rhs the Optional to copy
     */
    constexpr Optional(Optional const& rhs) = default;

    /*!
     * Move constructs a Optional from \p rhs.
     *
     * If \p rhs contains a value, move initializes the value of the constructed
     * Optional from the value contained in \p rhs, but does not make \p rhs empty.
     *
     * If \p rhs contains an error, the constructed copy will contain the
     * error moved from \p rhs.
     *
     * @param rhs the Optional to move
     */
    constexpr Optional(Optional&& rhs) = default;

    /*!
     * \brief Converting copy constructor.
     *
     * Usage:
     *
     * ```cpp
     * auto const r1 = pimc::Optional<int, int>{1};
     * auto const r2 = pimc::Optional<long, long>{r1};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Optional to convert
     */
    template <typename T2>
    /* implicit */ Optional(Optional<T2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2 const&>)
    requires detail::ImplicitlyCopyConvertibleOptional<T, T2>
            : value_{std::monostate{}} {
        value_.constructFromOptional(rhs.value_);
    }

    /*!
     * \brief Converting copy constructor.
     *
     * Usage:
     *
     * ```cpp
     * auto const r1 = pimc::Optional<int, int>{1};
     * auto const r2 = pimc::Optional<long, long>{r1};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Optional to convert
     */
    template <typename T2>
    explicit Optional(Optional<T2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<T, const T2&>)
    requires detail::ExplicitlyCopyConvertibleOptional<T, T2>
            : value_{std::monostate{}} {
        value_.constructFromOptional(rhs.value_);
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
     * auto r1 = pimc::Optional<std::unique_ptr<B>>{std::make_unique<B>()};
     * auto r2 = pimc::Optional<std::unique_ptr<A>>{std::move(r1)};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Optional to move convert
     */
    template <typename T2>
    /* implicit */ Optional(Optional<T2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2&&>)
    requires detail::ImplicitlyMoveConvertibleOptional<T, T2>
            : value_{std::monostate{}} {
        value_.constructFromOptional(std::move(rhs).value_);
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
     * auto r1 = pimc::Optional<std::unique_ptr<B>>{std::make_unique<B>()};
     * auto r2 = pimc::Optional<std::unique_ptr<A>>{std::move(r1)};
     * ```
     *
     * @tparam T2 the value type of \p rhs
     * @param rhs the Optional to move convert
     */
    template <typename T2>
    explicit Optional(Optional<T2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2&&>)
    requires detail::ExplicitlyMoveConvertibleOptional<T, T2>
            : value_{std::monostate{}} {
        value_.constructFromOptional(std::move(rhs).value_);
    }

    /*!
     * \brief Constructs a Optional object that contains a value.
     *
     * The value is constructed from the specified arguments.
     *
     * Example:
     *
     * ```cpp
     * auto r = pimc::Optional<std::string, int>{pimc::InPlaceValue, "abc"};
     * ```
     *
     * @tparam Ts the types of the arguments to the value constructor
     * @param args the arguments to the value constructor
     */
    template <typename ... Ts>
    constexpr explicit Optional(InPlaceValueType, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, Ts...>)
            : value_{InPlaceValue, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Optional which contains a value.
     *
     * The value is constructed from `std::initializer_list<U>` and \p args.
     *
     * Example:
     *
     * ```cpp
     * pimc::Optional<std::string> r{pimc::InPlaceValue, {'a', 'b', 'c'}};
     * ```
     *
     * @tparam U the type of the value in the initializer list
     * @tparam Ts the types of the optional arguments that follow the initializer list
     * @param il the initializer list
     * @param args the optional arguments supplied after the initializer list
     */
    template <typename U, typename ... Ts>
    constexpr explicit Optional(
            InPlaceValueType, std::initializer_list<U> il, Ts&& ... args)
    noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Ts...>)
    requires std::constructible_from<T, std::initializer_list<U>, Ts...>
            : value_{InPlaceValue, il, std::forward<Ts>(args)...} {}

    /*!
     * \brief Constructs a Optional that contains a value.
     *
     * The value of the constructed result is constructed from \p value.
     *
     * Example:
     *
     * ```cpp
     * pimc Optional<int, bool> r = 100;
     *
     * auto getValue() -> pimc::Optional<std::string, int> {
     *     char const* msg = "message";
     *     return msg;
     * }
     * ```
     *
     * @tparam U the type of the argument to this constructor
     * @param value the value convertible to the value contained in the result.
     */
    template <detail::ImplicitlyOptionalValueConvertibleTo<T> U>
    constexpr /* implicit */ Optional(U&& value)
    noexcept(std::is_nothrow_constructible_v<T, U>)
            : value_{InPlaceValue, std::forward<U>(value)} {}

    /*!
     * \brief Constructs a Optional that contains a value.
     *
     * The value of the constructed result is constructed from \p value.
     *
     * Example:
     *
     * ```cpp
     * pimc Optional<int> r = 100;
     *
     * auto getValue() -> pimc::Optional<std::string> {
     *     char const* msg = "message";
     *     return msg;
     * }
     * ```
     *
     * @tparam U the type of the argument to this constructor
     * @param value the value convertible to the value contained in the result.
     */
    template <detail::ExplicitlyOptionalValueConvertibleTo<T> U>
    constexpr explicit Optional(U&& value)
    noexcept(std::is_nothrow_constructible_v<T, U>)
            : value_{InPlaceValue, std::forward<U>(value)} {}

    /*!
     * The copy assignment operator.
     *
     * @param rhs the Optional to copy assign
     * @return a reference to this Optional
     */
    auto operator= (Optional const& rhs) -> Optional& = default;

    /*!
     * The move assignment operator.
     *
     * @param rhs the Optional to move assign
     * @return a reference to this Optional
     */
    auto operator= (Optional&& rhs)
    noexcept(std::is_nothrow_assignable_v<T&, T&&>)
    -> Optional& = default;

    /*!
     * \brief Copy constructs the state of \p rhs.
     *
     * @tparam T2 the value type of \p rhs
     * @tparam E2 the error type of \p rhs
     * @param rhs the Optional whose state is copied into this Optional
     * @return a reference to this Optional
     */
    template <typename T2>
    auto operator= (Optional<T2> const& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2 const&> and
             std::is_nothrow_assignable_v<T&, T2 const&>) -> Optional&
    requires detail::CopyConvertAssignableOptional<T, T2> {
        value_.assignFromOptional(rhs.value_);
        return *this;
    }

    /*!
     * \brief Move constructs the state of \p rhs.
     *
     * @tparam T2 the value type of \p rhs
     * @param rhs the Optional whose state is moved into this Optional
     * @return a reference to this Optional
     */
    template <typename T2>
    auto operator= (Optional<T2>&& rhs)
    noexcept(std::is_nothrow_constructible_v<T, T2 const&> and
             std::is_nothrow_assignable_v<T&, T2&&>) -> Optional&
    requires detail::MoveConvertAssignableOptional<T, T2> {
        value_.assignFromOptional(std::move(rhs).value_);
        return *this;
    }

    /*!
     * \brief Perfect-forwarding value assignment operator.
     *
     * @tparam U the value type convertible to the value type of this Optional
     * @param value the value to perfect forward to the value of this Optional
     * @return a reference to this Optional
     */
    template <detail::OptionalValueAssignableTo<T> U>
    auto operator= (U&& value)
    noexcept(std::is_nothrow_constructible_v<T, U> and
             std::is_nothrow_assignable_v<T&, U>) -> Optional& {
        value_.assignValue(std::forward<U>(value));
        return *this;
    }

    /*!
     * \brief Returns an lvalue reference to the contained value.
     *
     * This operators is similar to the same operator in `std::optional` for
     * cases where the Optional contains a value.
     *
     * \warning The behavior of this operator is undefined if the Optional does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Optional<std::string> r{"xyz"s};
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
     * cases where the Optional contains a value.
     *
     * \warning The behavior of this operator is undefined if the Optional does
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
     * cases where the Optional contains a value.
     *
     * \warning The behavior of this operator is undefined if the Optional does
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
     * cases where the Optional contains a value.
     *
     * \warning The behavior of this operator is undefined if the Optional does
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
     * cases where the Optional contains a value.
     *
     * \warning The behavior of this operator is undefined if the Optional does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Optional<std::string> r{"xyz"s};
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
     * cases where the Optional contains a value.
     *
     * \warning The behavior of this operator is undefined if the Optional does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Optional<std::string> r{"xyz"s};
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
     * \brief Returns `true` if the Optional contains a value.
     *
     * @return `true` if the Optional contains a value, false otherwise
     */
    [[nodiscard]]
    constexpr auto hasValue() const noexcept -> bool {
        return value_.storage_.hasValue_;
    }

    /*!
     * \brief Returns `true` if the Optional contains a value.
     *
     * @return `true` if the Optional contains a value, false otherwise
     */
    [[nodiscard]]
    constexpr explicit operator bool() const noexcept {
        return hasValue();
    }

    /*!
     * \brief Returns an lvalue reference to the contained value.
     *
     * \warning The behavior of this function is undefined if the Optional does
     * not contain a value.
     *
     * Example:
     *
     * ```cpp
     * Optional<std::string, int> r{"xyz"s};
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
     * \warning The behavior of this function is undefined if the Optional does
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
     * \warning The behavior of this function is undefined if the Optional does
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
     * \warning The behavior of this function is undefined if the Optional does
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
     * \brief Returns a copy of the contained value if this Optional holds a value
     * otherwise returns \p defaultValue.
     *
     * Example:
     *
     * ```cpp
     * auto r1 = pimc::Optional<int>{100};
     * assert(r1.valueOr(0) == 100);
     *
     * auto r2 = pimc::Optional<int>{};
     * assert(r2.valueOr(0) == 0);
     * ```
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a copy of the contained value if this Optional holds a value otherwise
     * \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto valueOr(U&& defaultValue) const & -> std::remove_reference_t<T> {
        return hasValue() ? value_.storage_.value_ : std::forward<U>(defaultValue);
    }

    /*!
     * Returns a moved out value of the Optional if it holds a value, otherwise
     * returns \p defaultValue.
     *
     * @tparam U the type of the default value
     * @param defaultValue the default value
     * @return a moved out value of the Optional if it holds a value, otherwise
     * return \p defaultValue
     */
    template <typename U>
    [[nodiscard]]
    constexpr auto valueOr(U&& defaultValue) && -> std::remove_reference_t<T> {
        return hasValue() ?
               std::move(value_.storage_.value_) : std::forward<U>(defaultValue);
    }

    /*!
     * \brief Invokes the function \p f with a const reference to the value
     * contained in this Optional as the argument and returns the return value
     * of the function.
     *
     * \note The function \p f must return a Optional.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained value
     * @return a return value of the function (which is a Optional) if this
     * Optional is not empty
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMap(F&& f) const & -> std::invoke_result_t<F, const T&>
    requires requires(F&& fn, T const& v) { { fn(v) } -> OptionalType; } {
        using ReturnType = std::invoke_result_t<F, T const&>;

        return hasValue()
               ? std::invoke(std::forward<F>(f), value_.storage_.value_)
               : ReturnType{};
    }

    /*!
     * \brief Invokes the function \p f with an rvalue reference to the
     * value contained in this Optional as the argument and returns the return
     * value of the function.
     *
     * \note If this Optional contains a value, it's moved to the argument of
     * the function.
     *
     * \note The function \p f must return an Optional.
     *
     * @tparam F the type of the function \p f
     * @param f the function to apply to the contained value
     * @return a return value of the function (which is a Optional) if this
     * Optional is not empty
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto flatMap(F&& f) && -> std::invoke_result_t<F, T&&>
    requires requires(F&& fn, T&& v) { { fn(v) } -> OptionalType; } {
        using ReturnType = std::invoke_result_t<F, T&&>;

        return hasValue()
               ? std::invoke(std::forward<F>(f), std::move(value_.storage_.value_))
               : ReturnType{};
    }

    /*!
     * \brief Invokes the function \p f with a const reference to the value
     * contained in this Optional and returns a Optional containing the result of
     * the function as the value.
     *
     * @tparam F the type of the function \p f
     * @param f the function to invoke on the value contained in this Optional
     * @return a Optional containing the return value of the function if this Optional
     * is not empty
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto map(F&& f) const & -> Optional<std::invoke_result_t<F, T const&>>
    requires detail::OptionalMapper<F, T const&> {
        using ReturnValueType = std::invoke_result_t<F, T const&>;
        using ReturnType = Optional<ReturnValueType>;

        return hasValue()
               ? ReturnType{
                        InPlaceValue,
                        std::invoke(std::forward<F>(f), value_.storage_.value_)}
               : ReturnType{};
    }

    /*!
     * \brief Invokes the function \p f with an rvalue reference to the value
     * contained in this Optional and returns a Optional containing the result of
     * the function as the value.
     *
     * \note If this Optional contains a value it's moved to the argument of the
     * function \p f, otherwise the error contained in this Optional is moved to
     * the returned Optional.
     *
     * @tparam F the type of the function \p f
     * @param f the function to invoke on the value contained in this Optional
     * @return a Optional containing the return value of the function if this Optional
     * is not empty
     */
    template <typename F>
    [[nodiscard]]
    constexpr auto map(F&& f) && -> Optional<std::invoke_result_t<F, T&&>>
    requires detail::OptionalMapper<F, T&&> {
        using ReturnValueType = std::invoke_result_t<F, T&&>;
        using ReturnType = Optional<ReturnValueType>;

        return hasValue()
               ? ReturnType{
                        InPlaceValue,
                        std::invoke(
                                std::forward<F>(f), std::move(value_.storage_.value_))}
               : ReturnType{};
    }

private:
    detail::OptionalStorage<T> value_;
};

/*!
 * The equality relation between two optionals containing comparable value objects.
 * The following rules apply:
 *   - If one optional contains a value but the other one doesn't, these
 *     optionals are not equal
 *   - If both optionals contain a value, they are equal if and only if the
 *     contained values are equal
 *   - Otherwise, two empty optionals are equal
 *
 * @tparam T1 the type of the value contained in the first optional
 * @tparam T2 the type of the value contained in the second optional
 * @param lhs the left optional
 * @param rhs the right optional
 * @return `true` if the two optionals are equal, `false` otherwise
 */
template <typename T1, typename T2>
inline constexpr auto operator== (Optional<T1> const& lhs, Optional<T2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<T1, T2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() == rhs.value();

        return true;
    }

    return false;
}

/*!
 * The inequality relation between two optionals containing comparable value objects.
 * The following rules apply:
 *   - If one optional contains a value but the other one doesn't, these
 *     optionals are not equal
 *   - If both optionals contain a value, they are equal if and only if the
 *     contained values are equal
 *   - Otherwise (both optionals are empty), the two optionals are equal
 *
 * @tparam T1 the type of the value contained in the first optional
 * @tparam T2 the type of the value contained in the second optional
 * @param lhs the left optional
 * @param rhs the right optional
 * @return `true` if the two optionals are equal, `false` otherwise
 */
template <typename T1, typename T2>
inline constexpr auto operator!= (Optional<T1> const& lhs, Optional<T2> const& rhs)
noexcept -> bool
requires std::equality_comparable_with<T1, T2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() != rhs.value();

        return false;
    }

    return true;
}

/*!
 * The less than relation between two optionals. The following rules apply:
 *   - If the left optional is empty and the right optional has a value
 *     this operator returns `true`
 *   - If the left optional has a value and the right optional is empty
 *     this operator returns `false`
 *   - If both optionals contain a value, the result of this operator is
 *     the result of the less than relation between the contained values
 *   - Otherwise (both optionals are empty) this operator returns `false`
 *
 * @tparam T1 the type of the value contained in the first optional
 * @tparam T2 the type of the value contained in the second optional
 * @param lhs the left optional
 * @param rhs the right optional
 * @return `true` if the left optional is less than the right optional,
 *  `false` otherwise
 */
template <typename T1, typename T2>
inline constexpr auto operator< (Optional<T1> const& lhs, Optional<T2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() < rhs.value();

        return false;
    }

    return static_cast<int>(lhs.hasValue()) < static_cast<int>(rhs.hasValue());
}

/*!
 * The less than or equal relation between two optionals. The following rules
 * apply:
 *   - If the left optional is empty and the right optional has a value
 *     this operator returns `true`
 *   - If the left optional has a value and the right optional is empty
 *     this operator returns `false`
 *   - If both optionals contain a value, the result of this operator is
 *     the result of the less than relation between the contained values
 *   - Otherwise (both optionals are empty) this operator returns `true`
 *
 * @tparam T1 the type of the value contained in the first optional
 * @tparam T2 the type of the value contained in the second optional
 * @param lhs the left optional
 * @param rhs the right optional
 * @return `true` if the left optional is less than the right optional,
 *  `false` otherwise
 */
template <typename T1, typename T2>
inline constexpr auto operator<= (Optional<T1> const& lhs, Optional<T2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() <= rhs.value();

        return true;
    }

    return static_cast<int>(lhs.hasValue()) <= static_cast<int>(rhs.hasValue());
}

/*!
 * The greater than relation between two optionals. The following rules apply:
 *   - If the left optional is empty and the right optional has a value
 *     this operator returns `false`
 *   - If the left optional has a value and the right optional is empty
 *     this operator returns `true`
 *   - If both optionals contain a value, the result of this operator is
 *     the result of the less than relation between the contained values
 *   - Otherwise (both optionals are empty) this operator returns `false`
 *
 * @tparam T1 the type of the value contained in the first optional
 * @tparam T2 the type of the value contained in the second optional
 * @param lhs the left optional
 * @param rhs the right optional
 * @return `true` if the left optional is less than the right optional,
 *  `false` otherwise
 */
template <typename T1, typename T2>
inline constexpr auto operator> (Optional<T1> const& lhs, Optional<T2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() > rhs.value();

        return false;
    }

    return static_cast<int>(lhs.hasValue()) > static_cast<int>(rhs.hasValue());
}

/*!
 * The greater than or equal relation between two optionals. The following
 * rules apply:
 *   - If the left optional is empty and the right optional has a value
 *     this operator returns `false`
 *   - If the left optional has a value and the right optional is empty
 *     this operator returns `true`
 *   - If both optionals contain a value, the result of this operator is
 *     the result of the less than relation between the contained values
 *   - Otherwise (both optionals are empty) this operator returns `true`
 *
 * @tparam T1 the type of the value contained in the first optional
 * @tparam T2 the type of the value contained in the second optional
 * @param lhs the left optional
 * @param rhs the right optional
 * @return `true` if the left optional is less than the right optional,
 *  `false` otherwise
 */
template <typename T1, typename T2>
inline constexpr auto operator>= (Optional<T1> const& lhs, Optional<T2> const& rhs)
noexcept -> bool
requires std::totally_ordered_with<T1, T2> {
    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue())
            return lhs.value() >= rhs.value();

        return true;
    }

    return static_cast<int>(lhs.hasValue()) >= static_cast<int>(rhs.hasValue());
}

//
// Comparison of Optional<T> with a va
//

/*!
 * the equality relation between an optional and a value comparable to the
 * value of the optional. The following rules apply:
 *   - If the result is empty, this operator returns `false`
 *   - If the result is not empty, this operator returns the result of
 *     comparing the value of the optional with \p rhs
 * @tparam T the type of the value of the optional
 * @tparam V the type of the value to compare
 * @param lhs the optional
 * @param rhs the value
 * @return `true` if the optional contains a value and that value is equal to
 * \p rhs, `false` otherwise
 */
template <typename T, typename V>
inline constexpr auto operator== (Optional<T> const& lhs, V const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::equality_comparable_with<T, V>) {
    return lhs.hasValue() and lhs.value() == rhs;
}

/*!
 * the equality relation between an optional and a value comparable to the
 * value of the optional. The following rules apply:
 *   - If the result is empty, this operator returns `false`
 *   - If the result is not empty, this operator returns the result of
 *     comparing the value of the optional with \p lhs
 * @tparam V the type of the value to compare
 * @tparam T the type of the value of the optional
 * @param lhs the value
 * @param rhs the optional
 * @return `true` if the optional contains a value and that value is equal to
 * \p lhs, `false` otherwise
 */
template <typename V, typename T>
inline constexpr auto operator== (V const& lhs, Optional<T> const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::equality_comparable_with<V, T>) {
    return rhs.hasValue() and lhs == rhs.value();
}

/*!
 * the inequality relation between an optional and a value comparable to the
 * value of the optional. The following rules apply:
 *   - If the result is empty, this operator returns `true`
 *   - If the result is not empty, this operator returns the result of
 *     comparing the value of the optional with \p rhs
 * @tparam T the type of the value of the optional
 * @tparam V the type of the value to compare
 * @param lhs the optional
 * @param rhs the value
 * @return `true` if the optional does not contain a value or the contained
 * value is not equal to \p rhs, `false` otherwise
 */
template <typename T, typename V>
inline constexpr auto operator!= (Optional<T> const& lhs, V const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::equality_comparable_with<T, V>) {
    return (not lhs.hasValue()) or lhs.value() != rhs;
}

/*!
 * the inequality relation between an optional and a value comparable to the
 * value of the optional. The following rules apply:
 *   - If the result is empty, this operator returns `true`
 *   - If the result is not empty, this operator returns the result of
 *     comparing the value of the optional with \p lhs
 * @tparam V the type of the value to compare
 * @tparam T the type of the value of the optional
 * @param lhs the value
 * @param rhs the optional
 * @return `true` if the optional does not contain a value or the contained
 * value is not equal to \p lhs, `false` otherwise
 */
template <typename V, typename T>
inline constexpr auto operator!= (V const& lhs, Optional<T> const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::equality_comparable_with<V, T>) {
    return (not rhs.hasValue()) or lhs != rhs.value();
}

/*!
 * The less than relation between an optional and a value comparable to the
 * value of the optional. The following rules apply:
 *   - If the optional is empty this operator returns true
 *   - If the optional has a value, then the result of this operator is
 *     the result of the less than operator applied to the value contained
 *     in the optional and \p rhs
 *
 * @tparam T the type of the value of the optional
 * @tparam V the type of the value to compare
 * @param lhs the optional
 * @param rhs the value
 * @return `true` if the optional is empty or its value is less than \p rhs,
 * `false` otherwise
 */
template <typename T, typename V>
inline constexpr auto operator< (Optional<T> const& lhs, V const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<T, V>) {
    return (not lhs.hasValue()) or lhs.value() < rhs;
}

/*!
 * The less than relation between a value and an optional whose value is comparable
 * to the value. The following rules apply:
 *   - If the optional is empty this operator returns false
 *   - If the optional has a value, then the result of this operator is
 *     the result of the less than operator applied to \p lhs and the value
 *     contained in the optional
 *
 * @tparam V the type of the value to compare
 * @tparam T the type of the value of the optional
 * @param lhs the value
 * @param rhs the optional
 * @return `true` if the optional is not empty and \p rhs is less than the value
 * contained in the optional, `false` otherwise
 */
template <typename V, typename T>
inline constexpr auto operator< (V const& lhs, Optional<T> const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<V, T>) {
    return rhs.hasValue() and lhs < rhs.value();
}

/*!
 * The less than or equal relation between an optional and a value comparable
 * to the value of the optional. The following rules apply:
 *   - If the optional is empty this operator returns true
 *   - If the optional has a value, then the result of this operator is
 *     the result of the less than or equal operator applied to the value
 *     contained in the optional and \p rhs
 *
 * @tparam T the type of the value of the optional
 * @tparam V the type of the value to compare
 * @param lhs the optional
 * @param rhs the value
 * @return `true` if the optional is empty or its value is less than or equal
 * to \p rhs, `false` otherwise
 */
template <typename T, typename V>
inline constexpr auto operator<= (Optional<T> const& lhs, V const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<T, V>) {
    return (not lhs.hasValue()) or lhs.value() <= rhs;
}

/*!
 * The less than or equal relation between a value and an optional whose value
 * is comparable to the value. The following rules apply:
 *   - If the optional is empty this operator returns false
 *   - If the optional has a value, then the result of this operator is
 *     the result of the less than operator applied to \p lhs and the value
 *     contained in the optional
 *
 * @tparam V the type of the value to compare
 * @tparam T the type of the value of the optional
 * @param lhs the value
 * @param rhs the optional
 * @return `true` if the optional is not empty and \p rhs is less than or
 * equal to the value contained in the optional, `false` otherwise
 */
template <typename V, typename T>
inline constexpr auto operator<= (V const& lhs, Optional<T> const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<V, T>) {
    return rhs.hasValue() and lhs <= rhs.value();
}

/*!
 * The greater than relation between an optional and a value comparable to the
 * value of the optional. The following rules apply:
 *   - If the optional is empty this operator returns false
 *   - If the optional has a value, then the result of this operator is
 *     the result of the greater than operator applied to the value contained
 *     in the optional and \p rhs
 *
 * @tparam T the type of the value of the optional
 * @tparam V the type of the value to compare
 * @param lhs the optional
 * @param rhs the value
 * @return `true` if the optional is not empty and its value is greater than
 * \p rhs, `false` otherwise
 */
template <typename T, typename V>
inline constexpr auto operator> (Optional<T> const& lhs, V const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<T, V>) {
    return lhs.hasValue() and lhs.value() > rhs;
}

/*!
 * The greater than relation between a value and an optional whose value is
 * comparable to the value. The following rules apply:
 *   - If the optional is empty this operator returns true
 *   - If the optional has a value, then the result of this operator is
 *     the result of the greater than operator applied to \p lhs and the value
 *     contained in the optional
 *
 * @tparam V the type of the value to compare
 * @tparam T the type of the value of the optional
 * @param lhs the value
 * @param rhs the optional
 * @return `true` if the optional is empty or \p lhs is greater than the
 * value contained in the optional, `false` otherwise
 */
template <typename V, typename T>
inline constexpr auto operator> (V const& lhs, Optional<T> const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<V, T>) {
    return (not rhs.hasValue()) or lhs > rhs.value();
}

/*!
 * The greater than or equal relation between an optional and a value
 * comparable to the value of the optional. The following rules apply:
 *   - If the optional is empty this operator returns false
 *   - If the optional has a value, then the result of this operator is
 *     the result of the greater than or equal operator applied to the value
 *     contained in the optional and \p rhs
 *
 * @tparam T the type of the value of the optional
 * @tparam V the type of the value to compare
 * @param lhs the optional
 * @param rhs the value
 * @return `true` if the optional is not empty and its value is greater than
 * or equal to \p rhs, `false` otherwise
 */
template <typename T, typename V>
inline constexpr auto operator>= (Optional<T> const& lhs, V const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<T, V>) {
    return lhs.hasValue() and lhs.value() >= rhs;
}

/*!
 * The greater than or equal relation between a value and an optional whose
 * value is comparable to the value. The following rules apply:
 *   - If the optional is empty this operator returns true
 *   - If the optional has a value, then the result of this operator is
 *     the result of the greater than or equal operator applied to \p lhs
 *     and the value contained in the optional
 *
 * @tparam V the type of the value to compare
 * @tparam T the type of the value of the optional
 * @param lhs the value
 * @param rhs the optional
 * @return `true` if the optional is empty or \p lhs is greater than or equal
 * to the value contained in the optional, `false` otherwise
 */
template <typename V, typename T>
inline constexpr auto operator>= (V const& lhs, Optional<T> const& rhs)
noexcept -> bool
requires (not OptionalType<V> and std::totally_ordered_with<V, T>) {
    return (not rhs.hasValue()) or lhs >= rhs.value();
}

//
// swap
//

/*!
 * Swaps the two optionals.
 * @tparam T the value type of the optionals
 * @param lhs the left optional
 * @param rhs the right optional
 */
template <typename T>
inline auto swap(Optional<T>& lhs, Optional<T>& rhs)
noexcept(std::is_nothrow_move_constructible_v<Optional<T>> and
         std::is_nothrow_move_assignable_v<Optional<T>> and
         std::is_nothrow_swappable_v<T>) -> void {
    using std::swap;

    if (lhs.hasValue() == rhs.hasValue()) {
        if (lhs.hasValue()) swap(lhs.value(), rhs.value());
    } else {
        auto tmp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(tmp);
    }
}

} // namespace pimc

#pragma GCC diagnostic pop
