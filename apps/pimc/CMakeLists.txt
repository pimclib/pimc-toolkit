set(PIMSMConfig
        config/PIMSMConfig.hpp
        config/IPv4PIMSMConfigLoader.hpp
        config/IPv4PIMSMConfigLoader.cpp
)

set(JPConfig
        config/JPConfig.hpp
        config/IPv4JPConfigLoader.hpp
        config/IPv4JPConfigLoader.cpp
)

set(PIMUpdate
        pimsm/Update.hpp
        pimsm/UpdateBuilder.hpp
        pimsm/Pack.hpp
)

add_executable(
        pimc
        Main.cpp
        config/ConfigUtils.hpp
        config/Config.hpp
        config/Config.cpp
        config/PIMCConfig.hpp
        config/IPv4PIMCConfigLoader.hpp
        config/IPv4PIMCConfigLoader.cpp
        ${PIMSMConfig}
        ${JPConfig}
        config/Formatters.hpp
        ${PIMUpdate}
)

find_file(CAPABILITY_H sys/capability.h)
find_library(LIBCAP cap)

if ((NOT CAPABILITY_H) OR (NOT LIBCAP))
    message(
            STATUS
            "pimc: sys/capability.h is not found, consider installing libcap-devel"
    )
    set(MCLST_LIBS)
else()
    target_compile_definitions(
            pimc
            PRIVATE
            WITH_LIBCAP)
    set(PIMC_LIBS ${LIBCAP})
endif()

target_include_directories(
        pimc
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
        pimc
        PRIVATE
            -static-libgcc
            -static-libstdc++
            ProjectSettings
            VersionLib
            PimcLib
            fmt
            ${PIMC_LIBS}
)
