#pragma once

#include "StructuredYaml.hpp"

namespace pimc::yaml {

/*!
 * A concept which requires that a class which extends ErrorHandler
 * must have a function showError(ErrorContext const& ectx)
 *
 * @tparam T the class extending ErrorHandler
 */
template <typename T>
concept ErrorReporter = requires(T t, ErrorContext const& ectx) {
    t.showError(ectx);
};

/*!
 * An enumeration which is used to convey to the ErrorReporter if
 * the context of the error should be always shown or only if the
 * line numbers in the source YAML file are not available.
 */
enum class ErrorContextShow: int {
    /*!
     * Indicates that the context of the error messages should only
     * be shown if the line numbers in the source YAML file are not
     * available.
     */
    Optionally = 0,
    /*!
     * Indicates that the context of the error messages should always
     * be shown.
     */
    Always = 1
};

/*!
 * \brief A structured YAML error handler base class.
 *
 * This class provides utility functions to process errors and in
 * addition it also keeps track if any errors occurred during
 * structured YAML processing.
 *
 * It relies on the class that extends it to perform the output of
 * the error messages.
 *
 * @tparam ER a class that satisfies the ErrorReporter concept
 */
template <typename ER>
class ErrorHandler {
public:
    constexpr explicit ErrorHandler(const char* yamlfn)
    : yamlfn_{yamlfn}, cnt_{0} {}

    /*!
     * \brief Reports an error if \p r contains error, otherwise does
     * nothing, and always returns \p unchanged.
     *
     * @tparam T  a type which must be Scalar, ValueContext, MappingContext
     * or SequenceContext
     * @param r the result of structured YAML operation
     * @return the same result \p r
     */
    template <typename T>
    requires OneOf<T, Scalar, ValueContext, MappingContext, SequenceContext>
    auto chk(Result<T, ErrorContext> r) -> Result<T, ErrorContext> {
        if (r.hasError()) error(r.error());

        return r;
    }

    /*!
     * \brief Reports the error messages listed in \p errs.
     *
     * If \p errs is empty, this function has no effect.
     *
     * @param errs a vector of ErrorContext objects describing the
     * encountered errors.
     */
    void chk(std::vector<ErrorContext> const& errs) {
        for (auto const& err: errs) error(err);
    }

    /*!
     * \brief Reports the error described by \p ectx.
     *
     * @param ectx the ErrorContext to report
     */
    void error(ErrorContext const& ectx) {
        ++cnt_;
        impl().showError(ectx);
    }

    /*!
     * \brief Returns the number of encountered errors.
     *
     * @return the number of encountered errors.
     */
    [[nodiscard]]
    int errors() const { return cnt_; }

private:
    template <typename Self = ER>
    Self& impl() noexcept requires ErrorReporter<Self> {
        return static_cast<Self&>(*this);
    }
protected:
    char const* yamlfn_;
    int cnt_;
};

/*!
 * A concrete class which prints the error messages to `stderr`.
 *
 * @tparam ECS the parameter which controls whether the context of
 * the error messages should be always printed, or only if the line
 * number in the source YAML file is not available.
 */
template <ErrorContextShow ECS = ErrorContextShow::Optionally>
struct StderrErrorHandler: public ErrorHandler<StderrErrorHandler<ECS>> {
    using ErrorHandler<StderrErrorHandler<ECS>>::ErrorHandler;

    void showError(ErrorContext const& ectx) {
        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(bi, "error: ");
        if (ectx.line() != -1)
            fmt::format_to(bi, "{}, {}: ", this->yamlfn_, ectx.line());
        if constexpr (ECS == ErrorContextShow::Always) {
            fmt::format_to(bi, "{}", ectx.context());
        } else {
            if (ectx.line() == -1)
                fmt::format_to(bi, "{}", ectx.context());
        }
        fmt::format_to(bi, "{}", ectx.message());
        mb.push_back('\n');
        mb.push_back(static_cast<char>(0));
        fputs(mb.data(), stderr);
    }
};

StderrErrorHandler(char const*) -> StderrErrorHandler<ErrorContextShow::Optionally>;

} // namespace pimc::yaml