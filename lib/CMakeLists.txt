set_target_properties(
        PimcLib
        PROPERTIES EFFECTIVE_SOURCES_DIR ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(
    PimcLib
    PRIVATE
        pimc/net/IntfTable.cpp
        pimc/net/SocketUtils.cpp
        pimc/packets/IPChecksum.cpp
        pimc/unix/GetOptLong.cpp
        pimc/yaml/Structured.cpp
        pimc/yaml/LoadAll.cpp
    PUBLIC
        pimc/core/CompilerUtils.hpp
        pimc/core/Meta.hpp
        pimc/core/TypeUtils.hpp
        pimc/core/Deferred.hpp
        pimc/core/OptionalResultHelpers.hpp
        pimc/core/Optional.hpp
        pimc/core/Result.hpp
        pimc/core/TupleUtils.hpp
        pimc/core/Endian.hpp
        pimc/net/IP.hpp
        pimc/net/IPv4Address.hpp
        pimc/net/IPv4Prefix.hpp
        pimc/net/IPv4PktInfo.hpp
        pimc/net/IntfTable.hpp
        pimc/net/SocketUtils.hpp
        pimc/formatters/Fmt.hpp
        pimc/formatters/FmtChrono.hpp
        pimc/formatters/MemoryBuffer.hpp
        pimc/formatters/FailureFormatter.hpp
        pimc/formatters/IPFormatters.hpp
        pimc/formatters/ConsumeIfUnlessEscaped.hpp
        pimc/formatters/HexAsciiBlock.hpp
        pimc/formatters/NanosText.hpp
        pimc/formatters/IndentBlock.hpp
        pimc/formatters/detect_strerror_r.hpp
        pimc/formatters/IPv4Formatters.hpp
        pimc/formatters/IntfTableFormatter.hpp
        pimc/formatters/SysErrorFormatter.hpp
        pimc/formatters/IPv4Ostream.hpp
        pimc/formatters/SysErrorOstream.hpp
        pimc/parsers/IPParsers.hpp
        pimc/parsers/IPv4Parsers.hpp
        pimc/parsers/NumberParsers.hpp
        pimc/packets/PacketView.hpp
        pimc/packets/PacketWriter.hpp
        pimc/packets/IPv4HdrView.hpp
        pimc/packets/IPv4HdrWriter.hpp
        pimc/packets/UDPHdrView.hpp
        pimc/packets/IPChecksum.hpp
        pimc/packets/PIMSMv2.hpp
        pimc/packets/PIMSMv2Utils.hpp
        pimc/system/Exceptions.hpp
        pimc/system/SysError.hpp
        pimc/text/NumberLengths.hpp
        pimc/text/CString.hpp
        pimc/text/SCLine.hpp
        pimc/text/Plural.hpp
        pimc/text/Location.hpp
        pimc/time/TimeUtils.hpp
        pimc/time/Timestamp.hpp
        pimc/unix/SignalHandler.hpp
        pimc/unix/CapNetRaw.hpp
        pimc/unix/GetOptLong.hpp
        pimc/unix/TerminalColors.hpp
        pimc/events/FixedEventQueue.hpp
        pimc/yaml/Structured.hpp
        pimc/yaml/BuilderBase.hpp
        pimc/yaml/ErrorHandler.hpp
        pimc/yaml/LoadAll.hpp
)

find_file(CAPABILITY_H sys/capability.h)
find_library(LIBCAP cap)

if ((NOT CAPABILITY_H) OR (NOT LIBCAP))
    message(
            STATUS
            "PimcLib: sys/capability.h is not found, consider installing libcap-devel"
    )
    set(EXTRA_LIBS)
else()
    target_compile_definitions(
            PimcLib
            PUBLIC
            WITH_LIBCAP)
    set(EXTRA_LIBS ${LIBCAP})
endif()


target_include_directories(
        PimcLib
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(
        PimcLib
        PUBLIC
            ProjectSettings
            fmt::fmt
            yaml-cpp
            ${EXTRA_LIBS}
)

target_sources(
        Tests
        PRIVATE
            pimc/core/tests/TypeUtils-Tests.cpp
            pimc/core/tests/Optional-Result-TestUtils.hpp
            pimc/core/tests/Optional-Tests.cpp
            pimc/core/tests/Failure-Tests.cpp
            pimc/core/tests/ResultConstexpr-Tests.cpp
            pimc/core/tests/Result-Tests.cpp
            pimc/core/tests/Result-AdHoc-Tests.cpp
            pimc/core/tests/Deferred-Tests.cpp
            pimc/net/tests/IPv4Net-tests.cpp
            pimc/parsers/tests/IPv4Parser-tests.cpp
            pimc/parsers/tests/NumberParsers-Tests.cpp
            pimc/formatters/tests/IPv4Output-Tests.cpp
            pimc/formatters/tests/ConsumeIfUnlessEscaped-tests.cpp
            pimc/formatters/tests/NanosText-tests.cpp
)
