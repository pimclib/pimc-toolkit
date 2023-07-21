#include <climits>
#include <map>
#include <memory>
#include <set>

#include <getopt.h>

#include "pimc/formatters/Fmt.hpp"

#include "pimc/text/IndentBlock.hpp"

#include "GetOptLong.hpp"

using namespace std::string_literals;

namespace pimc {

namespace {

template <typename ... Ts>
void fatal(fmt::format_string<Ts...> const& f, Ts&& ... args) {
    fmt::memory_buffer buf;
    auto bi = std::back_inserter(buf);
    fmt::format_to(bi, f, std::forward<Ts>(args)...);
    throw GetOptLongError(fmt::to_string(buf));
}

template <typename ... Ts>
void error(fmt::format_string<Ts...> const& f, Ts&& ... args) {
    fmt::memory_buffer buf;
    auto bi = std::back_inserter(buf);
    fmt::format_to(bi, f, std::forward<Ts>(args)...);
    throw CommandLineError(fmt::to_string(buf));
}


struct OptionInfo final {
    // If no short version is available this should be 0
    int shortOpt;
    // If no long version is available this string should be empty
    std::string longOpt;
    // flag disables metavar and multiple
    bool flag;
    std::string metavar;
    bool optional;
    bool multiple;
    std::string help;

    // These are values, they are set during getopt_long execution
    bool flagValue;
    std::vector<std::string> values;

    void setValue(char const* value) {
        if (flag) {
            flagValue = true;
            return;
        }

        if (value == nullptr)
            fatal("option -{}|--{} requires a value, but none is provided",
                  static_cast<char>(shortOpt), longOpt);

        if (not values.empty() and not multiple)
            error("duplicate -{}|--{} option",
                  static_cast<char>(shortOpt), longOpt);

        values.emplace_back(value);
    }

    template <std::output_iterator<char> O>
    void helpHeading(O o) const {
        bool pipe{false};
        if (shortOpt != 0) {
            o = fmt::format_to(o, "-{}", static_cast<char>(shortOpt));
            pipe = true;
        }

        if (not longOpt.empty()) {
            if (pipe) o = fmt::format_to(o, "|");
            o = fmt::format_to(o, "--{}", longOpt);
        }

        if (not flag)
            fmt::format_to(o, " <{}>", metavar);
    }

    [[nodiscard]]
    bool isHelp() const { return flag and multiple; }

    void checkRequired() {
        if (not optional and values.empty()) {
            if (shortOpt != 0 and not longOpt.empty())
                error("option -{}|--{} is required",
                      static_cast<char>(shortOpt), longOpt);
            if (shortOpt != 0)
                error("option -{} is required", static_cast<char>(shortOpt));

            error("option --{} is required", longOpt);
        }
    }

    void requiresArg() {
        if (shortOpt != 0 and not longOpt.empty())
            error("option -{}|--{} requires an argument",
                  static_cast<char>(shortOpt), longOpt);
        if (shortOpt != 0)
            error("option -{} requires an argument", static_cast<char>(shortOpt));

        error("option --{} requires an argument", longOpt);
    }

    OptionInfo(
            char so,
            std::string lo,
            bool fl,
            std::string mv,
            bool opt,
            bool mlt,
            std::string hlp)
            : shortOpt{static_cast<int>(so)}
            , longOpt{std::move(lo)}
            , flag{fl}
            , metavar{std::move(mv)}
            , optional{opt}
            , multiple{mlt}
            , help{std::move(hlp)}
            , flagValue{false} {}
};


void checkShortOpt(char c) {
    if (c == 0) return;
    if (c >= 'a' and c <= 'z') return;
    if (c >= 'A' and c <= 'Z') return;
    if (c >= '0' and c <= '9') return;
    fatal("invalid short option '{}'", c);
}

void checkLongOpt(std::string const& s) {
    if (s.empty()) return;
    if (s.size() < 2)
        fatal("invalid long option '{}', must be at least 2 characters long", s);

    char c = s[0];
    if (not ((c >= 'a' and c <= 'z') or
             (c >= 'A' and c <= 'Z') or
             (c >= '0' and c <= '9')))
        fatal("invalid long option '{}'", s);

    for (size_t i = 1; i < s.size(); ++i) {
        c = s[i];
        if (not ((c >= 'a' and c <= 'z') or
                 (c >= 'A' and c <= 'Z') or
                 (c >= '0' and c <= '9') or
                 (c == '-' or c == '_')))
            fatal("invalid long option '{}'", s);
    }
}

void checkOpts(char c, std::string const& s) {
    checkShortOpt(c);
    checkLongOpt(s);
    if (c == 0 and s.empty())
        fatal("at least a short or long option must be defined");
}

void checkMetavar(std::string const& s) {
    if (s.empty())
        fatal("metavar may not be empty");
    if (s.size() < 2)
        fatal("invalid metavar '{}', must be at least 2 characters long", s);

    char c = s[0];
    if (not ((c >= 'a' and c <= 'z') or
             (c >= 'A' and c <= 'Z') or
             (c >= '0' and c <= '9')))
        fatal("invalid metavar '{}'", s);

    for (size_t i = 1; i < s.size(); ++i) {
        c = s[i];
        if (not ((c >= 'a' and c <= 'z') or
                 (c >= 'A' and c <= 'Z') or
                 (c >= '0' and c <= '9') or
                 (c == '-' or c == '_' or c == '=')))
            fatal("invalid metavar '{}'", s);
    }
}

} // anon.namespace


struct GetOptLong::Impl {
    std::vector<std::unique_ptr<OptionInfo>> ois;
    std::map<uint32_t , OptionInfo*> oiMap;
    std::vector<char> shortOpts;
    std::vector<::option> longOpts;
    std::map<int, OptionInfo*> optMap;
    std::set<std::string> loSet;
    std::string prog;
    std::string helpLegend;
    char const* progName;

