set(JPConfig
        config/JPConfigLoader.hpp
        config/JPConfig.hpp
        config/JPConfigLoader.cpp
)

add_executable(
        pimc
        Main.cpp
        ${JPConfig}
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
