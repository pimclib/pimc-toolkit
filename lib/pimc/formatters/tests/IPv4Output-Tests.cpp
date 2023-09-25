#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include "pimc/net/IPv4Address.hpp"
#include "pimc/net/IPv4Prefix.hpp"
#include "pimc/formatters/IPv4Ostream.hpp"

using namespace std::string_literals;

namespace pimc::testing {

class IPv4OStreamTests: public ::testing::Test {
protected:
};

TEST_F(IPv4OStreamTests, Basic) {
    IPv4Address a{224,1,2,3};
    std::ostringstream ossA;
    ossA << a;
    auto as = std::move(ossA).str();
    EXPECT_EQ(as, "224.1.2.3"s)
     << "address " << as << " != 224.1.2.3";
}

} // namespace pimc::testing
