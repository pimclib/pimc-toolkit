include(${PROJECT_SOURCE_DIR}/cmake/DoxygenSphinxSupport.cmake)

SphinxDoxyLibs(
        PimcLib
        ${PROJECT_SOURCE_DIR}/docs/source/index.rst
        ${PROJECT_SOURCE_DIR}/docs/source/api.rst
        ${PROJECT_SOURCE_DIR}/docs/source/api-sources/unix.rst
        ${PROJECT_SOURCE_DIR}/docs/source/api-sources/core.rst
        ${PROJECT_SOURCE_DIR}/docs/source/api-sources/text.rst
)
