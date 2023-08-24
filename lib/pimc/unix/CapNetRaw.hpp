#pragma once

#ifdef WITH_LIBCAP
#include <unistd.h>
#include <sys/capability.h>
#endif

#include <string>

#include "pimc/core/Result.hpp"
#include "pimc/formatters/Fmt.hpp"

namespace pimc {

class CapNetRaw final {
public:
    CapNetRaw(CapNetRaw const&) = delete;

    CapNetRaw(CapNetRaw&& rhs) noexcept: capRaised_{rhs.capRaised_} {
        rhs.capRaised_ = false;
    }

    CapNetRaw& operator= (CapNetRaw const&) = delete;

    CapNetRaw& operator= (CapNetRaw&& rhs) noexcept {
        if (this == &rhs)
            return *this;

        capRaised_ = rhs.capRaised_;
        rhs.capRaised_ = false;
        return *this;
    }

    ~CapNetRaw() {
        if (capRaised_)
            dropAllCaps();
    }

#ifdef WITH_LIBCAP
    static inline Result<CapNetRaw, std::string> raise(char const* progname) {
        cap_t caps;
        cap_value_t capv[1];

        caps = cap_get_proc();
        if (caps == nullptr)
            return fail(fmt::format("cap_get_proc() failed: {}", SysError{}));

        capv[0] = CAP_NET_RAW;
        if (cap_set_flag(caps, CAP_EFFECTIVE, 1, capv, CAP_SET) == -1) {
            cap_free(caps);
            return fail(fmt::format("cap_set_flag() failed: {}", SysError{}));
        }

        if (cap_set_proc(caps) == -1) {
            cap_free(caps);
            auto ec = errno;
            if (ec == EPERM)
                return fail(fmt::format(
                        "unable to raise CAP_NET_RAW capability: "
                        "try running under sudo or "
                        "grant {} the CAP_NET_RAW capability by running "
                        "setcap cap_net_raw=p {}", progname, progname));

            return fail(fmt::format("cap_set_proc() failed: {}", SysError{ec}));
        }

        if (cap_free(caps) == -1)
            return fail(fmt::format("cap_free() failed: {}", SysError{}));

        return CapNetRaw{true};
    }
#else
    static inline Result<CapNetRaw, std::string> raise(char const*) {
        return CapNetRaw{false};
    }
#endif

private:
    constexpr explicit CapNetRaw(bool capRaised): capRaised_{capRaised} {}

#ifdef WITH_LIBCAP
    void dropAllCaps() {
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
#else
    void dropAllCaps() {}
#endif

private:
    bool capRaised_;
};

} // namespace pimc
