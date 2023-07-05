#pragma once

#include <utility>
#include <unordered_set>
#include <string>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

#include "pimc/core/CompilerUtils.hpp"
#include "pimc/core/Optional.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/system/Exceptions.hpp"
#include "pimc/text/MemoryBuffer.hpp"

using namespace std::string_literals;

namespace pimc {

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
    PIMC_ALWAYS_INLINE
    int line() const {
        return PIMC_LIKELY(node_.Mark().line != -1) ? node_.Mark().line + 1 : -1;
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
        int lineNo = line();
        std::string context;

        auto &mbctx = getMemoryBuffer();
        auto bictx = std::back_inserter(mbctx);
        for (auto const& name: ctx_)
            fmt::format_to(bictx, "{}: ", name);
        context = fmt::to_string(mbctx);

        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        return {lineNo, std::move(context), fmt::to_string(mb)};
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

protected:
    YAML::Node node_;
    std::vector<std::string> ctx_;
};

/*!
 * An object which contains the string value of a YAML scalar and
 * the line in the YAML source code where the scalar was parsed.
 */
class ScalarRef final {
public:
    ScalarRef(std::string value, int line)
            : value_{std::move(value)}, line_{line} {}

    /*!
     * \brief Returns the const reference to the value of the scalar.
     *
     * @return the const reference to the value of the scalar
     */
    [[nodiscard]]
    std::string const& value() const& { return value_; }

    /*!
     * \brief Returns an rvalue reference to the value of the scalar.
     *
     * @return an rvalue reference to the value of the scalar
     */
    [[nodiscard]]
    std::string value() && { return std::move(value_); }

    /*!
     * \brief Returns the line in the YAML source where the scalar
     * was parsed.
     *
     * If the YAML scalar was added programmatically, the returnved
     * value is -1.
     *
     * @return the line in the YAML source
     */
    [[nodiscard]]
    int line() const { return line_; }
private:
    std::string value_;
    int line_;
};

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
    auto getScalar(std::string const& name) const -> Result<ScalarRef, ErrorContext>;

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
    auto getScalar() const -> Result<ScalarRef, ErrorContext>;

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
    auto required(std::string const& field) -> Result<ValueContext, ErrorContext>;

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
    auto optional(std::string const& field) const -> Optional<ValueContext>;

    /*!
     * \brief If the YAML mapping contains unrecognized fields, returns a
     * result whose error describes the unrecognized fields.
     *
     * This function **must** be called after all of the calls to required()
     * and optional() have been made. This function considers any fields that
     * did not appear in the calls to required() and optional() as unrecognized.
     *
     * @return an empty result if no unrecognized fields have been detected
     * in the mapping, or a result containing an error describing the
     * unrecognized fields
     */
    auto unrecognized() const -> Result<void, ErrorContext>;
private:
    MappingContext(
            std::string name,
            YAML::Node const& node,
            std::vector<std::string> parentCtx_)
            : NodeContext{node, std::move(parentCtx_)}
            , name_{std::move(name)} {}

    std::string describe(std::string const& field) const;
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

private:
    SequenceContext(
            std::string name,
            YAML::Node const& node,
            std::vector<std::string> parentCtx_)
            : NodeContext{node, std::move(parentCtx_)}
            , name_{std::move(name)} {}

    std::string describe(size_t i) const;
private:
    std::string name_;
    using NodeContext::NodeContext;
};


auto scalar() {
    return [] (ValueContext const& vctx) { return vctx.getScalar(); };
}

auto scalar(std::string const& name) {
    return [&name] (ValueContext const& vctx) {
        return vctx.getScalar(name);
    };
}

auto mapping() {
    return [] (ValueContext const& vctx) { return vctx.getMapping(); };
}

auto mapping(std::string name) {
    return [name = std::move(name)] (ValueContext const& vctx) mutable {
        return vctx.getMapping(std::move(name));
    };
}

auto sequence() {
    return [] (ValueContext const& vctx) { return vctx.getSequence(); };
}

auto sequence(std::string name) {
    return [name = std::move(name)] (ValueContext const& vctx) mutable {
        return vctx.getSequence(std::move(name));
    };
}

} // namespace pimc
