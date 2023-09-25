add_custom_target(
        ProjectVersionHeader
        ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/version_internal.hpp
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version_internal.hpp
        COMMAND ${CMAKE_COMMAND}
                -DGIT=${GIT}
                -DINPUT_TEMPLATE_FILE=${CMAKE_CURRENT_LIST_DIR}/version_internal.hpp.in
                -DOUTPUT_HEADER_FILE=${CMAKE_CURRENT_BINARY_DIR}/version_internal.hpp
                -P ${CMAKE_SOURCE_DIR}/cmake/update_version_header.cmake
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/version_internal.hpp.in
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )

add_dependencies(VersionLib ProjectVersionHeader)

target_sources(
        VersionLib
        PRIVATE
            version.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/version_internal.hpp
        PUBLIC
            version.hpp
)

target_include_directories(
        VersionLib
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
        VersionLib
        PUBLIC
        ProjectSettings
        fmt::fmt
)