    explicit Impl(std::string p, std::string hl)
    : prog{std::move(p)}, helpLegend{std::move(hl)}, progName{nullptr} {}

    void addOption(
            uint32_t id,
            char shortOpt,
            std::string longOpt,
            bool flag,
            std::string metavar,
            bool optional,
            bool multiple,
            std::string help) {
        ois.emplace_back(
                std::make_unique<OptionInfo>(
                    static_cast<int>(shortOpt),
                    std::move(longOpt),
                    flag,
                    std::move(metavar),
                    optional,
                    multiple,
                    std::move(help)
                ));
        auto& oi = ois.back();

        auto oii = oiMap.emplace(id, oi.get());
        if (not oii.second)
            fatal("duplicate option id {}", id);

        if (shortOpt != 0) {
            auto ii = optMap.emplace(shortOpt, oi.get());
            if (not ii.second)
                fatal("duplicate short option '{}'", shortOpt);
        }

        if (not oi->longOpt.empty()) {
            auto loi = loSet.emplace(oi->longOpt);
            if (not loi.second)
                fatal("duplicate long option '{}'", oi->longOpt);
        }

        if (shortOpt != 0) {
            if (shortOpts.empty())
                shortOpts.push_back(':');
            shortOpts.push_back(shortOpt);
            if (not flag)
                shortOpts.push_back(':');
        }

        if (not oi->longOpt.empty()) {
            // Assuming i is the index of the OptionInfo in ois, we
            // use the value -(i+1) as the option ID. This value is
            // always negative, which guarantees that we'll never clash
            // with any short option values (which we guarantee to be
            // a subset of ASCII chars with values < 128; to be sure
            // a char, being signed, never gets promoted to a negative
            // int)
            auto optId = -static_cast<int>(ois.size());
            int hasArg = flag ? no_argument : required_argument;
            longOpts.emplace_back(oi->longOpt.data(), hasArg, nullptr, optId);

            auto jj = optMap.emplace(optId, oi.get());
            if (not jj.second)
                // This should never happen
                fatal("internal error, duplicate optId {}", optId);
        }
    }

    void fini() {
        shortOpts.push_back(static_cast<char>(0));
        longOpts.emplace_back(nullptr, 0, nullptr, 0);
    }

    void showHelp() {
        unsigned line1indent;
        unsigned line1maxWidth;
        constexpr unsigned indent{25};
        constexpr unsigned maxWidth{90};
        fmt::memory_buffer buf;
        fmt::memory_buffer optline;
        auto bi = std::back_inserter(buf);
        fmt::format_to(
                bi, "Usage: {} {}\n\nOptions:\n\n",
                progName, helpLegend);

        for (auto const& oi: ois) {
            optline.clear();
            auto olbi = std::back_inserter(optline);
            oi->helpHeading(olbi);
            auto olsz = optline.size();
            std::copy(optline.data(), optline.data()+olsz, bi);
            if (olsz + 1 < indent) {
                line1indent = static_cast<unsigned>(indent - olsz);
                line1maxWidth = static_cast<unsigned>(maxWidth - olsz);
            } else {
                std::copy(stdstr::uxnl.cbegin(), stdstr::uxnl.cend(), bi);
                line1indent = indent;
                line1maxWidth = maxWidth;
            }
            indentBlock(
                    oi->help, stdstr::ws, stdstr::esc, bi,
                    line1indent, line1maxWidth, indent, maxWidth, ' ', stdstr::uxnl);
            std::copy(stdstr::uxnl.cbegin(), stdstr::uxnl.cend(), bi);
        }
        buf.push_back(static_cast<char>(0));
        std::fputs(buf.data(), stdout);
        exit(EXIT_SUCCESS);
    }
};


struct GetOptLongResult::Impl {
    std::vector<std::unique_ptr<OptionInfo>> ois;
    std::map<unsigned, OptionInfo*> oiMap;
    std::vector<std::string> positional;

