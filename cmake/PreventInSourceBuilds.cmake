#
# This function will prevent in-source builds
#
function(prevent_in_source_builds)
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

  # disallow in-source builds
  if("${srcdir}" STREQUAL "${bindir}")
    message(FATAL_ERROR "in-source builds are forbidden")
  endif()
endfunction()

prevent_in_source_builds()
