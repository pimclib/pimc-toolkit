#pragma once

#include <utility>
#include <vector>
#include <unordered_set>
#include <string>

#include "pimc/formatters/Fmt.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#if __GNUC__ >= 13
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif
#include <yaml-cpp/yaml.h>
#pragma GCC diagnostic pop

#include "pimc/core/CompilerUtils.hpp"
#include "pimc/core/Optional.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/formatters/MemoryBuffer.hpp"

using namespace std::string_literals;

namespace pimc::yaml {

/*!
 * \brief An object that describes an error which occurs when the
 * structure of the loaded YAML file doesn't match the expectation
 * of the structured YAML processing.
 */
class ErrorContext {
public:

    ErrorContext(int line, std::string context, std::string message)
            : line_{line}
            , context_{std::move(context)}
            , message_{std::move(message)} {}

    /*!
     * \brief Returns the line number in the YAML source file where
     * the error was encountered.
     *
     * If the YAML structure was modified after it was loaded but
     * before the structured processing takes place, the returned
     * value may be -1 if the line number was no longer available.
     *
     * @return the line number
     */
    [[nodiscard]]
    int line() const { return line_; }

    /*!
     * \brief The textual description of the context of the structure
     * where the error was encountered.
     *
     * This is the chain of the YAML elements starting from the root
     * node and down to the last successfully processed YAML node.
     *
     * @return the textual description of the location where the error
     * was encountered
     */
    [[nodiscard]]
    std::string const& context() const { return context_; }

    /*!
     * \brief The detail error message.
     *
     * @return the error message
     */
    [[nodiscard]]
    std::string const& message() const { return message_; }

private:
    int line_;
    std::string context_;
    std::string message_;
};

/*!
 * The base class for ValueContext, MappingContext and SequenceContext.
 */
class NodeContext {
public:
    /*!
     * \brief Returns the line in the YAML source file for node contained
     * represented by this context.
     *
     * If the node was added programmatically, the returned value is -1.
     *
     * @return the line in the YAML source file
     */
    inline int line() const {
        return nodeLine(node_);
    }

    /*!
     * \brief Creates an error context context for the YAML node represented
     * by this context with the specified error message.
     *
     * @tparam Ts the types of the argument to the format string
     * @param fs the format string
     * @param args the arguments to the format string
     * @return an error context
     */
    template <typename ... Ts>
    ErrorContext error(
            fmt::format_string<Ts...> const& fs,
            Ts&& ... args) const {
        return errorAt(line(), fs, std::forward<Ts>(args)...);
    }

    /*!
     * \brief Creates an error context for the YAML node represented by
     * this context with the specified error message.
     *
     * This function is useful to convert textual error messages
     * returned by general purpose functions in the form of
     * Result<T, std::string> (e.g. an IP address parser, etc.) for
     * the node represented by this context.
     *
     * @tparam T a type from which std::string can be constructed
     * @param msg the message
     * @return an error context
     */
    template <typename T>
    requires requires(T&& v) { std::string(std::forward<T>(v)); }
    ErrorContext error(T&& msg) const {
        auto &mbctx = getMemoryBuffer();
        auto bictx = std::back_inserter(mbctx);
        for (auto const& name: ctx_)
            fmt::format_to(bictx, "{}: ", name);
        std::string context = fmt::to_string(mbctx);
        return {line(), std::move(context), std::forward<T>(msg)};
    }

protected:
    NodeContext(
            YAML::Node const& node,
            std::vector<std::string> parentCtx_,
            std::string name)
            : node_{node}
            , ctx_{std::move(parentCtx_)} {
        ctx_.emplace_back(std::move(name));
    }

    NodeContext(
            YAML::Node const& node,
            std::vector<std::string> parentCtx_)
            : node_{node}
            , ctx_{std::move(parentCtx_)} {}

    PIMC_ALWAYS_INLINE
    static int nodeLine(YAML::Node const& n) {
        return PIMC_LIKELY(n.Mark().line != -1) ? n.Mark().line + 1 : -1;
    }

