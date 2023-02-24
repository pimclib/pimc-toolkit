#pragma once

#include <iterator>
#include <ranges>
#include <functional>
#include <tuple>

namespace pimc {

/**
 * Reads the symbols starting at the ``first`` iterator for as long as the
 * symbols pass the ``accept`` predicate and passes the symbols to the
 * ``cons`` function. If a symbol that would not pass the ``accept``
 * predicate needs to be excepted and sent to ``cons`` anyway, it should be
 * escaped with the preceding escape symbol, that is a symbol for which the
 * ``escape`` predicate returns true.
 *
 * @tparam I a type conforming to the input iterator requirements
 * @tparam S a type which is the sentinel for I
 * @tparam Accept a predicate which takes a symbol
 * @tparam Escape a predicate which takes a symbol
 * @tparam Consume a function that consumes a single symbol
 * @param first the beginning of the symbol sequence
 * @param last the end of the symbol sequence
 * @param accept the predicate which accepts symbols
 * @param escape the predicate which designates symbols as escape
 * @param cons the function which receives symbols
 * @return a tuple of the adjusted for escaping length of accepted input and
 *  the input iterator position right after the last accepted symbol
 */
template <
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::indirect_unary_predicate<I> Accept,
        std::indirect_unary_predicate<I> Escape,
        std::indirectly_unary_invocable<I> Consume>
std::tuple<unsigned, I> consumeIfUnlessEscaped(
        I first, S last, Accept&& accept, Escape&& escape, Consume&& cons) {
    unsigned count{0};
    bool escaping{false};
    std::iter_value_t<I> e;
    while(first != last) {
        auto c = *first;
        if (not escaping) {
            if (std::invoke(std::forward<Escape>(escape), c)) {
                e = c;
                escaping = true;
            } else {
                if (not std::invoke(std::forward<Accept>(accept), c))
                    return {count, first};
                std::invoke(std::forward<Consume>(cons), c);
                ++count;
            }
        } else {
            if (std::invoke(std::forward<Accept>(accept), c) and
                not std::invoke(std::forward<Escape>(escape), c)) {
                std::invoke(std::forward<Consume>(cons), e);
                ++count;
            }
            escaping = false;
            std::invoke(std::forward<Consume>(cons), c);
            ++count;
        }

        ++first;
    }
    if (escaping) {
        std::invoke(std::forward<Consume>(cons), e);
        ++count;
    }
    return {count, first};
}

/**
 * Reads the symbols from the specified range for as long as they pass
 * the ``accept`` predicate and sends them to the ``cons`` function.
 * If a symbol that would not pass the ``accept`` predicate needs to be
 * excepted and sent to ``cons`` anyway, it should be escaped with the
 * preceding escape symbol, that is a symbol for which the ``escape``
 * predicate returns true.
 *
 * @tparam R a type representing a range of symbols
 * @tparam Accept a predicate which takes a symbol
 * @tparam Escape a predicate which takes a symbol
 * @tparam Consume a function that consumes a single symbol
 * @param r the range of symbols
 * @param accept the predicate which accepts symbols
 * @param escape the predicate which designates symbols as escape
 * @param cons the function which receives symbols
 * @return a tuple of the adjusted for escaping length of accepted input and
 *  the input iterator position right after the last accepted symbol
 */
template <
        std::ranges::input_range R,
        std::indirect_unary_predicate<std::ranges::iterator_t<R>> Accept,
        std::indirect_unary_predicate<std::ranges::iterator_t<R>> Escape,
        std::indirectly_unary_invocable<std::ranges::iterator_t<R>> Consume>
std::tuple<unsigned, std::ranges::borrowed_iterator_t<R>> consumeIfUnlessEscaped(
        R&& r, Accept&& accept, Escape&& escape, Consume&& cons) {
    return consumeIfUnlessEscaped(
            std::ranges::begin(std::forward<R>(r)), std::ranges::end(std::forward<R>(r)),
            std::forward<Accept>(accept), std::forward<Escape>(escape),
            std::forward<Consume>(cons));
}

} // namespace pimc