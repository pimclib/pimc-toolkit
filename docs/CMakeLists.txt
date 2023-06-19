include(${PROJECT_SOURCE_DIR}/cmake/SphinxManSupport.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/DoxygenSphinxSupport.cmake)

SphinxManPages(
        man-pages
        man-source
        mclst
)

SphinxDoxyLib(
        api-docs
        PimcLib
        api-source
        ${PROJECT_SOURCE_DIR}/docs/api-source/index.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/time.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/formatters.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/system.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/net.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/dissect.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/unix.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/core.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/core-api/Deferred.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/core-api/Result.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/core-api/TypeUtils.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/core-api/CompilerUtils.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/text.rst
        ${PROJECT_SOURCE_DIR}/docs/api-source/api-sources/parsers.rst
)
