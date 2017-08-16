list(APPEND WTF_SOURCES
    PlatformUserPreferredLanguagesWin.cpp

    text/win/TextBreakIteratorInternalICUWin.cpp

    text/cf/AtomicStringImplCF.cpp
    text/cf/StringCF.cpp
    text/cf/StringImplCF.cpp
    text/cf/StringViewCF.cpp

    win/MainThreadWin.cpp
    win/MemoryPressureHandlerWin.cpp
    win/RunLoopWin.cpp
    win/WorkQueueWin.cpp
)

list(APPEND WTF_LIBRARIES
    winmm
)

if (${WTF_PLATFORM_WIN_CAIRO})
    list(APPEND WTF_LIBRARIES
        cflite
    )
else ()
    list(APPEND WTF_LIBRARIES
        CoreFoundation${DEBUG_SUFFIX}
    )
endif ()

set(WTF_PRE_BUILD_COMMAND "${CMAKE_BINARY_DIR}/DerivedSources/WTF/preBuild.cmd")
file(WRITE "${WTF_PRE_BUILD_COMMAND}" "@xcopy /y /s /d /f \"${WTF_DIR}/wtf/*.h\" \"${DERIVED_SOURCES_DIR}/ForwardingHeaders/WTF\" >nul 2>nul\n@xcopy /y /s /d /f \"${DERIVED_SOURCES_DIR}/WTF/*.h\" \"${DERIVED_SOURCES_DIR}/ForwardingHeaders/WTF\" >nul 2>nul\n")
file(MAKE_DIRECTORY ${DERIVED_SOURCES_DIR}/ForwardingHeaders/WTF)

set(WTF_OUTPUT_NAME WTF${DEBUG_SUFFIX})
