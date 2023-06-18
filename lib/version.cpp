#include <cstring>
#include <string>

#include <fmt/format.h>

#include "pimc/version.hpp"
#include "pimc/text/MemoryBuffer.hpp"

#include "version_internal.hpp"

namespace pimc {

std::string version() {
    auto& mb = getMemoryBuffer();
    auto bi = std::back_inserter(mb);

    fmt::format_to(bi, "pimclib v{}\n", taggedVersion);
    fmt::format_to(bi, "git commit ID {}\n", gitCommitId);
    if (strcmp(gitBranch, "main") != 0)
        fmt::format_to(bi, "git branch {}\n", gitBranch);
    return fmt::to_string(mb);
}

} // namespace pimc
