#include "StructuredYaml.hpp"

namespace pimc {

auto ValueContext::getScalar(std::string const& name)
const -> Result<ScalarRef, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return fail(error("expecting a scalar, not {}", typ));
        return fail(error("{} must be a scalar, not {}", name, typ));
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

auto ValueContext::getScalar()
const -> Result<ScalarRef, ErrorContext> {
    return getScalar(""s);
}

auto ValueContext::getMapping(std::string name)
const -> Result<MappingContext, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return fail(error("expecting a mapping, not {}", typ));
        return fail(error("{} must be a mapping, not {}", name, typ));
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

auto ValueContext::getMapping()
const -> Result<MappingContext, ErrorContext> {
    return getMapping(""s);
}

auto ValueContext::getSequence(std::string name)
const -> Result<SequenceContext, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return fail(error("expecting a sequence, not {}", typ));
        return fail(error("{} must be a sequence, not {}", name, typ));
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

auto ValueContext::getSequence() const
-> Result<SequenceContext, ErrorContext> {
    return getSequence(""s);
}

auto MappingContext::required(std::string const& field)
-> Result<ValueContext, ErrorContext> {
    knownFields_.emplace(field);
    auto node = node_[field];

    if (not node.IsDefined())
        return fail(error("{} is required", describe(field)));

    return ValueContext{node, ctx_, describe(field)};
}

auto MappingContext::optional(std::string const& field)
const -> Optional<ValueContext> {
    knownFields_.emplace(field);
    auto node = node_[field];

    if (not node.IsDefined()) return {};
    return ValueContext{node, ctx_, describe(field)};
}

auto MappingContext::unrecognized() const -> Result<void, ErrorContext> {
    std::set<std::string> uflds;
    for (auto ii = node_.begin(); ii != node_.end(); ++ii) {
        auto fn = ii->first.as<std::string>();
        if (not knownFields_.contains(fn))
            uflds.emplace(std::move(fn));
    }

    if (uflds.empty()) return {};

    if (name_.empty())
        return fail(error(
                "unrecognized fields: {}",
                fmt::join(uflds, ", ")));

    return fail(error(
            "unrecognized fields in {}: {}",
            name_, fmt::join(uflds, ", ")));
}

std::string MappingContext::describe(std::string const& field) const {
    if (name_.empty())
        return fmt::format("field {}", field);

    return fmt::format("field {} of {}", field, name_);
}

auto SequenceContext::operator[] (size_t i)
const -> Result<ValueContext, ErrorContext>{
    if (i < node_.size())
        return ValueContext{node_[i], ctx_, describe(i)};

    return fail(error(
            "{} does not exist (sequence size is {})",
            describe(i), node_.size()));
}


std::string SequenceContext::describe(size_t i) const {
    if (name_.empty())
        return fmt::format("element #{}", i);

    return fmt::format("element #{} of {}", i, name_);
}

} // namespace pimc
