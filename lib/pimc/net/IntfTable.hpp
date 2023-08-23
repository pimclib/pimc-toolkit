#pragma once

#include <concepts>
#include <string>
#include <map>
#include <functional>
#include <algorithm>

#include "pimc/core/Optional.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/net/IP.hpp"
#include "pimc/net/IPv4Address.hpp"

namespace pimc {

struct IntfInfo {
    IntfInfo(char const* name_, unsigned ifindex_, Optional<IPv4Address> ipv4addr_)
    : name{name_}, ifindex{ifindex_}, ipv4addr{ipv4addr_} {}

    std::string name;
    unsigned ifindex;
    Optional<IPv4Address> ipv4addr;
};

class IntfTable final {
public:
    static Result<IntfTable, std::string> newTable();

    [[nodiscard]]
    auto byIndex(unsigned ifindex) const -> Optional<IntfInfo const&> {
        if (auto ii = indexMap_.find(ifindex); ii != indexMap_.end())
            return ii->second;

        return {};
    }

    [[nodiscard]]
    auto byName(std::string const& name) const -> Optional<IntfInfo const&> {
        if (auto ii = nameMap_.find(name); ii != nameMap_.end())
            return *ii->second;

        return {};
    }

    template <typename VF>
    void forEach(VF&& viewFn) const
    requires std::regular_invocable<VF, IntfInfo const&> {
        for (auto const& ii : indexMap_)
            std::invoke(std::forward<VF>(viewFn), ii.second);
    }

private:
    IntfTable(
            std::map<unsigned, IntfInfo> indexMap,
            std::map<std::string, IntfInfo const*> nameMap)
            : indexMap_{std::move(indexMap)}
            , nameMap_{std::move(nameMap)} {}
private:
    std::map<unsigned, IntfInfo> indexMap_;
    std::map<std::string, IntfInfo const*> nameMap_;
};

template <IPVersion V>
struct IPIntf {};

template <>
struct IPIntf<v4> {
    static constexpr std::string const& name(IntfInfo const& intfInfo) {
        return intfInfo.name;
    }

    static constexpr unsigned ifindex(IntfInfo const& intfInfo) {
        return intfInfo.ifindex;
    }

    static constexpr Optional<IPv4Address> address(IntfInfo const& intfInfo) {
        return intfInfo.ipv4addr;
    }
};

} // namespace pimc