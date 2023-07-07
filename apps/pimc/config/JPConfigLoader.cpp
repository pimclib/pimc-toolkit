#include "JPConfigLoader.hpp"
#include "pimc/yaml/StructuredYamlErrorHandler.hpp"

namespace pimc {

namespace {

struct JPConfigBuilder final: yaml::BuilderBase<JPConfigBuilder> {

    auto loadSharedTreeConfig(yaml::ValueContext const& sptCtx)
    -> Optional<SharedTree> {
        auto sptCfg = chk(sptCtx.getMapping("SPT config"));

        if (sptCfg) {

        }
    }

    void consume(yaml::ErrorContext ectx) {
        errors_.emplace_back(std::move(ectx));
    }

    std::vector<yaml::ErrorContext> errors_;
};

} // anon.namespace


auto loadJPConfig(yaml::ValueContext const& jpConfigCtx)
-> Result<JPConfig, std::vector<yaml::ErrorContext>> {

}

} // namespace pimc