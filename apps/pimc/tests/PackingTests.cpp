#include <gtest/gtest.h>

#include "pimc/formatters//MemoryBuffer.hpp"

#include "pimsm/UpdateFormatter.hpp"
#include "pimsm/Pack.hpp"
#include "pimsm/PackSanityCheck.hpp"
#include "pimsm/InversePack.hpp"
#include "pimsm/InversePackSanityCheck.hpp"


#include "PackingVerifierConfig.hpp"

namespace pimc::testing {

#include "PackingVerifierConfigs.incl"

class PackingTests: public ::testing::Test {
protected:

    template <IPVersion V>
    [[nodiscard]]
    static std::string updatesText(std::vector<Update<V>> const& us) {
        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);

        int i{0};
        for (auto const& u: us)
            fmt::format_to(bi, "#{} {}\n", ++i, u);

        return fmt::to_string(mb);
    }

    static std::string fmtDiff(
            std::string const& name,
            std::string const& expu,
            std::vector<Update<IPv4>> const& expus,
            std::string const& effu,
            std::vector<Update<IPv4>> const& effus) {
        std::vector<UpdateSummary<IPv4>> expuss, effuss;
        expuss.reserve(expus.size());
        size_t i{1};
        for (auto const& u: expus)
            expuss.emplace_back(i++, u);
        i = 1;
        effuss.reserve(effus.size());
        for (auto const& u: effus)
            effuss.emplace_back(i++, u);

        auto& mb = getMemoryBuffer();
        auto bi = std::back_inserter(mb);

        fmt::format_to(bi, "Test: {}\n", name);

        fmt::format_to(bi, "\nExpecting:\n");
        for (auto const& us: expuss)
            fmt::format_to(bi, "{}", us);
        fmt::format_to(bi, "\nReceived:\n");
        for (auto const& us: effuss)
            fmt::format_to(bi, "{}", us);

        fmt::format_to(bi, "\nExpecting:\n{}\nReceived:\n{}", expu, effu);
        return fmt::to_string(mb);
    }
};

TEST_F(PackingTests, AllTests) {
    auto vcfs = parsePVConfigs<IPv4>(pvConfigs);

    for (auto const& vcf: vcfs) {
        auto updates = pack(vcf.jpConfig());

        auto rc = verifyUpdates(vcf.jpConfig(), updates);
        if (not rc)
            ADD_FAILURE() << rc.error();

        auto expu = updatesText(vcf.updates());
        auto effu = updatesText(updates);

        if (expu != effu)
            ADD_FAILURE() << fmtDiff(vcf.name(), expu, vcf.updates(), effu, updates);

        auto inverseUpdates = inversePack(vcf.jpConfig());

        auto irc = verifyInverseUpdates(vcf.jpConfig(), inverseUpdates);
        if (not irc)
            ADD_FAILURE() << irc.error();

        auto expiu = updatesText(vcf.inverseUpdates());
        auto effiu = updatesText(inverseUpdates);

        if (expiu != effiu)
            ADD_FAILURE() << fmtDiff(
                    vcf.name(), expiu, vcf.inverseUpdates(), effiu, inverseUpdates);
    }
}

} // namespace pimc::testing