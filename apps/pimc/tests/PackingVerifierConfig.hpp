#pragma once

#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <yaml-cpp/yaml.h>
#pragma GCC diagnostic pop

#include "pimc/yaml/ErrorHandler.hpp"

#include "config/JPConfig.hpp"
#include "config/JPConfigLoader.hpp"
#include "PackingVerifierConfigLoader.hpp"

namespace pimc::pimsm_config {

template <IPVersion V>
class PackingVerifierConfig final {
    template <IPVersion U>
    friend PackingVerifierConfig<U> parse(char const* cfg);
public:

private:
    PackingVerifierConfig(JPConfig<V> jpCfg, std::vector<JPConfig<V>> pktCfgs)
    : jpCfg_{std::move(jpCfg)}, pktCfgs_{std::move(pktCfgs)} {}
private:
    JPConfig<V> jpCfg_;
    std::vector<JPConfig<V>> pktCfgs_;
};

struct ThrowingErrorHandler: public yaml::ErrorHandler {

    ThrowingErrorHandler(): yaml::ErrorHandler{""} {}

    void showError(yaml::ErrorContext const& ectx) final {
        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);
        fmt::format_to(bi, "at line {}: ", ectx.line());
        fmt::format_to(bi, "{}", ectx.message());
        mb.push_back('\n');
        mb.push_back(static_cast<char>(0));
        throw std::logic_error{mb.data()};
    }

    template <typename T>
    auto chkErrors(Result<T, std::vector<yaml::ErrorContext>> r)
    -> Result<T, std::vector<yaml::ErrorContext>> {
        if (not r) {
            auto& mb = getMemoryBuffer();
            auto bi = std::back_inserter(mb);
            for (auto const& ectx: r.error()) {
                fmt::format_to(bi, "at line {}: ", ectx.line());
                fmt::format_to(bi, "{}", ectx.message());
                mb.push_back('\n');
            }
            mb.push_back(static_cast<char>(0));
            throw std::logic_error{mb.data()};
        }

        return r;
    }

};

template <IPVersion V>
inline PackingVerifierConfig<V> parse(const char* cfg) {
    auto nodes = YAML::LoadAll(cfg);
    if (nodes.empty())
        throw std::logic_error{"empty packing verifier config"};

    if (nodes.size() > 1)
        throw std::logic_error{"multiple documents in packing verifier config"};

    auto vCtx = yaml::ValueContext::root(nodes[0]);

    ThrowingErrorHandler eh{};
    auto rPVefCfg = eh.chk(vCtx.getMapping("packing verifier config"));

    if (rPVefCfg) {
        auto rJPCtx = eh.chk(rPVefCfg->required("multicast"));

        if (rJPCtx) {
            auto rJPCfg = eh.chkErrors(loadJPConfig<V>(rJPCtx.value()));

            auto rPVCfgCtx = eh.chk(rPVefCfg->required("verify"));

            if (rPVCfgCtx) {
                auto rPktCfgs =
                        eh.chkErrors(loadPackingVerifierConfig<V>(rPVefCfg.value()));

                if (rPktCfgs)
                    return PackingVerifierConfig<V>{
                            std::move(rJPCfg).value(),
                            std::move(rPktCfgs).value()
                    };
            }
        }
    }

    throw std::logic_error{"*** unknown error ***"};
}

} // namespace pimc::pimsm_config
