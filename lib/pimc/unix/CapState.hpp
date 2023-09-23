#pragma once

/*
 * IMPORTANT:
 *
 * This file should be included *before* any of the files in
 * the "pimc/net" directory. Otherwise g++ reports errors for
 * the Linux types __u32, __le16, etc. in file
 * /usr/include/linux/capability.h, e.g.:
 *
 * /usr/include/linux/capability.h, 40 error: "__u32" does not name a type
 */

#include <concepts>

#ifdef WITH_LIBCAP
#include <unistd.h>
#include <sys/capability.h>
#endif

#include <string>

#include "pimc/core/Result.hpp"

#ifdef WITH_LIBCAP
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/Fmt.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

#define CAP_(v) std::forward_as_tuple(CAP_ ## v, static_cast<char const*>("CAP_" #v))

namespace pimc {

class CapState final {
private:
    using CapArg = std::tuple<cap_value_t&&, char const*&&>;

    struct CapFailure {
        char const* capName;
        std::string msg;
    };

    template <typename ... Ts>
    static Result<void, CapFailure> error(
            fmt::format_string<Ts...> const& fs, Ts&& ... args) {
        return fail(CapFailure {
            .capName = nullptr,
            .msg = fmt::format(fs, std::forward<Ts>(args)...)
        });
    }

    inline static Result<void, CapFailure> notSet(char const* capName) {
        return fail(CapFailure {
            .capName = capName,
            .msg = ""
        });
    }

    template <typename OI, std::same_as<CapArg> CA>
    static void joinCaps(OI bi, CA&& arg) {
        fmt::format_to(bi, "{}", std::get<1>(std::forward<CA>(arg)));
    }

    template <typename OI,
              std::same_as<CapArg> CA,
              std::same_as<CapArg> ... CAs>
    static void joinCaps(OI bi, CA&& arg1, CA&& arg2, CAs&& ... args) {
        fmt::format_to(bi, "{},", std::get<1>(std::forward<CA>(arg1)));
        joinCaps(bi, std::forward<CA>(arg2), std::forward<CAs>(args)...);
    }

public:
    CapState(CapState const&) = delete;

    CapState(CapState&& rhs) noexcept: capRaised_{rhs.capRaised_} {
        rhs.capRaised_ = false;
    }

    CapState& operator= (CapState const&) = delete;

    CapState& operator= (CapState&& rhs) noexcept {
        if (this == &rhs)
            return *this;

        capRaised_ = rhs.capRaised_;
        rhs.capRaised_ = false;
        return *this;
    }

    ~CapState() {
        if (capRaised_)
            dropAllCaps();
    }


private:
    constexpr explicit CapState(): capRaised_{true} {}

    static inline Result<void, CapFailure> raiseCap(
            cap_value_t capValue, char const* capName) {
        cap_t caps;
        cap_value_t capv[1];

        caps = cap_get_proc();
        if (caps == nullptr)
            return error("cap_get_proc() failed: {}", SysError{});

        capv[0] = capValue;
        if (cap_set_flag(caps, CAP_EFFECTIVE, 1, capv, CAP_SET) == -1) {
            cap_free(caps);
            return error("cap_set_flag() failed: {}", SysError{});
        }

        if (cap_set_proc(caps) == -1) {
            auto ec = errno;
            cap_free(caps);
            dropAllCaps();
            if (ec == EPERM)
                return notSet(capName);

            return error("cap_set_proc() failed: {}", SysError{ec});
        }

        if (cap_free(caps) == -1)
            return error("cap_free() failed: {}", SysError{});

        return {};
    }

    inline static void dropAllCaps() {
        cap_t empty;

        empty = cap_init();
        if (empty == nullptr)
            return;

        if (cap_set_proc(empty) == -1) {
            cap_free(empty);
            return;
        }

        cap_free(empty);
    }

    template <typename ArgTuple, size_t ... Idxs>
    static Result<void, CapFailure> raise1(
            ArgTuple&& arg, std::index_sequence<Idxs...>) {
        return raiseCap(std::get<Idxs>(std::forward<ArgTuple>(arg))...);
    }

    inline static Result<CapState, CapFailure> raiseImpl() {
        return CapState{};
    }

    template <typename First, typename ... Rest>
    static Result<CapState, CapFailure> raiseImpl(First&& arg, Rest&& ... args) {
        auto r = raise1(
                std::forward<First>(arg),
                std::make_index_sequence<std::tuple_size_v<First>>{});
        if (not r)
            return fail(std::move(r).error());

        return raiseImpl(std::forward<Rest>(args)...);
    }

public:
    class Builder final {
        friend class CapState;
    public:
        template <std::same_as<CapArg> ... ArgTuples>
        Result<CapState, std::string> raise(ArgTuples&& ... args) {
            auto r = CapState::raiseImpl(std::forward<ArgTuples>(args)...);
            if (r)
                return std::move(r).value();

            auto e = std::move(r).error();
            if (e.capName != nullptr) {
                fmt::memory_buffer mb;
                auto bi = std::back_inserter(mb);
                fmt::format_to(
                        bi,
                        "unable to raise {} capability: "
                        "try running under sudo or grant {} the required capabilities "
                        "by running sudo setcap ",
                        e.capName, progName_);
                CapState::joinCaps(bi, std::forward<ArgTuples>(args)...);
                fmt::format_to(bi, "=p {}", progName_);
                return fail(fmt::to_string(mb));
            }

            return fail(std::move(e.msg));
        }
    private:
        constexpr explicit Builder(char const* progName): progName_{progName} {}
    private:
        char const* progName_;
    };

    friend class Builder;

    inline constexpr static Builder program(char const* progName) {
        return Builder{progName};
    }
private:
    bool capRaised_;
};

} // namespace pimc

#else

#define CAP_(v) std::tuple<int>{0}

namespace pimc {

class CapState final {
    using CapArg = std::tuple<int>;
public:
    CapState(CapState const&) = delete;
    CapState(CapState&&) noexcept = default;
    CapState& operator= (CapState const&) = delete;
    CapState& operator= (CapState&&) noexcept = default;

    ~CapState() = default;

    class Builder final {
        friend class CapState;
    public:
        template<std::same_as<CapArg> ... ArgTuples>
        Result<CapState, std::string> raise(ArgTuples &&...) {
            return CapState{};
        }
    private:
        Builder() = default;
    };

    friend class Builder;

    inline static Builder program(char const*) {
        return Builder{};
    }

private:
    CapState() = default;
};

}

#endif
