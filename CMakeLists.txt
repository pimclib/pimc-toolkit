cmake_minimum_required(VERSION 3.20)
project(PIMC-Toolkit)

option(BuildApiDocs "Build API Docs using doxygen and sphinx")

if (APPLE)
    set(DOXY_OS_DEF "__APPLE__")
elseif(LINUX)
    set(DOXY_OS_DEF "__linux__")
else()
    message(FATAL_ERROR "Unable to build for ${CMAKE_SYSTEM_NAME}")
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_program(GIT NAMES git)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(cmake/ProjectSettings.cmake)
include(cmake/ExportHeaders.cmake)

add_library(ProjectSettings INTERFACE)
set_project_warnings(ProjectSettings)

add_subdirectory(external)

add_library(VersionLib STATIC)
add_subdirectory(version)

add_library(PimcLib STATIC)

# This needs to be declared before 'add_subdirectory(lib)' because
# the library will need to be able to add its tests to Tests executable
add_executable(Tests)
add_subdirectory(lib)

target_link_libraries(
        Tests
        PRIVATE
            ProjectSettings
            PimcLib
            gtest_main
)

add_subdirectory(apps)

# Documentation
add_subdirectory(docs)
