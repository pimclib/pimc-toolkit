set(PIMSMConfig
        config/PIMSMConfig.hpp
        config/PIMSMConfigLoader.hpp
)

set(JPConfig
        config/JPConfig.hpp
        config/PIMSMParams.hpp
        config/MulticastConfigLoader.hpp
        config/JPConfigLoader.hpp
)

set(PIMUpdate
        pimsm/Update.hpp
        pimsm/Pack.hpp
        pimsm/PackSanityCheck.hpp
        pimsm/UpdateFormatter.hpp
)

add_executable(
        pimc
        Main.cpp
        config/ConfigUtils.hpp
        config/Config.hpp
        config/Config.cpp
        config/PIMCConfig.hpp
        config/PIMCConfigLoader.hpp
        ${PIMSMConfig}
        ${JPConfig}
        config/Formatters.hpp
        ${PIMUpdate}
        net/IPv4RawSocket.hpp
        net/IPv4RawSocket.cpp
        net/BindToDevice.hpp
        net/BindToDevice.cpp
        net/BindToDeviceLinux.icc
        net/BindToDeviceMacos.icc
        net/IPv4PIMIntf.hpp
        net/IPv4PIMIntf.cpp
)

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
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/PackingVerifierConfigs.incl
        COMMAND ${CMAKE_COMMAND}
          -DSOURCE_YAML=${CMAKE_CURRENT_LIST_DIR}/tests/packing-tests.yml
          -DINCLUDE_FILE=${CMAKE_CURRENT_BINARY_DIR}/PackingVerifierConfigs.incl
          -P ${CMAKE_CURRENT_LIST_DIR}/tests/packing-tests.cmake
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/tests/packing-tests.yml
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(
        pimc-tests-generated-includes
        ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/PackingVerifierConfigs.incl
)

add_executable(
        pimc-tests
        ${JPConfig}
        ${PIMUpdate}
        tests/PackingVerifierConfig.hpp
        tests/UpdateLoader.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/PackingVerifierConfigs.incl
        tests/PackingTests.cpp
)

target_include_directories(
        pimc-tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
        pimc-tests
        PRIVATE
            -static-libgcc
            -static-libstdc++
            ProjectSettings
            VersionLib
            PimcLib
            fmt
            gtest_main
)

add_dependencies(pimc-tests pimc-tests-generated-includes)