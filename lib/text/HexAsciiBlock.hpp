#pragma once

#include <cstdint>
#include <iterator>
#include <fmt/format.h>

namespace pimc {

/**
 * Formats the \p sz bytes of binary data starting at the pointer \p data
 * into an output object accessible through the output iterator \p oi. The
 * produced output contains a hex pane on the left and printable pane on the
 * right, similar to how `tcpdump` formats its output. The last character is
 * always `\n`
 *
 * @tparam OutputIter an output iterator type
 * @param oi the output iterator
 * @param data the data to format
 * @param sz the number of the bytes of the data
 */
template <std::output_iterator<char> OutputIter>
OutputIter formatHexAscii(OutputIter oi, uint8_t const* data, size_t sz) {
    for (unsigned start = 0; start < sz; start += 16) {
        size_t end = sz - start < 16 ? sz : start + 16;

        oi = fmt::format_to(oi, "  ");

        // hex view
        for (auto i = start; i < end; i++) {
            oi = fmt::format_to(oi, "{:02x} ", static_cast<unsigned>(data[i]));
            if (i - start == 7)
                oi = fmt::format_to(oi, " ");
        }

        // If the last row is shorter than 16 characters fill in
        // the missing hex values with blanks. Nothing needs to
        // be done with the payload_size is multiple of 16
        if (end == sz && sz % 16 != 0) {
            for (auto i = sz % 16; i < 16; i++) {
                if (i == 7) oi = fmt::format_to(oi, " ");
                oi = fmt::format_to(oi, "   ");
            }
        }

        oi = fmt::format_to(oi, " ");

        // char view
        for (auto i = start; i < end; i++) {
            if (std::isprint(data[i]))
                oi = fmt::format_to(oi, "{}", static_cast<char>(data[i]));
            else oi = fmt::format_to(oi, ".");
        }
        if (end != sz)
            oi = fmt::format_to(oi, "\n");
    }

    return oi;
}

} // namespace pimc
