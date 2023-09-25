#pragma once

#include <utility>
#include <concepts>

namespace pimc {

/*!
 * An object which executes a specified function on exit from another
 * function.
 *
 * @tparam D the type of a function which must be invocable without arguments
 */
template <typename D>
requires std::invocable<D>
class Deferred final {
    template <typename F>
    friend Deferred<F> defer(F&& f) requires std::invocable<F>;
public:
    ~Deferred() {
        if (not cancelled_) d_();
    }

    Deferred(Deferred const&) = delete;
    Deferred(Deferred&&) = delete;
    auto operator= (Deferred const&) -> Deferred& = delete;
    auto operator= (Deferred&&) -> Deferred& = delete;

    /*!
     * Cancels the deferred function execution.
     */
    void cancel() { cancelled_ = true; }

private:
    template <typename F>
    constexpr explicit Deferred(F&& f)
            : d_{std::forward<F>(f)}, cancelled_{false} {}

private:
    D d_;
    bool cancelled_;
};

/*!
 * Create a deferred computation.
 *
 * @tparam F the type of a function which must be invocable without arguments
 * @param f the function to invoke on exit
 * @return a Deferred object encapsulating the specified deferred computation
 */
template <typename F>
Deferred<F> defer(F&& f) requires std::invocable<F> {
    return Deferred<F>(std::forward<F>(f));
}

} // namespace pimc