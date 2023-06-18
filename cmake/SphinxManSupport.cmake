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
    list(LENGTH ARGN ArgnLen)
    if (${ArgnLen} EQUAL 0)
        message(FATAL_ERROR "No man sources given")
    endif()

    set(SPHINX_MAN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SourceSubdir}")
    set(SPHINX_MAN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TargetName}")
    unset(SPHINX_MAN_PAGES)

    foreach(ManSubdir IN LISTS ARGN)
        set(SPHINX_MAN_SOURCE "${SPHINX_MAN_SOURCE_DIR}/${ManSubdir}")
        set(SPHINX_MAN_PAGE "${SPHINX_MAN_BUILD_DIR}/${ManSubdir}.1")
        add_custom_command(
                OUTPUT ${SPHINX_MAN_PAGE}
                COMMAND ${SPHINX_EXECUTABLE} -b man
                ${SPHINX_MAN_SOURCE} ${SPHINX_MAN_BUILD_DIR}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                MAIN_DEPENDENCY "${SPHINX_MAN_SOURCE}/index.rst"
                COMMENT "Generating man page for ${ManSubdir}"
        )
        list(APPEND SPHINX_MAN_PAGES ${SPHINX_MAN_PAGE})
    endforeach()

    add_custom_target(${TargetName} ALL DEPENDS ${SPHINX_MAN_PAGES})
endfunction()
