#pragma once

#include <concepts>
#include <vector>
#include <deque>

#include "pimc/net/IP.hpp"

#include "Update.hpp"

namespace pimc {

template <template <typename X> class UB, typename V>
concept UpdateBuilderImpl =
IPVersion<V> and requires(UB<V> ub) {
    UB{};
    { ub.full() } -> std::same_as<bool>;
    { ub.empty() } -> std::same_as<bool>;
};

template <template <typename X> class GEB, typename V>
concept GroupEntryBuilderImpl =
IPVersion<V> and requires(GEB<V> geb) {
    { geb.size() } -> std::same_as<size_t>;
    { geb.build() } -> std::same_as<std::vector<Update<V>>>;
};

template <IPVersion V, template <typename X> class UB>
        requires UpdateBuilderImpl<UB, V>
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
            requires GroupEntryBuilderImpl<GEB, V>
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

    std::deque<UB<V>>* ubq_;
    size_t& start_;
    size_t i_;
};


} // namespace pimc
