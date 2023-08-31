#pragma once

#include <concepts>
#include <vector>
#include <deque>

#include "pimc/net/IP.hpp"

#include "Update.hpp"

namespace pimc {

template <typename V, template <typename X> class UB>
concept UpdateBuilder =
IPVersion<V> and requires(UB<V> ub, GroupEntry<V> ge, size_t sz) {
    UB<V>{};
    { ub.add(ge, sz) };
    { ub.full() } -> std::same_as<bool>;
    { ub.empty() } -> std::same_as<bool>;
};

template <typename V, template <typename X> class GEB>
concept GroupEntryBuilder =
IPVersion<V> and requires(GEB<V> geb) {
    { geb.size() } -> std::same_as<size_t>;
    { geb.build() } -> std::same_as<GroupEntry<V>>;
};

/*!
 * \brief An iterator like object, which provides access to the
 * underlying queue of the update builder objects and auto-appends
 * new builder objects at the end of the queue.
 *
 * @tparam V the IP version type
 * @tparam UB An IP version parameterized update builder class
 * template type
 */
template <IPVersion V, template <typename X> class UB>
requires UpdateBuilder<V, UB>
class UBCursor final {
public:

    constexpr UBCursor(std::deque<UB<V>>* ubq, size_t& start)
    : ubq_{ubq}, start_{start}, i_{start} {}

    auto operator++ () -> UBCursor& {
        if (++i_ >= ubq_->size())
            ubq_->emplace_back();

        return *this;
    }

    auto operator* () -> UB<V>& {
        return (*ubq_)[i_];
    }

    auto operator-> () -> UB<V>* {
        return addr();
    }

    auto addr() -> UB<V>* {
        return &((*ubq_)[i_]);
    }

    template <template <typename X> class GEB>
    requires GroupEntryBuilder<V, GEB>
    void add(GEB<V>& geb) {
        auto geSz = geb.size();
        (*ubq_)[i_].add(geb.build(), geSz);
        updateStart();
    }

    void updateStart() {
        for (size_t j = start_; j <= i_; ++j) {
            if ((*ubq_)[j].full())
                start_ = j;
            else return;
        }

        if (start_ == ubq_->size())
            ubq_->emplace_back();
    }

private:
    std::deque<UB<V>>* ubq_;
    size_t& start_;
    size_t i_;
};

template <IPVersion V, template <typename X> class UB>
UBCursor(std::deque<UB<V>>*, size_t&) -> UBCursor<V, UB>;

} // namespace pimc
