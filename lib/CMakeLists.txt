set_target_properties(
        PimcLib
        PROPERTIES EFFECTIVE_SOURCES_DIR ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(
    PimcLib
    PRIVATE
        net/IPv4IntfTable.cpp
        system/SysError.cpp
    PUBLIC
        core/CompilerUtils.hpp
        core/TypeUtils.hpp
        core/Deferred.hpp
        core/Result.hpp
        core/Endian.hpp
        net/IPv4HdrView.hpp
        net/UDPHdrView.hpp
        net/IPv4PktInfo.hpp
        net/IPv4Prefix.hpp
        net/IPv4Address.hpp
        net/IPv4IntfTable.hpp
        formatters/IPv4Formatters.hpp
        formatters/IPv4Ostream.hpp
        parsers/IPv4Parsers.hpp
        parsers/NumberParsers.hpp
        dissect/PacketView.hpp
        system/Exceptions.hpp
        system/SysError.hpp
        text/MemoryBuffer.hpp
        text/NumberLengths.hpp
        text/ConsumeIfUnlessEscaped.hpp
        text/HexAsciiBlock.hpp
        text/NanosText.hpp
        text/IndentBlock.hpp
        text/CString.hpp
        text/SCLine.hpp
        unix/SignalHandler.hpp
        unix/GetOptLong.hpp
        unix/GetOptLong.cpp
        unix/TerminalColors.hpp
)

export_headers(PimcLib ${CMAKE_CURRENT_LIST_DIR} pimc)

target_link_libraries(
        PimcLib
        PUBLIC
            ProjectSettings
            fmt::fmt
)

target_sources(
        Tests
        PRIVATE
            core/tests/TypeUtils-Tests.cpp
            core/tests/Failure-Tests.cpp
            core/tests/ResultConstexpr-Tests.cpp
            core/tests/Result-Tests.cpp
            core/tests/Deferred-Tests.cpp
            net/tests/IPv4Net-tests.cpp
            parsers/tests/IPv4Parser-tests.cpp
            parsers/tests/NumberParsers-Tests.cpp
            formatters/tests/IPv4Output-Tests.cpp
            text/tests/ConsumeIfUnlessEscaped-tests.cpp
            text/tests/NanosText-tests.cpp
            time/TimeUtils.hpp
)