    template <typename ... Ts>
    ErrorContext errorAt(
            int lineNo,
            fmt::format_string<Ts...> const& fs,
            Ts&& ... args) const {
        auto &mbctx = getMemoryBuffer();
        auto bictx = std::back_inserter(mbctx);
        for (auto const& name: ctx_)
            fmt::format_to(bictx, "{}: ", name);
        std::string context = fmt::to_string(mbctx);

        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        return {lineNo, std::move(context), fmt::to_string(mb)};
    }

protected:
    YAML::Node node_;
    std::vector<std::string> ctx_;
};



class ScalarContext;
class MappingContext;
class SequenceContext;

/*!
 * A generic context over a YAML Node.
 *
 * An object of this type allows querying the properties of the YAML
 * node, such as whether it's a mapping, a sequence or a scalar, etc.
 *
 */
class ValueContext final: public NodeContext {
    friend class MappingContext;
    friend class SequenceContext;
public:

    /*!
     * \brief A factory function which creates the root ValueContext
     * from the specified YAML Node.
     *
     * @param rootNode the root YAML element
     * @return a ValueContext representing the root element
     */
    static ValueContext root(YAML::Node const& rootNode) {
        return {rootNode, {}};
    }

    /*!
     * \brief Returns `true` if the YAML node is undefined.
     *
     * This function allows finding out if the node exists or not.
     *
     * @return `true` if the YAML node is undefined, `false` otherwise
     */
    [[nodiscard]]
    bool isDefined() const { return node_.IsDefined(); }

    /*!
     * \brief Returns `true` if the YAML node is `null`.
     *
     * @return `true` if the YAML node is `null`, `false` otherwise
     */
    [[nodiscard]]
    bool isNull() const { return node_.IsNull(); }

    /*!
     * \brief Returns `true` if the YAML node is scalar.
     *
     * @return `true` if the YAML node is scalar, `false` otherwise
     */
    [[nodiscard]]
    bool isScalar() const { return node_.IsScalar(); }

    /*!
     * \brief Returns `true` if the YAML node is a sequence.
     *
     * @return `true` if the YAML node is sequence, `false` otherwise
     */
    [[nodiscard]]
    bool isSequence() const { return node_.IsSequence(); }

    /*!
     * \brief Returns `true` if the YAML node is a mapping.
     *
     * @return `true` if the YAML node is a mapping, `false` otherwise
     */
    [[nodiscard]]
    bool isMapping() const { return node_.IsMap(); }

    /*!
     * \brief If the YAML node represented by this context is a scalar,
     * returns a result containing the scalar context.
     *
     * If the YAML node is not a scalar, this function returns a result
     * containing the description of the error.
     *
     * The \p name parameter is the human readable description of the
     * scalar in question, e.g. `IP v4 address`.
     *
     * @param name the human readable description of the scalar
     * @return a result containing the scalar, or an error if this node
     * is not a scalar
     */
    auto getScalar(std::string const& name) const -> Result<ScalarContext, ErrorContext>;

    /*!
     * \brief If the YAML node represented by this context is scalar,
     * returns a result containing the scalar context.
     *
     * If the YAML node is not a scalar, this function returns a result
     * containing the description of the error.
     *
     * @return a result containing the scalar, or an error if this node
     * is not a scalar
     */
    auto getScalar() const -> Result<ScalarContext, ErrorContext>;

    /*!
     * \brief If the YAML node represented by this context is a mapping,
     * returns a result containing a mapping context.
     *
     * If the YAML node is not a mapping, this function returns a result
     * containing the description of the error.
     *
     * The \p name parameter is the human readable description of the
     * mapping in question, e.g. `host configuration`.
     *
     * @param name the human readable description of the mapping
     * @return a result containing a mapping context, or an error if this
     * node is not a mapping
     */
    auto getMapping(std::string name) const -> Result<MappingContext, ErrorContext>;

