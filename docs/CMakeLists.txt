include(${PROJECT_SOURCE_DIR}/cmake/SphinxManSupport.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/DoxygenSphinxSupport.cmake)

SphinxManPages(
        man-pages
        man-source
        mclst
)

if (${BuildApiDocs})
    SphinxDoxyLib(
            api-docs
            PimcLib
            api-source
            ${PROJECT_SOURCE_DIR}/docs/api-source/index.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/time.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/formatters.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/system.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/net.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/net-api/addressing.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/net-api/interfaces.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/packets.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/packets-api/adapters.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/packets-api/packet-view.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/packets-api/pimsmv2.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/unix.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/core.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/core-api/Deferred.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/core-api/Optional.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/core-api/Result.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/core-api/TypeUtils.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/core-api/CompilerUtils.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/text.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/parsers.rst
            ${PROJECT_SOURCE_DIR}/docs/api-source/sections/yaml.rst
    )
endif()
