#pragma once

#include <csignal>
#include <cstring>

#include "pimc/system/Exceptions.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"

namespace pimc {

/*!
 * \brief The requirements for the exit handler.
 *
 * The handler must provide a static function `void::failedSignal(int signal)`
 * which is expected to terminate the program.
 *
 * @tparam H the exit handler
 */
template <typename H>
concept ExitHandler = requires(int s) {
    { H::failedSignal(s) };
};

/*!
 * The default signal error handler which throws an `std::runtime_error`.
 */
struct RuntimeErrorHandler {
    RuntimeErrorHandler() = delete;

    static inline void failedSignal(int signal) {
        raise<std::runtime_error>(
                "unable to install handler for signal {}: {}",
                signal, SysError{});
    }
};

/*!
 * A utility class which facilitates installing the same handler for one
 * or multiple UNIX signals.
 *
 * @tparam ExitPolicy The class that provides a static method
 * `void failedSignal(int signal)` which is invoked if the handler fails
 * to be installed for a specific signal, which is passed as the
 * only argument to this static method.
 */
template <ExitHandler EH = RuntimeErrorHandler>
class SignalHandler final {
public:
    SignalHandler() = delete;

    /*!
     * Installs the specified signal handler `h` for the signals
     * specified as the template arguments to this function.
     * @tparam Signals the signals for which this signal hander
     * is installed
     * @param h the signal handler
     * @return `true` if the signal handler is successfully installed
     * for each of the signals, `false` otherwise
     */
    template <int ... Signals>
    static bool install(void (*h)(int)) {
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = h;
        return installImpl<Signals...>(&sa);
    }
private:
    template <int Signal>
    static bool installImpl(struct sigaction* sa) {
        if (sigaction(Signal, sa, nullptr) == -1) {
            EH::failedSignal(Signal);
            return false;
        }
        return true;
    }

    template <int First, int Second, int ... Rest>
    static bool installImpl(struct sigaction* sa) {
        if (sigaction(First, sa, nullptr) == -1) {
            EH::failedSignal(First);
            return false;
        }
        return installImpl<Second, Rest...>(sa);
    }
};

} // namespace pimc