    /*!
     * \brief If the YAML node represented by this context is a mapping,
     * returns a result containing a mapping context.
     *
     * If the YAML node is not a mapping, this function returns a result
     * containing the description of the error.
     *
     * @return a result containing a mapping context, or an error if this
     * node is not a mapping
     */
    auto getMapping() const -> Result<MappingContext, ErrorContext>;

    /*!
     * \brief If the YAML node represented by this context is a sequence,
     * returns a result containing a sequence context.
     *
     * If the YAML node is not a sequence, this function returns a result
     * containing the description of the error.
     *
     * The \p name parameter is the human readable description of the
     * sequence in question, e.g. `host addresses`.
     *
     * @param name the human readable description of the sequence
     * @return a result containing a sequence context, or an error if this
     * node is not a sequence
     */
    auto getSequence(std::string name) const -> Result<SequenceContext, ErrorContext>;

    /*!
     * \brief If the YAML node represented by this context is a sequence,
     * returns a result containing a sequence context.
     *
     * If the YAML node is not a sequence, this function returns a result
     * containing the description of the error.
     *
     * @param name the human readable description of the sequence
     * @return a result containing a sequence context, or an error if this
     * node is not a sequence
     */
    auto getSequence() const -> Result<SequenceContext, ErrorContext>;

private:
    using NodeContext::NodeContext;
};

/*!
 * A context representing a YAML scalar.
 */
class ScalarContext final: public NodeContext {
    friend class ValueContext;
public:

    /*!
     * \brief Returns the const reference to the value of the scalar.
     *
     * @return the const reference to the value of the scalar
     */
    [[nodiscard]]
    std::string const& value() const& { return node_.Scalar(); }

private:
    ScalarContext(
            YAML::Node const& node,
            std::vector<std::string> parentCtx)
            : NodeContext{node, std::move(parentCtx)} {}

};

/*!
 * A context representing a YAML mapping.
 */
class MappingContext final: public NodeContext {
    friend class ValueContext;
public:
    /*!
     * \brief Returns a result containing a ValueContext for the specified
     * field in the mapping.
     *
     * If the YAML mapping does not have a field with the specified name,
     * this function returns a result containing the error.
     *
     * @param field The field as it appears in the YAML source file
     * @return a result containing a ValueContext representing the field,
     * or an error if there is no such field
     */
    [[nodiscard]]
    auto required(std::string const& field) const -> Result<ValueContext, ErrorContext>;

    /*!
     * \brief Returns an optional containing a ValueContext for the specified
     * field.
     *
     * If the YAML mapping does not have a field with the specified name,
     * the returned optional is empty.
     *
     * @param field the field as it appears in the YAML source file
     * @return an optional containing a ValueContext representing the field,
     * or an empty optional if there is no such field
     */
    [[nodiscard]]
    auto optional(std::string const& field) const -> Optional<ValueContext>;

    /*!
     * \brief Returns the size of the mapping.
     *
     * @return the size of the mapping
     */
    size_t size() const { return node_.size(); }

    /*!
     * \brief Returns a vector consisting of key/value pairs of the mapping.
     *
     * @return a vector consisting of key/value pairs of the mapping
     */
    [[nodiscard]]
    auto items() const -> std::vector<std::pair<ValueContext, ValueContext>>;

    /*!
     * \brief If the YAML mapping contains unrecognized or duplicate fields,
     * returns a vector containing an ErrorContext for each of those fields.
     *
     * If the mapping has no extraneous fields, the returned vector is
     * empty.
     *
     * This function **must** be called after all of the calls to required()
     * and optional() have been made. This function considers any fields that
     * did not appear in the calls to required() and optional() as unrecognized.
     *
     * @return a vector of ErrorContext objects describing the unrecognized
     * fields
     */
    auto extraneous() const -> std::vector<ErrorContext>;
private:
    MappingContext(
            std::string name,
            YAML::Node const& node,
            std::vector<std::string> parentCtx_)
            : NodeContext{node, std::move(parentCtx_)}
            , name_{std::move(name)} {}

