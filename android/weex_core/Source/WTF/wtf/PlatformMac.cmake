set(WTF_LIBRARY_TYPE STATIC)

find_library(COCOA_LIBRARY Cocoa)
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(READLINE_LIBRARY Readline)
list(APPEND WTF_LIBRARIES
    ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIBRARY}
    ${READLINE_LIBRARY}
    libicucore.dylib
)

list(APPEND WTF_SOURCES
    AutodrainedPoolMac.mm
    BlockObjCExceptions.mm
    PlatformUserPreferredLanguagesMac.mm
    RunLoopTimerCF.cpp
    SchedulePairCF.cpp
    SchedulePairMac.mm

    text/mac/TextBreakIteratorInternalICUMac.mm

    cf/RunLoopCF.cpp

    cocoa/MemoryPressureHandlerCocoa.mm
    cocoa/WorkQueueCocoa.cpp

    mac/DeprecatedSymbolsUsedBySafari.mm
    mac/MainThreadMac.mm

    text/cf/AtomicStringImplCF.cpp
    text/cf/StringCF.cpp
    text/cf/StringImplCF.cpp
    text/cf/StringViewCF.cpp

    text/mac/StringImplMac.mm
    text/mac/StringMac.mm
    text/mac/StringViewObjC.mm
)

list(APPEND WTF_INCLUDE_DIRECTORIES
    "${WTF_DIR}/icu"
    "${WTF_DIR}/wtf/spi/darwin"
)
