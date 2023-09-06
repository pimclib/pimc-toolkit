set(PIMSMConfig
        config/PIMSMConfig.hpp
        config/PIMSMConfigLoader.hpp
)

set(JPConfig
        pimsm/PIMSMParams.hpp
        config/JPConfig.hpp
        config/MulticastConfigLoader.hpp
        config/JPConfigLoader.hpp
)

set(PIMUpdate
        pimsm/Update.hpp
        pimsm/UBCursor.hpp
        pimsm/Pack.hpp
        pimsm/InversePack.hpp
        pimsm/SanityCheckUtils.hpp
        pimsm/PackSanityCheck.hpp
        pimsm/InversePackSanityCheck.hpp
        pimsm/UpdateFormatter.hpp
        pimsm/GenerationID.hpp
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
        packets/IPv4PIMHelloPacket.hpp
        packets/IPv4PIMHelloPacket.cpp
        packets/IPv4PIMUpdatePacket.hpp
        packets/IPv4PIMUpdatePacket.cpp
        scheduler/Timer.hpp
        scheduler/IPv4HelloEvent.hpp
        scheduler/IPv4JPUpdateEvent.hpp
        scheduler/IPv4GoodbyeEvent.hpp
        scheduler/IPv4GoodbyeJPUpdateEvent.hpp
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
        tests/InverseUpdateLoader.hpp
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