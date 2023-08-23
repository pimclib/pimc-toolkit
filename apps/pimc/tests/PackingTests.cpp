#include <gtest/gtest.h>

#include "pimc/text/MemoryBuffer.hpp"

#include "pimsm/Pack.hpp"
#include "pimsm/UpdateFormatter.hpp"
#include "pimsm/UpdatesSanityCheck.hpp"

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
};

TEST_F(PackingTests, AllTests) {
    auto vcfs = pimsm_config::parse<IPv4>(pvConfigs);

    for (auto const& vcf: vcfs) {
        auto updates = pack(vcf.jpConfig());

        auto rc = verifyUpdates(vcf.jpConfig(), updates);
        if (not rc)
            ADD_FAILURE() << rc.error();

        auto exp = updatesText(vcf.updates());
        auto eff = updatesText(updates);

        if (exp != eff) {
            std::vector<UpdateSummary<IPv4>> expuss, rsltuss;
            expuss.reserve(vcf.updates().size());
            size_t i{1};
            for (auto const& u: vcf.updates())
                expuss.emplace_back(i++, u);
            i = 1;
            rsltuss.reserve(updates.size());
            for (auto const& u: updates)
                rsltuss.emplace_back(i++, u);

            auto& mb = getMemoryBuffer();
            auto bi = std::back_inserter(mb);

            fmt::format_to(bi, "Test: {}\n", vcf.name());

            fmt::format_to(bi, "\nExpecting:\n");
            for (auto const& us: expuss)
                fmt::format_to(bi, "{}", us);
            fmt::format_to(bi, "\nReceived:\n");
            for (auto const& us: rsltuss)
                fmt::format_to(bi, "{}", us);

            fmt::format_to(bi, "\nExpecting:\n{}\nReceived:\n{}", exp, eff);

            ADD_FAILURE() << fmt::to_string(mb);
        }
    }
}

} // namespace pimc::testing