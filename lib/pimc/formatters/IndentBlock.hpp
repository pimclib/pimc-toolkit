#pragma once

#include <cctype>
#include <concepts>
#include <functional>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <string_view>

#include "pimc/core/CompilerUtils.hpp"
#include "pimc/system/Exceptions.hpp"
#include "ConsumeIfUnlessEscaped.hpp"

namespace pimc {

/*!
 * Copies an indented version of the text block into the specified output
 * iterator. The following rules apply:
 *
 * #. If original block of text has nothing but white space characters, nothing
 *    is sent to the output and the returned result contains the input iterator
 *    positioned to the end of the input and the output iterator unchanged.
 * #. Otherwise the first line of the indented block is indented by the number
 *    of space characters equal to the ``line1indent`` argument, whereas all
 *    subsequent lines are indented by the number of spaces equal to the ``indent``
 *    argument
 * #. The text is broken into words separated by the whitespace characters, which
 *    are identified using the predicate ``ws``. If some whitespace characters are
 *    to be treated as non-whitespace, should should be escaped using the character
 *    for which the predicate ``esc`` returns true.
 *
 *
 * @tparam I the symbol iterator type
 * @tparam S the sentinel for the symbol iterator type
 * @tparam WS a white-space predicate type
 * @tparam ESC an escape predicate type
 * @tparam O thd output iterator type
 * @tparam NLI the new line sequence iterator type
 * @tparam NLS the sentine for the new line sequence iterator type
 * @param start the input iterator pointing to the sequence of symbols
 * @param end the end sentinel for the sequence of symbols
 * @param ws the predicate which deterministically identifies white space characters
 * @param esc the predicate which deterministically identifies the escape character
 * @param oi the output iterator
 * @param line1indent the size of the first line indent
 * @param line1maxWidth the maximum width of the first line, which must be greater than
 *   the indent of the first line
 * @param indent the size of indent for the subsequent lines
 * @param maxWidth the maximum with, which must be greater than the indent
 * @param sp the space symbol
 * @param nlStart the start of the new line sequence
 * @param nlEnd the end of the new line sequence
 * @return a ranges in/out result holding the new position of the input iterator and the
 *   new position of the output iterator
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::indirect_unary_predicate<I> WS,
        std::indirect_unary_predicate<I> ESC,
        std::weakly_incrementable O,
        std::input_iterator NLI,
        std::sentinel_for<NLI> NLS>
        requires std::indirectly_copyable<I, O> && std::indirectly_copyable<NLI, O>
std::ranges::in_out_result<I, O> indentBlock(
        I start, S end, WS&& ws, ESC&& esc, O oi,
        unsigned line1indent, unsigned line1maxWidth,
        unsigned indent, unsigned maxWidth,
        std::iter_value_t<I> sp, NLI nlStart, NLS nlEnd) {
    if (PIMC_UNLIKELY(line1indent >= line1maxWidth))
        raise<std::invalid_argument>(
                "line1indent ({}) may not be greater or equal to line1maxWidth ({})",
                line1indent, line1maxWidth);
    if (PIMC_UNLIKELY(indent >= maxWidth))
        raise<std::invalid_argument>(
                "indent ({}) may not be greater or equal to maxWidth ({})",
                indent, maxWidth);
    // Before appending the first block of line1indent we need to make sure
    // if there are any printable characters at all
    auto notWS = [&ws] (auto const& c) { return not std::invoke(std::forward<WS>(ws), c); };
    auto copySymb = [&oi] (auto const& c) { *oi = c; return ++oi; };
    auto noop = [] (auto const&) {};

    auto pchr = std::ranges::find_if_not(start, end, std::forward<WS>(ws));
    if (pchr == end)
        // No printable characters found, the iterator pointing to the end of
        // input and the output iterator unchanged
        return {pchr, oi};

    bool empty{true};
    unsigned curPos = 0;
    bool firstLine{true};
    unsigned indsz = line1indent;
    unsigned width = line1maxWidth;

    while (pchr != end) {
        // At the start of the look pchr always points to the first
        // printable character
        unsigned wordLen;
        I wschr;
        std::tie(wordLen, wschr) = consumeIfUnlessEscaped(pchr, end, notWS, esc, noop);

        if (not empty) {
            if (curPos + wordLen + 1 < width) {
                oi = std::fill_n(oi, 1, sp);
                ++curPos;
            } else {
                // This is the case where the current word would go over
                // the maxWidth, so we have to output the <nl><indent>
                // sequence first
                if (not firstLine) {
                    auto cr = std::ranges::copy(nlStart, nlEnd, oi);
                    oi = cr.out;
                    width = maxWidth;
                }
                else firstLine = false;
                oi = std::fill_n(oi, indsz, sp);
                curPos = indsz;
                indsz = indent;
            }

            consumeIfUnlessEscaped(pchr, end, notWS, esc, copySymb);
            if (curPos + wordLen < width) {
                curPos += wordLen;
            } else {
                // This is the case where the word, even though it is
                // output after the <nl><indent> sequence, still overflows
                // the maxWidth
                empty = true;
                curPos = 0;
            }
        } else {
            // Because we start with the empty state we have to
            // first output <nl><indent> sequence
            if (not firstLine) {
                auto cr = std::ranges::copy(nlStart, nlEnd, oi);
                oi = cr.out;
                width = maxWidth;
            }
            else firstLine = false;
            oi = std::fill_n(oi, indsz, sp);
            curPos = indsz;
            indsz = indent;

            // Only then we output the word
            consumeIfUnlessEscaped(pchr, end, notWS, esc, copySymb);
            if (curPos + wordLen < width) {
                curPos += wordLen;
                empty = false;
            }
        }

        pchr = std::ranges::find_if_not(wschr, end, std::forward<WS>(ws));
    }

    return {pchr, oi};
}

/*!
 * A range specific overload of ``indentBlock``.
 *
 * @tparam R a range of symbols type
 * @tparam WS a white-space predicate type
 * @tparam ESC an escape predicate type
 * @tparam O thd output iterator type
 * @tparam NL a range of symbols type for the new line sequence
 * @param start the input iterator pointing to the sequence of symbols
 * @param end the end sentinel for the sequence of symbols
 * @param ws the predicate which deterministically identifies white space characters
 * @param esc the predicate which deterministically identifies the escape character
 * @param oi the output iterator
 * @param line1indent the size of the first line indent
 * @param line1maxWidth the maximum width of the first line, which must be greater than
 *   the indent of the first line
 * @param indent the size of indent for the subsequent lines
 * @param maxWidth the maximum with, which must be greater than the indent
 * @param sp the space symbol
 * @param nl the new line sequence range
 * @return a ranges in/out result holding the new position of the input iterator and the
 *   new position of the output iterator
 */
template <
        std::ranges::input_range R,
        std::indirect_unary_predicate<std::ranges::iterator_t<R>> WS,
        std::indirect_unary_predicate<std::ranges::iterator_t<R>> ESC,
        std::output_iterator<std::ranges::range_value_t<R>> O,
        std::ranges::input_range NL>
        requires std::indirectly_copyable<std::ranges::iterator_t<R>, O> &&
                 std::indirectly_copyable<std::ranges::iterator_t<NL>, O>
auto indentBlock(
        R&& r, WS&& ws, ESC&& esc, O oi,
        unsigned line1indent, unsigned line1maxWidth,
        unsigned indent, unsigned maxWidth,
        std::ranges::range_value_t<R> sp, NL&& nl
        ) -> std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O> {
    return indentBlock(
            std::ranges::begin(std::forward<R>(r)), std::ranges::end(std::forward<R>(r)),
            std::forward<WS>(ws), std::forward<ESC>(esc),
            oi, line1indent, line1maxWidth, indent, maxWidth,
            sp, std::ranges::begin(nl), std::ranges::end(nl));
}

/*!
 * This struct is a namespace for the three typical primitives
 * for the use with the ``indentBlock`` functions:
 *   - ``esc()`` which designations ``\`` as the escape character
 *   - ``ww()`` which uses ``std::isspace()`` to identify the
 *      whitespace characters
 *   - ``uxnl``, which is a stringview of the character ``\n``
 */
struct stdstr final {
    stdstr() = delete;

    /*!
     * Returns true if c is ``\``, false otherwise
     * @param c the character to test
     * @return true if c is ``\``, false otherwise
     */
    static constexpr inline bool esc(char c) { return c == '\\'; }

    /*!
     * Returns true if c is a standard whitespace character, false
     * otherwise.
     *
     * @param c the character to test
     * @return true if c is a standar whitespace character, false
     * otherwise
     */
    static inline bool ws(char c) { return std::isspace(c); }

    /*!
     * The UNIX end of line sequence, which is a single character
     * ``\n``
     */
    static inline std::string_view uxnl{"\n"};
};

} // namespace pimc