    [[nodiscard]]
    OptionInfo const* byId(unsigned id) const {
        auto oii = oiMap.find(id);
        if (oii == oiMap.cend())
            fatal("non-existent id {}", id);
        return oii->second;
    }
};


GetOptLong::GetOptLong(std::string prog, std::string helpLegend)
: impl_{new GetOptLong::Impl{std::move(prog), std::move(helpLegend)}} {
    impl_->shortOpts.push_back(':');
    // flag+multiple designates the help option
    impl_->addOption(
            UINT_MAX, 'h', "help"s, true, ""s, true, Multiple,
            "Show this message and exit"s);
}

GetOptLong::~GetOptLong() = default;

GetOptLong& GetOptLong::flag(
        uint32_t id, char shortOpt, std::string longOpt, std::string help) {
    checkOpts(shortOpt, longOpt);
    impl_->addOption(
            id, shortOpt, std::move(longOpt), true, ""s, true, false, std::move(help));
    return *this;
}

GetOptLong& GetOptLong::required(
        uint32_t id, char shortOpt, std::string longOpt, std::string metavar,
        std::string help, bool multiple) {
    checkOpts(shortOpt, longOpt);
    checkMetavar(metavar);
    impl_->addOption(
            id,
            shortOpt,
            std::move(longOpt),
            false,
            std::move(metavar),
            false,
            multiple,
            std::move(help));
    return *this;
}

GetOptLong& GetOptLong::optional(
        uint32_t id, char shortOpt, std::string longOpt, std::string metavar,
        std::string help, bool multiple) {
    checkOpts(shortOpt, longOpt);
    checkMetavar(metavar);
    impl_->addOption(
            id,
            shortOpt,
            std::move(longOpt),
            false,
            std::move(metavar),
            true,
            multiple,
            std::move(help));
    return *this;
}

GetOptLongResult GetOptLong::args(int argc, char * const* argv) {
    impl_->fini();
    int longOptIndex;
    impl_->progName = impl_->prog.empty() ? argv[0] : impl_->prog.c_str();

    opterr = 0;
    optind = 1;

    while (true) {
        int c = getopt_long(
                argc, argv,
                impl_->shortOpts.data(), impl_->longOpts.data(),
                &longOptIndex);

        if (c == -1) break;

        switch (c) {

        case ':':
            if (auto ii = impl_->optMap.find(optopt); ii != impl_->optMap.cend()) {
                auto& oi = ii->second;
                oi->requiresArg();
            } else {
                fatal("getopt_long(), optopt contains unknown code {}", optopt);
            }
            break;

        case '?':
            if (optopt != 0)
                error("unrecognized option -{}", static_cast<char>(optopt));
            else
                error("unrecognized option {}", argv[optind - 1]);
            break;

        default:
            if (auto ii = impl_->optMap.find(c); ii != impl_->optMap.cend()) {
                auto& oi = ii->second;
                if (oi->isHelp()) impl_->showHelp();
                ii->second->setValue(optarg);
            } else {
                fatal("getopt_long() returned unknown code {}", c);
            }
        }
    }

    for (auto& oi: impl_->ois) oi->checkRequired();

    std::vector<std::string> positional;
    if (optind < argc) {
        positional.reserve(static_cast<size_t>(argc - optind));
        for (auto i = optind; i < argc; ++i)
            positional.emplace_back(argv[i]);
    }

    return GetOptLongResult{
        std::make_unique<GetOptLongResult::Impl>(GetOptLongResult::Impl{
            .ois = std::move(impl_->ois),
            .oiMap = std::move(impl_->oiMap),
            .positional = std::move(positional)
        })
    };
}


GetOptLongResult::GetOptLongResult(std::unique_ptr<Impl> impl)
: impl_{std::move(impl)} {}

GetOptLongResult::~GetOptLongResult() = default;

bool GetOptLongResult::flag(unsigned id) const {
    auto const* oi = impl_->byId(id);

    if (not oi->flag)
        fatal("option with id {} is not a flag", id);

    return oi->flagValue;
}

std::vector<std::string> const& GetOptLongResult::values(unsigned id) const {
    auto const* oi = impl_->byId(id);

    if (oi->flag)
        fatal("option with id {} is a flag", id);

    return oi->values;
}

std::vector<std::string> const& GetOptLongResult::positional() const {
    return impl_->positional;
}

} // namespace pimc
