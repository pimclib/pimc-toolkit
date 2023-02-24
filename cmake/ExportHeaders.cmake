include("${PROJECT_SOURCE_DIR}/cmake/HasNameComponent.cmake")

function(export_headers TARGET HEADER_SOURCE_DIR HEADER_INCL_PREFIX)
    # Add more C/C++ header extensions here
    SET(C_CXX_HEADER_EXTENSIONS .h .hpp .H .hxx)

    get_target_property(LIBRARY_TYPE ${TARGET} TYPE)
    if ((${LIBRARY_TYPE} STREQUAL STATIC_LIBRARY) OR
        (${LIBRARY_TYPE} STREQUAL SHARED_LIBRARY))
        set(EXPORT_ACCESS PUBLIC)
    elseif (${LIBRARY_TYPE} STREQUAL INTERFACE_LIBRARY)
        set(EXPORT_ACCESS INTERFACE)
    else()
        MESSAGE(
            FATAL_ERROR
            "Unsupported target type ${LIBRARY_TYPE} of target ${TARGET}")
    endif()

    if (IS_ABSOLUTE ${HEADER_INCL_PREFIX})
        MESSAGE(
            FATAL_ERROR
            "HEADER_INCL_PREFIX must not be absolute, got '${HEADER_INCL_PREFIX}'")
    endif()

    SET(HEADER_GLOB_PATTERNS)
    foreach (EXT IN LISTS C_CXX_HEADER_EXTENSIONS)
        list(APPEND HEADER_GLOB_PATTERNS "${HEADER_SOURCE_DIR}/*${EXT}")
    endforeach()

    file(
        GLOB_RECURSE EXPORT_HEADERS
        LIST_DIRECTORIES true
        CONFIGURE_DEPENDS
        RELATIVE "${HEADER_SOURCE_DIR}"
        ${HEADER_GLOB_PATTERNS})

    SET(LN_SUBDIRS)
    SET(LN_HEADERS)
    foreach (HFD IN LISTS EXPORT_HEADERS)
        set(IN_TESTS)
        has_name_component("${HFD}" tests IN_TESTS)
        if (NOT ${IN_TESTS})
            if (IS_DIRECTORY "${HEADER_SOURCE_DIR}/${HFD}")
                list(APPEND LN_SUBDIRS "${HFD}")
            else()
                list(APPEND LN_HEADERS "${HFD}")
            endif()
        endif()
    endforeach()

    set(HEADER_EXPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
    file(REMOVE_RECURSE "${HEADER_EXPORT_DIR}")
    set(PREFIXED_HEADER_INCLUDE_DIR "${HEADER_EXPORT_DIR}/${HEADER_INCL_PREFIX}")
    file(MAKE_DIRECTORY "${PREFIXED_HEADER_INCLUDE_DIR}")
    foreach (LN_SUBDIR IN LISTS LN_SUBDIRS)
        file(MAKE_DIRECTORY "${PREFIXED_HEADER_INCLUDE_DIR}/${LN_SUBDIR}")
    endforeach()

    foreach (LN_HEADER IN LISTS LN_HEADERS)
        file(
            CREATE_LINK "${HEADER_SOURCE_DIR}/${LN_HEADER}" "${PREFIXED_HEADER_INCLUDE_DIR}/${LN_HEADER}"
            SYMBOLIC)
    endforeach()

    target_include_directories(${TARGET} ${EXPORT_ACCESS} ${HEADER_EXPORT_DIR})
endfunction()
