#pragma once

#include "Structured.hpp"

namespace pimc::yaml {

/*!
 * \brief A structured YAML error handler base class.
 *
 * This class provides utility functions to process errors and in
 * addition it also keeps track if any errors occurred during
 * structured YAML processing.
 *
 * It's an abstract class which requires the concrete class to provide
 * the function showError() which is responsible for presenting the
 * error to the user.
 */
class ErrorHandler {
public:
    constexpr explicit ErrorHandler(const char* yamlfn)
    : yamlfn_{yamlfn}, cnt_{0} {}

    virtual ~ErrorHandler() = default;

    /*!
     * \brief Reports an error if \p r contains error, otherwise does
     * nothing, and always returns \p unchanged.
     *
     * @tparam T  the value type
     * @param r the result of structured YAML operation
     * @return the same result \p r
     */
    template <typename T>
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
        showError(ectx);
    }

    /*!
     * \brief Returns the number of encountered errors.
     *
     * @return the number of encountered errors.
     */
    [[nodiscard]]
    int errors() const { return cnt_; }

    /*!
     * \brief Returns the name of the YAML source file.
     *
     * @return the name of the YAML source file
     */
    [[nodiscard]]
    char const* filename() const { return yamlfn_; }

protected:
    virtual void showError(ErrorContext const&) = 0;

private:
    char const* yamlfn_;
    int cnt_;
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
 * A concrete class which prints the error messages to `stderr`.
 *
 * @tparam ECS the parameter which controls whether the context of
 * the error messages should be always printed, or only if the line
 * number in the source YAML file is not available.
 */
template <ErrorContextShow ECS = ErrorContextShow::Optionally>
struct StderrErrorHandler: public ErrorHandler {
    using ErrorHandler::ErrorHandler;

    void showError(ErrorContext const& ectx) final {
        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(bi, "error: ");
        if (ectx.line() != -1)
            fmt::format_to(bi, "{}, {}: ", filename(), ectx.line());
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