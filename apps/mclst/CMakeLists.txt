add_executable(
        mclst
        Main.cpp
        Config.hpp
        Config.cpp
        MclstBeacon.hpp
        PacketInfo.hpp
        OutputHandler.hpp
        MclstBase.hpp
        ReceiverBase.hpp
        Receiver.hpp
        IPRawReceiver.hpp
        Sender.hpp
        Sender.cpp
        RxStats.hpp
        Timer.hpp
)


find_file(CAPABILITY_H sys/capability.h)
find_library(LIBCAP cap)

if ((NOT CAPABILITY_H) OR (NOT LIBCAP))
    message(
            STATUS
            "mclst: sys/capability.h is not found, consider installing libcap-devel"
    )
    set(MCLST_LIBS)
else()
    target_compile_definitions(
            mclst
            PRIVATE
            WITH_LIBCAP)
    set(MCLST_LIBS ${LIBCAP})
endif()

target_link_libraries(
        mclst
        PRIVATE
            -static-libgcc
            -static-libstdc++
            ProjectSettings
            VersionLib
            PimcLib
            fmt
            ${MCLST_LIBS}
)
