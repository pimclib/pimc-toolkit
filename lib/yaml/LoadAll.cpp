#include <filesystem>

#include <fmt/format.h>

#include "pimc/core/Deferred.hpp"
#include "pimc/system/SysError.hpp"
#include "pimc/formatters/SysErrorFormatter.hpp"
#include "LoadAll.hpp"

namespace fs = std::filesystem;

namespace pimc::yaml {

auto loadAll(char const* ymlfn) -> Result<std::vector<YAML::Node>, std::string> {
    fs::path yml{ymlfn};

    if (not fs::exists(yml))
        return fail(fmt::format("file '{}' does not exist\n", ymlfn));

    if (fs::is_directory(yml))
        return fail(fmt::format("'{}' is a directory", yml.native()));

    if (not fs::is_regular_file(yml))
        return fail(fmt::format("'{}' is not a regular file", yml.native()));

    FILE* fp = fopen(yml.c_str(), "r");
    if (fp == nullptr)
        return fail(fmt::format(
                "unable to open file '{}': {}", yml.native(), pimc::SysError{}));

    auto fpc = defer([fp] { fclose(fp); });

    fmt::memory_buffer buf;
    int c;
    while ((c = fgetc(fp)) != EOF)
        buf.push_back(static_cast<char>(c));

    if (ferror(fp))
        return fail(fmt::format(
                "I/O error while reading file '{}': {}",
                yml.native(), pimc::SysError{}));

    buf.push_back(static_cast<char>(0));

    try {
        return YAML::LoadAll(buf.data());
    } catch (YAML::ParserException const& expt) {
        return fail(fmt::format(
                "{}, {}: {}",
                yml.native(),
                expt.mark.line + 1,
                expt.msg));
    }
}

} // namespace pimc
