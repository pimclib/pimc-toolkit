#pragma once

#include <cstddef>
#include <utility>

#include "pimc/core/CompilerUtils.hpp"
#include "pimc/core/Meta.hpp"
#include "pimc/core/Result.hpp"
#include "pimc/core/TupleUtils.hpp"

namespace pimc {

/*!
 * \brief A concept which expresses requirements for the event
 * handlers for FixedEventQueue
 *
 * @tparam EH The candidate event handler type
 */
template <typename EH, typename E>
concept EventHandler = requires(EH e) {
    { e.ready() } -> std::same_as<bool>;
    { e.fire() } -> std::same_as<Result<void, E>>;
};

template <typename E, EventHandler<E> ...> class FixedEventQueue;

namespace fixed_event_queue_detail {

/*!
 * \brief Event entry.
 *
 * @tparam EH the type that the entry holds must meet the EvenHandler
 * requirements
 */
template <size_t Idx, typename E, EventHandler<E> EH>
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
            : eh_{std::get<Idxs>(std::forward<ArgsTuple>(args))...} {}
private:
    EH eh_;
};

template <size_t Idx, typename E, EventHandler<E> ... Ts>
class FixedEventQueueImpl {};

template <size_t Idx, typename E, EventHandler<E> First, EventHandler<E> ... Rest>
class FixedEventQueueImpl<Idx, E, First, Rest...>:
        protected EventEntry<Idx, E, First>,
        protected FixedEventQueueImpl<Idx + 1, E, Rest...> {
    template <typename E1, EventHandler<E1> ...>
    friend class ::pimc::FixedEventQueue;

    template <size_t, typename E1, EventHandler<E1> ...> friend class FixedEventQueueImpl;

    template <StdTuple T, StdTuple ... Ts>
    FixedEventQueueImpl(int, T&& first, Ts ... rest)
    : EventEntry<0ul, E, First>{0ul, std::forward<T>(first)}
    , FixedEventQueueImpl<Idx + 1ul, E, Rest...>{0ul, std::forward<Ts>(rest)...} {}
};

template <size_t Idx, typename E, EventHandler<E> T>
class FixedEventQueueImpl<Idx, E, T>: protected EventEntry<Idx, E, T> {
    template <typename E1, EventHandler<E1> ...>
    friend class FixedEventQueue;

    template <size_t, typename E1, EventHandler<E1> ...> friend class FixedEventQueueImpl;

    template <StdTuple Last>
    FixedEventQueueImpl(int, Last&& last)
    : EventEntry<Idx, E, T>{0, std::forward<Last>(last)} {}
};

} // namespace fixed_event_queue_detail

/*!
 * \brief A fixed event queue is a tuple-like object, which stores event
 * handlers in its storage, and allows firing all of them in one go.
 *
 * @tparam EHs The types of event handler objects
 */
template <typename E, EventHandler<E> ... EHs>
class FixedEventQueue final:
        fixed_event_queue_detail::FixedEventQueueImpl<0, E, EHs...> {
    using Base = fixed_event_queue_detail::FixedEventQueueImpl<0, E, EHs...>;
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
    PIMC_ALWAYS_INLINE
    constexpr auto elem() -> TypeAt_t<Idx, EHs...>& {
        static_assert(Idx < sizeof...(EHs), "Event index out of bound");
        using EH = TypeAt_t<Idx, EHs...>;
        using ElemT = fixed_event_queue_detail::EventEntry<Idx, E, EH>;
        return static_cast<ElemT&>(*this).value();
    }

    /*!
     * \brief For each event handler this function first calls ready() and
     * if it returns true, it then calls fire() on that event handler.
     *
     * If any of the event handlers' fire() function returns a result in
     * the error state, the error is returned right away, and no further
     * events are queried.
     *
     * @return a result containing the number of events that fired, or an
     * error even any of the events failed to fire
     */
    Result<unsigned, E> runOnce() {
        return chainEvents<0>(0u);
    }
private:

    template <size_t Idx>
    PIMC_ALWAYS_INLINE
    Result<unsigned, E> chainEvents(unsigned count) {
        if constexpr (Idx < sizeof...(EHs)) {
            auto &eh = elem<Idx>();
            if (eh.ready()) {
                auto r = eh.fire();

                if (r)
                    return chainEvents<Idx + 1>(count + 1);

                return std::move(r).map([]() -> unsigned { return 0u; });
            }

            return chainEvents<Idx + 1>(count);
        }
        return count;
    }

private:
};

} // namespace pimc
