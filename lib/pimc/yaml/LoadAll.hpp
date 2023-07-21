#pragma once

#include <vector>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <yaml-cpp/yaml.h>
#pragma GCC diagnostic pop

#include "pimc/core/Result.hpp"

namespace pimc::yaml {

/*!
 * \brief Loads all documents from the YAML file named \p ymlfn.
 *
 * @param ymlfn the name of the YAML file
 * @return a Result which contains a vector of root YAML nodes for each
 * of the document or an error message if there is a problem with the
 * file or if parsing fails
 */
auto loadAll(char const* ymlfn) -> Result<std::vector<YAML::Node>, std::string>;

/*!
 * \copydoc loadAll(char const*)
 */
inline auto loadAll(std::string const& yamlfn)
-> Result<std::vector<YAML::Node>, std::string> {
    return loadAll(yamlfn.c_str());
}

} // namespace pimc