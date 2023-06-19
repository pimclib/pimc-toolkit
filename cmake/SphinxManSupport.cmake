# This function takes the following arguments:
#  - TargetName: the name of the API documentation target, e.g. man-pages
#  - The subdirectory where the man page directories reside, e.g. man-source
#  - One or more app names for which the man pages should be generated
#
#  Each app name must have a corresponding RST file inside the ${SourceSubdir}
#  directory whose name is the name of the app followed by the .rst extension.
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
    set(SPHINX_MAN_SOURCES "${SPHINX_MAN_SOURCE_DIR}/index.rst")

    set(SPHINX_MAN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TargetName}")
    unset(SPHINX_MAN_PAGES)

    foreach(AppName IN LISTS ARGN)
        list(APPEND SPHINX_MAN_SOURCES "${SPHINX_MAN_SOURCE_DIR}/${AppName}.rst")
        list(APPEND SPHINX_MAN_PAGES "${SPHINX_MAN_BUILD_DIR}/${AppName}.1")
    endforeach()

    add_custom_command(
            OUTPUT ${SPHINX_MAN_PAGES}
            COMMAND ${SPHINX_EXECUTABLE} -b man  ${SPHINX_MAN_SOURCE_DIR} ${SPHINX_MAN_BUILD_DIR}
            MAIN_DEPENDENCY "${SPHINX_MAN_SOURCE_DIR}/conf.py"
            DEPENDS ${SPHINX_MAN_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(${TargetName} ALL DEPENDS ${SPHINX_MAN_PAGES})
endfunction()
