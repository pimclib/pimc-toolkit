#include <cstdint>
#include <concepts>
#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

#include <fmt/format.h>

namespace pimc {

/**
 * This exception is thrown by the functions of GetOptLong and GetOptLongResult
 * when they encounter errors. Such errors are indicative of the semantic issues
 * therefore they should not be caught, but instead the errors should be corrected
 * in the program.
 */
struct GetOptLongError: public std::logic_error {
    explicit GetOptLongError(std::string const& msg)
    : std::logic_error{msg} {}
};

/**
 * This exception is thrown while processing the actual command line and it
 * should be caught to report the error.
 */
struct CommandLineError: public std::runtime_error {
    explicit CommandLineError(std::string const& msg)
    : std::runtime_error{msg} {}
};

class GetOptLongResult;

/**
 * The class which builds the internal structures required for the
 * command line processing as well as the arguments to the
 * ``getopt_long`` function and which invokes the ``getopt_long``
 * function to process the command line and return an object containing
 * the processed values of the options.
 */
class GetOptLong final {
public:
    /**
     * The constant which indicates that the short option is not
     * present.
     */
    static constexpr inline char LongOnly{0};
    /**
     * The constant which indicates that the corresponding value
     * option may be specified multiple times on the command line.
     */
    static constexpr inline bool Multiple{true};

    GetOptLong(GetOptLong const&) = delete;
    GetOptLong(GetOptLong&&) noexcept = default;
    GetOptLong& operator= (GetOptLong const&) = delete;
    GetOptLong& operator= (GetOptLong&&) noexcept = default;
    ~GetOptLong();

    /**
     * Adds a command line flag with the specified numeric ID,
     * the specified single character short option, the specified
     * multi-character long option and the specified help description.
     *
     * @param id a unique flag ID
     * @param shortOpt the short option
     * @param longOpt the long option
     * @param help the string describing the option
     * @return a reference to this GetOptLong object
     */
    GetOptLong& flag(
            uint32_t id, char shortOpt, std::string longOpt,
            std::string help);

    /**
     * Adds a required value option with the specified numeric ID,
     * the specified single character short option, the specified
     * multi-character long option, the specified metavariable and the
     * specified help description.
     *
     * @param id a unique value option ID
     * @param shortOpt the short option
     * @param longOpt the long option
     * @param metavar the metavariable
     * @param help the string describing the option
     * @param multiple if false (default) indicates that the option may
     * appear only once on the command line, if true indicates that the
     * option may appear multiple times
     * @return a reference to this GetOptLong object
     */
    GetOptLong& required(
            uint32_t id, char shortOpt, std::string longOpt,
            std::string metavar, std::string help, bool multiple = false);
    /**
     * Adds an optional value option with the specified numeric ID,
     * the specified single character short option, the specified
     * multi-character long option, the specified metavariable and the
     * specified help description.
     *
     * @param id a unique value option ID
     * @param shortOpt the short option
     * @param longOpt the long option
     * @param metavar the metavariable
     * @param help the string describing the option
     * @param multiple if false (default) indicates that the option may
     * appear only once on the command line, if true indicates that the
     * option may appear multiple times
     * @return a reference to this GetOptLong object
     */
    GetOptLong& optional(
            uint32_t id, char shortOpt, std::string longOpt,
            std::string metavar, std::string help, bool multiple = false);

    /**
     * Processes the command line using the added flag and value options
     * and returns the result object containing the values of the options.
     *
     * @param argc the number of the command line options as passed to the
     * ``main()`` function
     * @param argv the command line arguments as passed to the ``main()``
     * function
     * @return the result object GetOptLongResult
     * @throws CommandLineError if the command line does not match the
     * added options
     */
    GetOptLongResult args(int argc, char * const* argv);

    /**
     * The factory function which creates an instance of this object.
     * The argument to this function is the *help legend*, which in
     * this context is the text which follows the text ``USAGE: <progname>``,
     * where``<progname>`` is equal to the first element of the array of
     * the zero terminated strings as passed to the ``main()`` function.
     *
     * @param helpLegend the help legend
     * @return an instance of GetOptLong
     */
    static inline GetOptLong with(std::string helpLegend) {
        return GetOptLong(std::string{}, std::move(helpLegend));
    }

    /**
     * The factory function which creates an instance of this object.
     * The first argument to this function is the program name, and the
     * second argument *help legend*.
     *
     * @param progname the program name. This value overrides the program
     * name passed to the ``main()`` function.
     * @param helpLegend the help legend
     * @return an instance of GetOptLong
     */
    static inline GetOptLong prog(
            std::string progname, std::string helpLegend) {
        return GetOptLong(std::move(progname), std::move(helpLegend));
    }
private:
    explicit GetOptLong(std::string prog, std::string helpLegend);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * An object containing the values of the command line options.
 */
class GetOptLongResult final {
    friend class GetOptLong;
public:
    ~GetOptLongResult();

    /**
     * Returns the value of the flag with the specified ID. If the flag was
     * present on the command line this function returns true, if not, it
     * returns false.
     *
     * @return true if the flag was present on the command line, false otherwise.
     */
    [[nodiscard]]
    bool flag(unsigned) const;

    /**
     * Returns the values of the value option with the specified ID. The following
     * rules apply:
     *
     * - If the value option was defined using GetOptLong::required:
     *     - If the value option was specified as singular the returned vector
     *       will contain exactly one value.
     *     - If the value option was specified as multiple the returned vector's
     *       size will be equal to how many times the value option appeared on the
     *       command line, and its elements will be the option values in the order
     *       in which the appeared on the command line. The size of the returned
     *       vector will be at least 1.
     *
     * - If the value option was defined using GetOptLong::optional:
     *     - If the value option was specified as singular the returned vector
     *       will have at most one element. It will be empty if the option did
     *       not appear on the command line.
     *     - If the value option was specified as multiple the returned vector's
     *       size will be equal to how many times the value option appeared on the
     *       command line, and its elements will be the option values in the order
     *       in which the appeared on the command line. The returned vector will
     *       be empty if the option did not appear on the command line.
     *
     * @return the values of the value option
     */
    [[nodiscard]]
    std::vector<std::string> const& values(unsigned) const;

    /**
     * This function returns the positional command line arguments.
     *
     * @return the positional command line arguments.
     */
    [[nodiscard]]
    std::vector<std::string> const& positional() const;

private:
    struct Impl;

    explicit GetOptLongResult(std::unique_ptr<Impl>);
private:
    std::unique_ptr<Impl> impl_;
};

} // namespace pimc