    std::string describe(std::string const& field) const {
        if (name_.empty())
            return fmt::format("field '{}'", field);

        return fmt::format("field '{}' of {}", field, name_);
    }

private:
    std::string name_;
    mutable std::unordered_set<std::string> knownFields_;
};

/*!
 * A context representing a YAML sequence.
 */
class SequenceContext final: public NodeContext {
    friend class ValueContext;
public:
    /*!
     * \brief Returns the size of the sequence.
     *
     * @return the size of the sequence
     */
    size_t size() const { return node_.size(); }

    /*!
     * \brief Returns a result containing a ValueContext of the sequence
     * element with the specified index.
     *
     * If the index is out of bounds, this function returns a result
     * containing an error.
     *
     * @param i the index of the element
     * @return a result containing a ValueContext of the sequence element
     * with the specified index, or an error if the index is out of bounds
     */
    auto operator[] (size_t i) const -> Result<ValueContext, ErrorContext>;

    /*!
     * \brief Returns a vector of ValueContext objects for each of the
     * element of the sequence in the same order.
     *
     * @return a vector of ValueContext object for the elements of the sequence
     */
    [[nodiscard]]
    std::vector<ValueContext> list() const;

private:
    SequenceContext(
            std::string name,
            YAML::Node const& node,
            std::vector<std::string> parentCtx_)
            : NodeContext{node, std::move(parentCtx_)}
            , name_{std::move(name)} {}

