#pragma once

#include <iterator>

#include "pimc/formatters/Fmt.hpp"
#include "pimc/text/SCLine.hpp"
#include "pimc/text/NumberLengths.hpp"
#include "pimc/net/IntfTable.hpp"

namespace pimc {

template<std::output_iterator<char> OI>
void formatIntfTable(
        OI oi, IntfTable const &intfTable, unsigned indent, bool eotNl = true) {
    size_t indexColW{5ul};
    size_t intfNameColW{9u};
    size_t addrColW{12};

    intfTable.forEach(
            [&indexColW, &intfNameColW, &addrColW] (IntfInfo const& intfInfo) {
                // TODO we're currently only show IPv4 interfaces,
                //      need to add support for IPv6
                if (intfInfo.ipv4addr) {
                    indexColW = std::max(intfNameColW, decimalUIntLen(intfInfo.ifindex));
                    intfNameColW = std::max(intfInfo.name.size(), intfNameColW);
                    addrColW = std::max(intfInfo.ipv4addr->charlen(), addrColW);
                }
            });

    SCLine<'='> sep{std::max({indexColW, intfNameColW, addrColW})};
    SCLine<' '> ind{indent};

    auto fmts = fmt::format(
            "{}{{:<{}}} {{:<{}}} {{:<{}}}",
            ind(), indexColW, intfNameColW, addrColW);

    fmt::format_to(
            oi, fmt::runtime(fmts), "Index", "Interface", "IPv4 Address");
    fmt::format_to(oi, "\n");;
    fmt::format_to(
            oi, fmt::runtime(fmts),
            sep(indexColW), sep(intfNameColW), sep(addrColW));

    intfTable.forEach(
            [&oi, &fmts] (IntfInfo const& intfInfo) {
                // TODO we're currently only show IPv4 interfaces,
                //      need to add support for IPv6
                if (intfInfo.ipv4addr) {
                    fmt::format_to(oi, "\n");
                    fmt::format_to(
                            oi, fmt::runtime(fmts),
                            intfInfo.ifindex, intfInfo.name, intfInfo.ipv4addr.value());
                }
            });

    if (eotNl) fmt::format_to(oi, "\n");
}


} // namespace pimc
