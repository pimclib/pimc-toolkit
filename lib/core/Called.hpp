#pragma once

namespace pimc {

/**
 * An object that can be substituted for a callback callable
 * to keep track of calls to the callable.
 */
class Called final {
public:
    /**
     * Constructs an uncalled Called object.
     */
    constexpr Called(): count_{0} {}

    /**
     * Returns true if at least one call has been mad to the callable.
     *
     * @return true if the callable was called, false otherwise
     */
    constexpr explicit operator bool() const { return count_ > 0; }

    /**
     * Returns the number of times the callable has been called.
     *
     * @return the number of times the callable has been called
     */
    [[nodiscard]]
    constexpr unsigned count() const { return count_; }

    /**
     * Resets the instance to the uncalled state.
     */
    constexpr void reset() { count_ = 0; }

    /**
     * Makes this object a callable which can be invoked with any
     * number of any types of arguments. Each time a call is made
     * this operator increments the internal counter.
     *
     * @tparam Ts the types of the arguments with which the callable
     * is invoked at the call site
     */
    template <typename ... Ts>
    constexpr void operator() (Ts&& ...) { ++count_; }
private:
    unsigned count_;
};

} // namespace pimc
