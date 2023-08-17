#include <gtest/gtest.h>

#include "pimsm/Pack.hpp"
#include "pimsm/UpdateFormatter.hpp"

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

    int ii{0};
    for (auto const& vcf: vcfs) {
        if (ii++ != 5) continue;
        // TODO debug
        fmt::print("** TEST: {}\n", vcf.name());
        auto updates = pack(vcf.jpConfig());
        auto exp = updatesText(vcf.updates());
        auto eff = updatesText(updates);
        EXPECT_TRUE(exp == eff)
          << "\n"
          << "Test: " << vcf.name() << "\n\n"
          << "Expecting:\n\n" << exp << "Received:\n\n" << eff;
    }
}

} // namespace pimc::testing