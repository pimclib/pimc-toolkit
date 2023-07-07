#include "Structured.hpp"

namespace pimc::yaml {

auto ValueContext::getScalar(std::string const& name)
const -> Result<Scalar, ErrorContext> {
    auto err = [this, &name] (char const* typ) {
        if (name.empty())
            return fail(error("expecting a scalar, not {}", typ));
        return fail(error("{} must be a scalar, not {}", name, typ));
    };
    auto t = node_.Type();
    switch (t) {
    case YAML::NodeType::Map:
        return err("a mapping");
    case YAML::NodeType::Undefined:
        return err("undefined");
    case YAML::NodeType::Null:
        return err("null");
    case YAML::NodeType::Scalar:
        return Scalar{node_.Scalar(), line()};
    case YAML::NodeType::Sequence:
        return err("a sequence");
    }

    raise<std::runtime_error>("unhandled yaml-cpp type {}", static_cast<int>(t));
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
        return err("a scalar");
    case YAML::NodeType::Sequence:
        return err("a sequence");
    }

    raise<std::runtime_error>("unhandled yaml-cpp type {}", static_cast<int>(t));
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
        return err("a mapping");
    case YAML::NodeType::Undefined:
        return err("undefined");
    case YAML::NodeType::Null:
        return err("null");
    case YAML::NodeType::Scalar:
        return err("a scalar");
    case YAML::NodeType::Sequence:
        return SequenceContext{std::move(name), node_, ctx_};
    }

    raise<std::runtime_error>("unhandled yaml-cpp type {}", static_cast<int>(t));
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

auto MappingContext::unrecognized() const -> std::vector<ErrorContext> {
    std::vector<ErrorContext> uflds;
    for (auto ii = node_.begin(); ii != node_.end(); ++ii) {
        YAML::Node const& n = ii->first;
        auto fn = n.as<std::string>();
        if (not knownFields_.contains(fn)) {
            if (name_.empty())
                uflds.emplace_back(
                        errorAt(nodeLine(n), "unrecognized field '{}'", fn));
            else uflds.emplace_back(
                    errorAt(nodeLine(n), "unrecognized field '{}' in {}", fn, name_));
        }
    }

    return uflds;
}

auto SequenceContext::operator[] (size_t i)
const -> Result<ValueContext, ErrorContext>{
    if (i < node_.size())
        return ValueContext{node_[i], ctx_, describe(i)};

    return fail(error(
            "{} does not exist (sequence size is {})",
            describe(i), node_.size()));
}

std::vector<ValueContext> SequenceContext::list() const {
    std::vector<ValueContext> elems;
    elems.reserve(node_.size());
    for (size_t i{0}; i < node_.size(); ++i)
        elems.emplace_back(ValueContext{node_[i], ctx_, describe(i)});
    return elems;
}

} // namespace pimc::yaml
