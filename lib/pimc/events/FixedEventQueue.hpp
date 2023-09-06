#pragma once

#include <cstddef>
#include <utility>

#include "pimc/core/Meta.hpp"
#include "pimc/core/TupleUtils.hpp"

namespace pimc {

/*!
 * \brief A concept which expresses requirements for the event
 * handlers for FixedEventQueue
 *
 * @tparam EH The candidate event handler type
 */
template <typename EH>
concept EventHandler = requires(EH e) {
    { e.ready() } -> std::same_as<bool>;
    { e.fire() };
};

template <EventHandler ...> class FixedEventQueue;

namespace fixed_event_queue_detail {

/*!
 * \brief Event entry.
 *
 * @tparam EH the type that the entry holds must meet the EvenHandler
 * requirements
 */
template <size_t Idx, EventHandler EH>
class EventEntry {
public:
    template <StdTuple ArgsTuple>
    EventEntry(int, ArgsTuple&& args)
    : EventEntry{std::forward<ArgsTuple>(args),
                 std::make_index_sequence<std::tuple_size_v<ArgsTuple>>{}} {}

    [[nodiscard]]
    EH& value() { return eh_; }

private:
    template <typename ArgsTuple, size_t ... Idxs>
    EventEntry(ArgsTuple&& args, std::index_sequence<Idxs...>)
            : eh_{std::get<Idxs>(args)...} {}
private:
    EH eh_;
};

template <size_t Idx, typename ... Ts>
class FixedEventQueueImpl {};

template <size_t Idx, typename First, typename ... Rest>
class FixedEventQueueImpl<Idx, First, Rest...>:
        protected EventEntry<Idx, First>,
        protected FixedEventQueueImpl<Idx + 1, Rest...> {
    template <EventHandler ...>
    friend class ::pimc::FixedEventQueue;

    template <size_t, typename...> friend class FixedEventQueueImpl;

    template <StdTuple T, StdTuple ... Ts>
    FixedEventQueueImpl(int, T&& first, Ts ... rest)
    : EventEntry<0ul, First>{0ul, std::forward<T>(first)}
    , FixedEventQueueImpl<Idx + 1ul, Rest...>{0ul, std::forward<Ts>(rest)...} {}
};

template <size_t Idx, typename T>
class FixedEventQueueImpl<Idx, T>: protected EventEntry<Idx, T> {
    template <EventHandler ...>
    friend class FixedEventQueue;

    template <size_t, typename...> friend class FixedEventQueueImpl;

    template <StdTuple Last>
    FixedEventQueueImpl(int, Last&& last)
    : EventEntry<Idx, T>{0, std::forward<Last>(last)} {}
};

} // namespace fixed_event_queue_detail

/*!
 * \brief A fixed event queue is a tuple-like object, which stores event
 * handlers in its storage, and allows firing all of them in one go.
 *
 * @tparam EHs The types of event handler objects
 */
template <EventHandler ... EHs>
class FixedEventQueue final:
        fixed_event_queue_detail::FixedEventQueueImpl<0, EHs...> {
    using Base = fixed_event_queue_detail::FixedEventQueueImpl<0, EHs...>;
public:
    template <StdTuple ... Ts>
    explicit FixedEventQueue(Ts&& ... args)
    : Base{0ul, std::forward<Ts>(args)...} {}

    /*!
     * \brief returns a reference to the event handler with the index
     * \p Idx.
     *
     * \note This is a constant time function.
     *
     * @tparam Idx the index of the event handler to retrieve
     * @return a reference to the event handler with the specified
     * index
     */
    template <size_t Idx>
    constexpr auto elem() -> TypeAt_t<Idx, EHs...>& {
        static_assert(Idx >= sizeof...(EHs), "Event index out of bound");
        using EH = TypeAt_t<Idx, EHs...>;
        using ElemT = fixed_event_queue_detail::EventEntry<Idx, EH>;
        return static_cast<ElemT&>(*this).value();
    }

    /*!
     * \brief For each event handler this function first calls ready() and
     * if it returns true, it then calls fire() on that event handler.
     *
     * @return the number of events that fired
     */
    int runOnce() {
        return runOnceImpl(std::make_index_sequence<sizeof...(EHs)>{});
    }
private:

    template <size_t Idx>
    int runEvent() {
        auto& eh = elem<Idx>();
        if (eh.ready()) {
            eh.fire();
            return 1;
        }
        return 0;
    }

    template <size_t ... Idxs>
    int runOnceImpl(std::index_sequence<Idxs...>) {
        return (... + runEvent<Idxs>());
    }

private:
};

} // namespace pimc
