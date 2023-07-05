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

class NodeContext {
public:
    PIMC_ALWAYS_INLINE
    int line() const {
        return PIMC_LIKELY(node_.Mark().line != -1) ? node_.Mark().line + 1 : -1;
    }

    template <typename ... Ts>
    Failure<ErrorContext> error(
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
        return fail<ErrorContext>(lineNo, std::move(context), fmt::to_string(mb));
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

class ScalarRef final {
public:
    ScalarRef(std::string value, int line)
            : value_{std::move(value)}, line_{line} {}

    [[nodiscard]]
    std::string const& value() const& { return value_; }

    [[nodiscard]]
    std::string value() && { return std::move(value_); }

    [[nodiscard]]
    int line() const { return line_; }
private:
    std::string value_;
    int line_;
};

class MappingContext;
class SequenceContext;

class ValueContext final: public NodeContext {
    friend class MappingContext;
    friend class SequenceContext;
public:

    static ValueContext root(YAML::Node const& rootNode) {
        return {rootNode, {}};
    }

    [[nodiscard]]
    bool isDefined() const { return node_.IsDefined(); }

    [[nodiscard]]
    bool isNull() const { return node_.IsNull(); }

    [[nodiscard]]
    bool isScalar() const { return node_.IsScalar(); }

    [[nodiscard]]
    bool isSequence() const { return node_.IsSequence(); }

    [[nodiscard]]
    bool isMap() const { return node_.IsMap(); }

    auto getScalar(std::string const& name) const -> Result<ScalarRef, ErrorContext>;

    auto getScalar() const -> Result<ScalarRef, ErrorContext>;

    auto getMapping(std::string name) const -> Result<MappingContext, ErrorContext>;

    auto getMapping() const -> Result<MappingContext, ErrorContext>;

    auto getSequence(std::string name) const -> Result<SequenceContext, ErrorContext>;

    auto getSequence() const -> Result<SequenceContext, ErrorContext>;

private:
    using NodeContext::NodeContext;
};

class MappingContext final: public NodeContext {
    friend class ValueContext;
public:
    auto required(std::string const& field) -> Result<ValueContext, ErrorContext> {
        knownFields_.emplace(field);
        auto node = node_[field];

        if (not node.IsDefined())
            return error("{} is required", describe(field));

        return ValueContext{node, ctx_, describe(field)};
    }

    auto optional(std::string const& field) const -> Optional<ValueContext> {
        knownFields_.emplace(field);
        auto node = node_[field];

        if (not node.IsDefined()) return {};
        return ValueContext{node, ctx_, describe(field)};
    }

    auto unrecognized() const -> Result<void, ErrorContext> {
        std::set<std::string> uflds;
        for (auto ii = node_.begin(); ii != node_.end(); ++ii) {
            auto fn = ii->first.as<std::string>();
            if (not knownFields_.contains(fn))
                uflds.emplace(std::move(fn));
        }

        if (uflds.empty()) return {};

        if (name_.empty())
            return error(
                    "unrecognized fields: {}",
                    fmt::join(uflds, ", "));

        return error(
                "unrecognized fields in {}: {}",
                name_, fmt::join(uflds, ", "));
    }
private:
    MappingContext(
            std::string name,
            YAML::Node const& node,
            std::vector<std::string> parentCtx_)
            : NodeContext{node, std::move(parentCtx_)}
            , name_{std::move(name)} {}

    std::string describe(std::string const& field) const {
        if (name_.empty())
            return fmt::format("field {}", field);

        return fmt::format("field {} of {}", field, name_);
    }
private:
    std::string name_;
    mutable std::unordered_set<std::string> knownFields_;
};

class SequenceContext final: public NodeContext {
    friend class ValueContext;
public:
    size_t size() const { return node_.size(); }

    auto operator[] (size_t i) const -> Result<ValueContext, ErrorContext>{
        if (i < node_.size())
            return ValueContext{node_[i], ctx_, describe(i)};

        return error(
                "{} does not exist (sequence size is {})",
                describe(i), node_.size());
    }
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

auto ValueContext::getScalar(std::string const& name)
const -> Result<ScalarRef, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return error("expecting a scalar, not {}", typ);
        return error("{} must be a scalar, not {}", name, typ);
    };
    auto t = node_.Type();
    switch (t) {
    case YAML::NodeType::Map:
        return err("mapping");
    case YAML::NodeType::Undefined:
        return err("undefined");
    case YAML::NodeType::Null:
        return err("null");
    case YAML::NodeType::Scalar:
        return ScalarRef{node_.Scalar(), line()};
    case YAML::NodeType::Sequence:
        return err("sequence");
    }

    raise<std::runtime_error>("unhandled yaml-cpp type {}", static_cast<int>(t));
}

auto ValueContext::getScalar() const -> Result<ScalarRef, ErrorContext> {
    return getScalar(""s);
}

auto ValueContext::getMapping(std::string name)
const -> Result<MappingContext, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return error("expecting a mapping, not {}", typ);
        return error("{} must be a mapping, not {}", name, typ);
    };
    auto t = node_.Type();
    switch (t) {
    case YAML::NodeType::Map:
        return MappingContext{std::move(name), node_, ctx_};
    case YAML::NodeType::Undefined:
        return err("undefined");
    case YAML::NodeType::Null:
        return err("null");
    case YAML::NodeType::Scalar:
        return err("scalar");
    case YAML::NodeType::Sequence:
        return err("sequence");
    }

    raise<std::runtime_error>("unhandled yaml-cpp type {}", static_cast<int>(t));
}

auto ValueContext::getMapping() const -> Result<MappingContext, ErrorContext> {
    return getMapping(""s);
}

auto ValueContext::getSequence(std::string name)
const -> Result<SequenceContext, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return error("expecting a sequence, not {}", typ);
        return error("{} must be a sequence, not {}", name, typ);
    };
    auto t = node_.Type();
    switch (t) {
    case YAML::NodeType::Map:
        return err("mapping");
    case YAML::NodeType::Undefined:
        return err("undefined");
    case YAML::NodeType::Null:
        return err("null");
    case YAML::NodeType::Scalar:
        return err("scalar");
    case YAML::NodeType::Sequence:
        return SequenceContext{std::move(name), node_, ctx_};
    }

    raise<std::runtime_error>("unhandled yaml-cpp type {}", static_cast<int>(t));
}

auto ValueContext::getSequence() const -> Result<SequenceContext, ErrorContext> {
    return getSequence(""s);
}

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
