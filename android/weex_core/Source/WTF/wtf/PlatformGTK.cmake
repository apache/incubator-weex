set(WTF_LIBRARY_TYPE STATIC)
set(WTF_OUTPUT_NAME WTFGTK)

list(APPEND WTF_SOURCES
    generic/WorkQueueGeneric.cpp
    glib/GLibUtilities.cpp
    glib/GRefPtr.cpp
    glib/MainThreadGLib.cpp
    glib/RunLoopGLib.cpp

    linux/CurrentProcessMemoryStatus.cpp
    linux/MemoryPressureHandlerLinux.cpp

    PlatformUserPreferredLanguagesUnix.cpp
    UniStdExtras.cpp

    text/gtk/TextBreakIteratorInternalICUGtk.cpp
)

list(APPEND WTF_LIBRARIES
    ${GLIB_GIO_LIBRARIES}
    ${GLIB_GOBJECT_LIBRARIES}
    ${GLIB_LIBRARIES}
    pthread
    ${ZLIB_LIBRARIES}
)

list(APPEND WTF_SYSTEM_INCLUDE_DIRECTORIES
    ${GLIB_INCLUDE_DIRS}
)
