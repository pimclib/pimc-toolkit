#include <gtest/gtest.h>

#include "pimsm/Pack.hpp"

#include "PackingVerifierConfig.hpp"

namespace pimc::testing {

class PackingTests: public ::testing::Test {
protected:
};

namespace {
char const* basic1 = R"yaml(
---
multicast:
  239.1.2.3:
    Join*:
      RP: 192.168.1.5
      Prune:
        - 10.1.1.120
        - 10.1.1.90
        - 10.1.0.100
    Join:
      - 10.1.1.50
      - 10.1.1.52

verify:
  - 239.1.2.3:
      Join(*,G):
        - 192.168.1.5
      Join(S,G):
        - 10.1.1.50
        - 10.1.1.52
      Prune(S,G,rpt):
        - 10.1.1.120
        - 10.1.1.90
        - 10.1.0.100
)yaml";
} // anon.namespace

TEST_F(PackingTests, Basic1) {
    auto vcf = pimsm_config::parse<IPv4>(basic1);
    auto updates = pack(vcf.jpConfig());
}

} // namespace pimc::testing