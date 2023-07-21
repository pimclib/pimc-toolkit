#include <utility>
#include <type_traits>

#define IL_ED25519 {                                                     \
    'e', 'd', '2', '5', '5', '1', '9', ' ', 'i', 's', ' ',               \
    'a', ' ', 'v', 'e', 'r', 'y', ' ', 'n', 'i', 'c', 'e', ' ',          \
    'e', 'l', 'l', 'i', 'p', 't', 'i', 'c', ' ', 'c', 'u', 'r', 'v', 'e' }

namespace pimc::testing {

struct NotCopyOrMoveable {
    NotCopyOrMoveable() = default;

    NotCopyOrMoveable(NotCopyOrMoveable const &) = delete;

    NotCopyOrMoveable(NotCopyOrMoveable &&) = delete;

    auto operator=(NotCopyOrMoveable const &) -> NotCopyOrMoveable & = delete;

    auto operator=(NotCopyOrMoveable &&) -> NotCopyOrMoveable & = delete;
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;
};

template<typename T>
struct CopyOnly : public T {
    using T::T;

    CopyOnly() = default;

    CopyOnly(T const &rhs)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
            : T{rhs} {}

    CopyOnly(CopyOnly const &) = default;

    CopyOnly(CopyOnly &&) = delete;

    auto operator=(T const &rhs) -> CopyOnly & {
        T::operator=(rhs);
        return *this;
    }

    auto operator=(CopyOnly const &) -> CopyOnly & = default;

    auto operator=(CopyOnly &&) -> CopyOnly & = delete;
};

template<typename T>
struct MoveOnly : public T {
    using T::T;

    MoveOnly() = default;

    MoveOnly(T &&rhs)
    noexcept(std::is_nothrow_move_constructible_v<T>)
            : T{std::move(rhs)} {}

    MoveOnly(MoveOnly const &) = delete;

    MoveOnly(MoveOnly &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    auto operator=(T &&rhs) noexcept(std::is_nothrow_move_assignable_v<T>) -> MoveOnly & {
        T::operator=(std::move(rhs));
        return *this;
    }

    auto operator=(MoveOnly const &) = delete;

    auto operator=(MoveOnly &&rhs)
    noexcept(std::is_nothrow_move_constructible_v<T>) -> MoveOnly & = default;
};

template<typename T>
struct Throwing : public T {
    Throwing() noexcept(false): T{} {}

    Throwing(T const &rhs) noexcept(false): T{rhs} {}

    Throwing(T &&rhs) noexcept(false): T{std::move(rhs)} {}

    Throwing(Throwing const &rhs) noexcept(false): T{rhs} {}

    Throwing(Throwing &&rhs) noexcept(false): T{std::move(rhs)} {}

    template<typename ... Ts>
    requires std::constructible_from<T, Ts...>
    Throwing(Ts &&... args): T{std::forward<Ts>(args)...} {}

    auto operator=(Throwing const &rhs) noexcept(false) -> Throwing & {
        T::operator=(rhs);
        return *this;
    }

    auto operator=(Throwing &&rhs) noexcept(false) -> Throwing & {
        T::operator=(std::move(rhs));
        return *this;
    }

    template<typename U>
    auto operator=(U &&rhs) noexcept(false) -> Throwing &requires std::is_nothrow_assignable_v<T, U> {
        T::operator=(std::forward<U>(rhs));
        return *this;
    }
};

template<typename T>
struct NotThrowing : public T {
    NotThrowing() noexcept: T{} {}

    NotThrowing(T const &rhs) noexcept: T{rhs} {}

    NotThrowing(T &&rhs) noexcept: T{std::move(rhs)} {}

    NotThrowing(NotThrowing const &rhs) noexcept: T{rhs} {}

    NotThrowing(NotThrowing &&rhs) noexcept: T{std::move(rhs)} {}

    template<typename ... Ts>
    requires std::constructible_from<T, Ts...>
    NotThrowing(Ts &&... args): T{std::forward<Ts>(args)...} {}

    auto operator=(NotThrowing const &rhs) noexcept -> NotThrowing & {
        T::operator=(rhs);
        return *this;
    }

    auto operator=(NotThrowing &&rhs) noexcept -> NotThrowing & {
        T::operator=(std::move(rhs));
        return *this;
    }

    template<typename U>
    auto operator=(U &&rhs) noexcept -> NotThrowing &requires std::is_nothrow_assignable_v<T, U> {
        T::operator=(std::forward<U>(rhs));
        return *this;
    }
};

template<typename T>
struct Explicit : public T {
    template<typename ... Ts>
    requires std::constructible_from<T, Ts...>
    explicit Explicit(Ts &&... args): T{std::forward<Ts>(args)...} {}

    explicit Explicit() = default;

    explicit Explicit(Explicit const &) = default;

    explicit Explicit(Explicit &&)
    noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    auto operator=(Explicit const &) -> Explicit & = default;

    auto operator=(Explicit &&)
    noexcept(std::is_nothrow_move_assignable_v<T>) -> Explicit & = default;
};

class BoolSetterOnDestr {
public:
    BoolSetterOnDestr() : v_{nullptr} {}

    BoolSetterOnDestr(bool *v) : v_{v} {}

    BoolSetterOnDestr(BoolSetterOnDestr const &) = default;

    BoolSetterOnDestr(BoolSetterOnDestr &&) noexcept = default;

    auto operator=(BoolSetterOnDestr const &) -> BoolSetterOnDestr & = default;

    auto operator=(BoolSetterOnDestr &&) noexcept -> BoolSetterOnDestr & = default;

    ~BoolSetterOnDestr() { if (v_ != nullptr) *v_ = true; }

private:
    bool *v_;
};

struct VValue {
    virtual ~VValue() = default;

    [[nodiscard]]
    virtual auto getValue() const noexcept -> int { return 25519; }
};

class CValue : public VValue {
public:
    CValue(int value) : value_{value} {}

    auto operator= (int v) noexcept -> CValue& {
        value_ = v;
        return *this;
    }

    [[nodiscard]]
    auto getValue() const noexcept -> int override { return value_; }

private:
    int value_;
};

class XY final {
public:
    constexpr XY(int x, int y) : x_{x}, y_{y} {}

    [[nodiscard]]
    int x() const { return x_; }

    [[nodiscard]]
    int y() const { return y_; }

private:
    int x_;
    int y_;
};

} // namespace pimc::testing

