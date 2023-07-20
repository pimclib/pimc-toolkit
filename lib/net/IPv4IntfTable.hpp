#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <algorithm>

#include "pimc/core/Result.hpp"
#include "pimc/net/IPv4Address.hpp"

namespace pimc {

struct IPv4IntfInfo {
    std::string name;
    unsigned ifindex;
    IPv4Address address;
};

class IPv4IntfTable final {
public:
    using IntfInfoResult = std::optional<std::reference_wrapper<IPv4IntfInfo const>>;

    static Result<IPv4IntfTable, std::string> newTable();

    [[nodiscard]]
    auto byIndex(unsigned ifindex) const -> IntfInfoResult {
        if (auto ii = indexMap_.find(ifindex); ii != indexMap_.end())
            return std::cref(*ii->second);

        return {};
    }

    [[nodiscard]]
    auto byName(std::string const& name) const -> IntfInfoResult {
        if (auto ii = nameMap_.find(name); ii != nameMap_.end())
            return std::cref(*ii->second);

        return {};
    }

    [[nodiscard]]
    auto ifIndexes() const -> std::vector<unsigned> {
        std::vector<unsigned> indexes;
        indexes.reserve(intfs_.size());
        auto bi = std::back_inserter(indexes);
        std::ranges::transform(
                intfs_, bi,
                [] (auto const& ifInfo) { return ifInfo.ifindex; });
        return indexes;
    }

private:
    IPv4IntfTable(
            std::vector<IPv4IntfInfo> intfs,
            std::unordered_map<unsigned, IPv4IntfInfo const*> indexMap,
            std::unordered_map<std::string, IPv4IntfInfo const*> nameMap)
            : intfs_{std::move(intfs)}
            , indexMap_{std::move(indexMap)}
            , nameMap_{std::move(nameMap)} {}
private:
    std::vector<IPv4IntfInfo> intfs_;
    std::unordered_map<unsigned, IPv4IntfInfo const*> indexMap_;
    std::unordered_map<std::string, IPv4IntfInfo const*> nameMap_;
};


} // namespace pimc