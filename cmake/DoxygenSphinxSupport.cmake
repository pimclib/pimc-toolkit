find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# The argument names are suffixed with _VN (for variable name). Unless this is done
# there is some sort of a name clash between the argument name and the variable with
# the same name in the PARENT_SCOPE
function(DoxyLib LIB_NAME DOXY_LIB_DEPS_VN BREATHE_PROJECT_XML_DEFS_VN DOXYGEN_INDEX_LIST_VN)
    MESSAGE("LIB ${LIB_NAME}")

    get_target_property(LIB_PUBLIC_HEADER_DIR ${LIB_NAME} INTERFACE_INCLUDE_DIRECTORIES)
    file(GLOB_RECURSE LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADER_DIR})
    get_target_property(LIB_SOURCE_DIR ${LIB_NAME} EFFECTIVE_SOURCES_DIR)

    set(DOXYGEN_LIB_INPUT_DIR ${LIB_SOURCE_DIR})
    set(DOXYGEN_LIB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen/${LIB_NAME})
    set(DOXYGEN_LIB_INDEX_FILE ${DOXYGEN_LIB_OUTPUT_DIR}/xml/index.xml)
    set(DOXYFILE_LIB_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-branch)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-${LIB_NAME})

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    # Doxygen won't create the output directory
    file(MAKE_DIRECTORY ${DOXYGEN_LIB_OUTPUT_DIR})
    add_custom_command(
            OUTPUT ${DOXYGEN_LIB_INDEX_FILE}
            DEPENDS ${LIB_PUBLIC_HEADERS}
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
            MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
            COMMENT "Generating docs for library ${LIB_NAME}"
    )

    list(APPEND ${DOXY_LIB_DEPS_VN} ${DOXYGEN_LIB_INDEX_FILE})
    set(${DOXY_LIB_DEPS_VN} ${${DOXY_LIB_DEPS_VN}} PARENT_SCOPE)
    list(APPEND ${BREATHE_PROJECT_XML_DEFS_VN} "-Dbreathe_projects.${LIB_NAME}=${DOXYGEN_LIB_OUTPUT_DIR}/xml")
    set(${BREATHE_PROJECT_XML_DEFS_VN} ${${BREATHE_PROJECT_XML_DEFS_VN}} PARENT_SCOPE)
    list(APPEND ${DOXYGEN_INDEX_LIST_VN} ${DOXYGEN_LIB_INDEX_FILE})
    set(${DOXYGEN_INDEX_LIST_VN} ${${DOXYGEN_INDEX_LIST_VN}} PARENT_SCOPE)
endfunction()

# This functions takes the name of a C/C++ library target and creates two targets,
# LibName-Doxygen and LibNameSphinx, where "LibName" is the name of the specified
# library. The optional additional parameters are the names of the .rst files,
# which provide the contents for the library help pages.
function(SphinxDoxyLibs TargetName LibName)
    set(DOXY_LIB_DEPS)
    set(BREATHE_PROJECT_XML_DEFS)
    set(DOXYGEN_INDEX_LIST)
    DoxyLib(${LibName} DOXY_LIB_DEPS BREATHE_PROJECT_XML_DEFS DOXYGEN_INDEX_LIST)

    MESSAGE("DOXY_LIB_DEPS: ${DOXY_LIB_DEPS}")
    MESSAGE("BREATHE_PROJECT_XML_DEFS: ${BREATHE_PROJECT_XML_DEFS}")
    MESSAGE("DOXYGEN_INDEX_LIST: ${DOXYGEN_INDEX_LIST}")

    add_custom_target(${LibName}-Doxygen ALL DEPENDS ${DOXY_LIB_DEPS})

    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
    set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)
    set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

    add_custom_command(
            OUTPUT ${SPHINX_INDEX_FILE}
            COMMAND ${SPHINX_EXECUTABLE} -b html
            ${BREATHE_PROJECT_XML_DEFS} ${SPHINX_SOURCE} ${SPHINX_BUILD}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
            ${ARGN}
            ${DOXYGEN_INDEX_LIST}
            MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
            COMMENT "Generating documentation with Sphinx"
    )

    add_custom_target(${TargetName} ALL DEPENDS ${SPHINX_INDEX_FILE})
endfunction()

