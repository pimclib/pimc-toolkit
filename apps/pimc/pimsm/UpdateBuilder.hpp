#pragma once

#include <deque>
#include <algorithm>

#include "pimc/system/Exceptions.hpp"

#include "Update.hpp"

namespace pimc::pimsm_detail {

template <net::IPAddress A>
class UpdateBuilder final {
public:
    UpdateBuilder(size_t initSz, size_t capacity)
    : sz_{initSz}, capacity_{capacity} {}

    void join(GroupEntry<A> group, size_t sz) {
        chkSz(sz);
        joins_.emplace_back(std::move(group));
        sz_ += sz;
    }

    void prune(GroupEntry<A> group, size_t sz) {
        chkSz(sz);
        prunes_.emplace_back(std::move(group));
        sz_ += sz;
    }

    [[nodiscard]]
    size_t remaining() const {
        return static_cast<size_t>(capacity_ - sz_);
    }

    Update<A> build() {
        std::vector<GroupEntry<A>> joins;
        std::vector<GroupEntry<A>> prunes;

        joins.reserve(joins_.size());
        std::ranges::copy(joins_, std::back_inserter(joins));
        prunes.reserve(prunes_.size());
        std::ranges::copy(joins_, std::back_inserter(joins));
        return {std::move(joins), std::move(prunes)};
    }
private:
    void chkSz(size_t sz) {
        if (sz_ + sz > capacity_)
            raise<std::logic_error>(
                    "pim-update capacity {}, current size {}, update size {}",
                    capacity_, sz_, sz);
    }
private:
    std::deque<GroupEntry<A>> joins_;
    std::deque<GroupEntry<A>> prunes_;
    size_t sz_;
    size_t capacity_;
};

} // namespace pimc::pimsm_detail