    std::string describe(size_t i) const {
        if (name_.empty())
            return fmt::format("element #{}", i);

        return fmt::format("element #{} of {}", i, name_);
    }
private:
    std::string name_;
    using NodeContext::NodeContext;
};

inline auto ValueContext::getScalar()
const -> Result<ScalarContext, ErrorContext> {
    return getScalar(""s);
}

inline auto ValueContext::getMapping()
const -> Result<MappingContext, ErrorContext> {
    return getMapping(""s);
}

inline auto ValueContext::getSequence() const
-> Result<SequenceContext, ErrorContext> {
    return getSequence(""s);
}

/*!
 * \brief Returns a mapping function which can be used to convert the
 * returned ValueContext into a ScalarContext.
 *
 * If the result object does not contain a ValueContext, the call to
 * Result::flatMap() will propagate the original error.
 *
 * If the ValueContext is not a scalar, the result of the call to
 * Result::flatMap() will contain the error describing the issue.
 *
 * Example:
 *
 * ```cpp
 * pimc::Result<ScalarContext, ErrorContext> scalarField(
 *         MappingContext const& mapCtx, std::string const& field) {
 *     return mapCtx.required(field).flatMap(scalar());
 * }
 * ```
 *
 * @return a function which applies ValueContext::getScalar() to
 * the ValueContext passed to it as a result of a call to
 * Result::flatMap()
 */
inline auto scalar() {
    return [] (ValueContext const& vctx) { return vctx.getScalar(); };
}

/*!
 * \brief Returns a mapping function which can be used to convert the
 * returned ValueContext into a ScalarContext using Result::flatMap().
 *
 * The \p name parameter is the human readable description of the
 * scalar.
 *
 * If the result object does not contain a ValueContext, the call to
 * Result::flatMap() will propagate the original error.
 *
 * If the ValueContext is not a scalar, the result of the call to
 * Result::flatMap() will contain the error describing the issue.
 *
 * Example:
 *
 * ```cpp
 * pimc::Result<ScalarContext, ErrorContext> scalarField(
 *         MappingContext const& mapCtx, std::string const& field) {
 *     return mapCtx.required(field).flatMap(scalar("IP v4 address"));
 * }
 * ```
 *
 * @return a function which applies ValueContext::getScalar() to
 * the ValueContext passed to it as a result of a call to
 * Result::flatMap()
 */
inline auto scalar(std::string const& name) {
    return [&name] (ValueContext const& vctx) {
        return vctx.getScalar(name);
    };
}

/*!
 * \brief Returns a mapping function which can be used to convert the
 * returned ValueContext into a MappingContext using Result::flatMap().
 *
 * If the result object does not contain a ValueContext, the call to
 * Result::flatMap() will propagate the original error.
 *
 * If the ValueContext is not a mapping, the result of the call to
 * Result::flatMap() will contain the error describing the issue.
 *
 * Example:
 *
 * ```cpp
 * pimc::Result<MappingContext, ErrorContext> scalarField(
 *         MappingContext const& mapCtx, std::string const& field) {
 *     return mapCtx.required(field).flatMap(mapping());
 * }
 * ```
 *
 * @return a function which applies ValueContext::getMapping() to
 * the ValueContext passed to it as a result of a call to
 * Result::flatMap()
 */
inline auto mapping() {
    return [] (ValueContext const& vctx) { return vctx.getMapping(); };
}

/*!
 * \brief Returns a mapping function which can be used to convert the
 * returned ValueContext into a MappingContext using Result::flatMap().
 *
 * The \p name parameter is the human readable description of the
 * mapping.
 *
 * If the result object does not contain a ValueContext, the call to
 * Result::flatMap() will propagate the original error.
 *
 * If the ValueContext is not a mapping, the result of the call to
 * Result::flatMap() will contain the error describing the issue.
 *
 * Example:
 *
 * ```cpp
 * pimc::Result<MappingContext, ErrorContext> scalarField(
 *         MappingContext const& mapCtx, std::string const& field) {
 *     return mapCtx.required(field).flatMap(mapping("host config"));
 * }
 * ```
 *
 * @return a function which applies ValueContext::getMapping() to
 * the ValueContext passed to it as a result of a call to
 * Result::flatMap()
 */
inline auto mapping(std::string name) {
    return [name = std::move(name)] (ValueContext const& vctx) mutable {
        return vctx.getMapping(std::move(name));
    };
}

/*!
 * \brief Returns a mapping function which can be used to convert the
 * returned ValueContext into a SequenceContext using Result::flatMap().
 *
 * If the result object does not contain a ValueContext, the call to
 * Result::flatMap() will propagate the original error.
 *
 * If the ValueContext is not a sequence, the result of the call to
 * Result::flatMap() will contain the error describing the issue.
 *
 * Example:
 *
 * ```cpp
 * pimc::Result<MappingContext, ErrorContext> scalarField(
 *         MappingContext const& mapCtx, std::string const& field) {
 *     return mapCtx.required(field).flatMap(sequence());
 * }
 * ```
 *
 * @return a function which applies ValueContext::getSequence() to
 * the ValueContext passed to it as a result of a call to
 * Result::flatMap()
 */
inline auto sequence() {
    return [] (ValueContext const& vctx) { return vctx.getSequence(); };
}

/*!
 * \brief Returns a mapping function which can be used to convert the
 * returned ValueContext into a SequenceContext using Result::flatMap().
 *
 * The \p name parameter is the human readable description of the
 * sequence.
 *
 * If the result object does not contain a ValueContext, the call to
 * Result::flatMap() will propagate the original error.
 *
 * If the ValueContext is not a sequence, the result of the call to
 * Result::flatMap() will contain the error describing the issue.
 *
 * Example:
 *
 * ```cpp
 * pimc::Result<MappingContext, ErrorContext> scalarField(
 *         MappingContext const& mapCtx, std::string const& field) {
 *     return mapCtx.required(field).flatMap(sequence("host addresses"));
 * }
 * ```
 *
 * @return a function which applies ValueContext::getSequence() to
 * the ValueContext passed to it as a result of a call to
 * Result::flatMap()
 */
inline auto sequence(std::string name) {
    return [name = std::move(name)] (ValueContext const& vctx) mutable {
        return vctx.getSequence(std::move(name));
    };
}

} // namespace pimc::yaml
