#include <unordered_map>
#include "Structured.hpp"

namespace pimc::yaml {

auto ValueContext::getScalar(std::string const& name)
const -> Result<ScalarContext, ErrorContext> {
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
        return ScalarContext{node_, ctx_};
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


auto MappingContext::required(std::string const& field) const
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

auto MappingContext::items() const
-> std::vector<std::pair<ValueContext, ValueContext>> {
    if (node_.size() == 0) return {};

    std::vector<std::pair<ValueContext, ValueContext>> ks;
    ks.reserve(node_.size());
    for (auto ii = node_.begin(); ii != node_.end(); ++ii)
        ks.emplace_back(ValueContext{ii->first, ctx_}, ValueContext{ii->second, ctx_});
    return ks;
}

auto MappingContext::extraneous() const -> std::vector<ErrorContext> {
    std::vector<ErrorContext> errors;
    std::unordered_map<std::string, int> observed;

    for (auto ii = node_.begin(); ii != node_.end(); ++ii) {
        YAML::Node const& n = ii->first;
        auto line = nodeLine(n);
        Optional<std::string> key;

        switch(n.Type()) {
        case YAML::NodeType::Undefined:
            errors.emplace_back(
                    errorAt(line, "mapping key may not be undefined"));
            break;
        case YAML::NodeType::Null:
            errors.emplace_back(
                    errorAt(line, "mapping key may not be null"));
            break;
        case YAML::NodeType::Scalar:
            key = n.as<std::string>();
            break;
        case YAML::NodeType::Sequence:
            errors.emplace_back(
                    errorAt(line, "mapping key may not be a sequence"));
            break;
        case YAML::NodeType::Map:
            errors.emplace_back(
                    errorAt(line, "mapping key may not be another mapping"));
            break;
        default:
            errors.emplace_back(
                    errorAt(line, "unrecognized mapping key type"));
        }

        if (key) {
            auto const& fn = key.value();
            if (not knownFields_.contains(fn)) {
                if (name_.empty())
                    errors.emplace_back(
                            errorAt(line, "unrecognized field '{}'", fn));
                else
                    errors.emplace_back(
                            errorAt(line, "unrecognized field '{}' in {}", fn, name_));
            }

            auto nfi = observed.try_emplace(std::move(key).value(), line);
            if (not nfi.second) {
                if (name_.empty())
                    errors.emplace_back(
                            errorAt(line,
                                    "duplicate field '{}', "
                                    "previously seen at line {}",
                                    nfi.first->first, nfi.first->second));
                else
                    errors.emplace_back(
                            errorAt(line,
                                    "duplicate field '{}' in {}, "
                                    "previously seen at line {}",
                                    nfi.first->first, name_, nfi.first->second));
            }
        }
    }

    return errors;
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
