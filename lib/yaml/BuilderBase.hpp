#pragma once

#include "Structured.hpp"

namespace pimc::yaml {

/*!
 * A concept which stipulates that a class which extends BuilderBase
 * must have a function ``consume()``.
 *
 * @tparam EC the class which extends BuilderBase
 */
template<typename EC>
concept ErrorConsumer = requires(EC ec, ErrorContext ectx) {
    { ec.consume(ectx) };
};

/*!
 * A base class which provides error handling utility for a builder
 * style YAML processor.
 *
 * @tparam EC
 */
template<typename EC>
class BuilderBase {
protected:
    /*!
     * \brief If \p r contains an error, this function moves the
     * error context to the consume() function of the class extending
     * BuilderBase.
     *
     * If the result contains a value, this function leaves the result
     * intact and returns it.
     *
     * If the result contains an error, this function moves the result
     * into the consume() function and returns the result with an
     * empty error context but still in the error state.
     *
     * @tparam T the type of the result value
     * @param r the result to check
     * @return a result in the error state but with an empty error
     * context if \p r is in the error state, or the unmodified \p r
     */
    template<typename T>
    auto chk(Result<T, yaml::ErrorContext> r) -> Result<T, yaml::ErrorContext> {
        if (r.hasError())
            impl().consume(std::move(r).error());
        return r;
    }

private:
    template<typename Self = EC>
    Self &impl() noexcept requires ErrorConsumer<Self> {
        return static_cast<Self &>(*this);
    }
};

} // namespace pimc::yaml