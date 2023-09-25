#pragma once

#include <concepts>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <string>

#include "pimc/net/IP.hpp"

namespace pimc {

template<std::input_iterator I,
        std::sentinel_for<I> S,
        typename T>
std::string concat(I start, S end, T&& sep) {
    fmt::memory_buffer buf;
    auto bi = std::back_inserter(buf);
    bool separating = false;

    for (I ii{start}; ii != end; ++ii) {
        if (separating)
            fmt::format_to(bi, "{}", std::forward<T>(sep));
        else separating = true;
        fmt::format_to(bi, "{}", *(ii));
    }

    return fmt::to_string(buf);
}

template<std::ranges::input_range R, typename T>
std::string concat(R&& r, T&& sep) {
    return concat(r.begin(), r.end(), std::forward<T>(sep));
}

template<IPVersion V>
auto compareAddrSets(
        std::unordered_set<typename IP<V>::Address> const &a,
        std::unordered_set<typename IP<V>::Address> const &b)
-> std::tuple<std::set<typename IP<V>::Address>, std::set<typename IP<V>::Address>> {
    using IPAddress = typename IP<V>::Address;
    std::set<IPAddress> missing;
    std::set<IPAddress> extraneous;

    for (auto const &addr: a) {
        if (not b.contains(addr)) missing.emplace(addr);
    }

    for (auto const &addr: b) {
        if (not a.contains(addr)) extraneous.emplace(addr);
    }

    return {missing, extraneous};
}

template<IPVersion V, typename T>
auto keySet(std::unordered_map<typename IP<V>::Address, T> const &m)
-> std::unordered_set<typename IP<V>::Address> {
    std::unordered_set<typename IP<V>::Address> keys;
    for (auto const &ii: m) keys.emplace(ii.first);
    return keys;
}

template<IPVersion V>
auto vecSet(std::vector<typename IP<V>::Address> const &v)
-> std::unordered_set<typename IP<V>::Address> {
    std::unordered_set<typename IP<V>::Address> items;
    for (auto const &a: v) items.emplace(a);
    return items;
}

class ErrorTracker {
public:
    ErrorTracker() : failed_{false} {}

    template<typename ... Ts>
    void error(fmt::format_string<Ts...> const &fs, Ts &&...args) {
        auto bi = std::back_inserter(buf_);
        failed_ = true;
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        fmt::format_to(bi, "\n\n");
    }

    void append(std::string const &msg) {
        failed_ = true;
        buf_.append(msg);
    }

    [[nodiscard]]
    bool failed() const { return failed_; }

    [[nodiscard]]
    std::string msg() {
        return fmt::to_string(buf_);
    }

private:
    bool failed_;
    fmt::memory_buffer buf_;
};

namespace pimsm_detail {

template<IPVersion V>
class GroupBase {
protected:
    using IPAddress = typename IP<V>::Address;

protected:
    explicit GroupBase(IPAddress group) : group_{group}, failed_{false} {}

    template<typename ... Ts>
    void error(fmt::format_string<Ts...> const &fs, Ts &&... args) {
        auto bi = std::back_inserter(buf_);
        if (not failed_) {
            fmt::format_to(bi, "Group {}:\n", group_);
            failed_ = true;
        }

        fmt::format_to(bi, "  ");
        fmt::format_to(bi, fs, std::forward<Ts>(args)...);
        fmt::format_to(bi, "\n");
    }

    [[nodiscard]]
    bool failed() const { return failed_; }

    [[nodiscard]]
    std::string msg() {
        buf_.push_back('\n');
        return fmt::to_string(buf_);
    }

protected:
    IPAddress group_;

private:
    bool failed_;
    fmt::memory_buffer buf_;
};

} // namespace pimsm_detail

} // namespace pimc
