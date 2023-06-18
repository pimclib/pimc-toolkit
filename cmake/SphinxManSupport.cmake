# This function takes the following arguments:
#  - TargetName: the name of the API documentation target, e.g. man-pages
#  - The subdirectory where the man page directories reside, e.g. man-source
#  - One or more app names for which the man pages should be generated
#
#  Each app name must have a directory inside the ${SourceSubdir? directory
#  whose name is the name of the app. This directory must contain two files:
#    1. index.rst, which is the source of the man page
#    2. conf.py, which is required for sphinx-build to operate
#
# This function will create a target named ${TargetName} which will depend
# on all of the resulting man pages.
#
function(SphinxManPages TargetName SourceSubdir)
    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${SourceSubdir})
    set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/${TargetName}/sphinx)
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
