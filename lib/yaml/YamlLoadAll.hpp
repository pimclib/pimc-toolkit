#pragma once

#include <vector>
#include <string>

#include <yaml-cpp/yaml.h>

#include "pimc/core/Result.hpp"

namespace pimc {

/*!
 * \brief Loads all documents from the YAML file named \p ymlfn.
 *
 * @param ymlfn the name of the YAML file
 * @return a Result which contains a vector of root YAML nodes for each
 * of the document or an error message if there is a problem with the
 * file or if parsing fails
 */
auto yamlLoadAll(char const* ymlfn) -> Result<std::vector<YAML::Node>, std::string>;

} // namespace pimc