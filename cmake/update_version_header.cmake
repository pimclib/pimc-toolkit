if (GIT)
    execute_process(
        COMMAND ${GIT} rev-parse HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT} describe --tags
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_VER_TAG
		OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT} branch --show-current
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    configure_file(
			${CMAKE_SOURCE_DIR}/cmake/version.h.in
			${OUTPUT_HEADER_FILE}
    )
else()
    message(FATAL_ERROR "variable GIT containing the path to git executable is required")
endif()
