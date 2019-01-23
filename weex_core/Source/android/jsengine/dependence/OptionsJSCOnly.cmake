# find_package(Threads REQUIRED)

add_definitions(-DBUILDING_JSCONLY__)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_MICRO 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_MICRO})

set(ALL_EVENT_LOOP_TYPES
    GLib
    Generic
)

set(DEFAULT_EVENT_LOOP_TYPE "Generic")

set(EVENT_LOOP_TYPE ${DEFAULT_EVENT_LOOP_TYPE} CACHE STRING "Implementation of event loop to be used in JavaScriptCore (one of ${ALL_EVENT_LOOP_TYPES})")

if (WTF_CPU_X86 OR WTF_CPU_X86_64)
    SET_AND_EXPOSE_TO_BUILD(USE_UDIS86 1)
endif ()

if (ENABLE_STATIC_JSC)
    set(JavaScriptCore_LIBRARY_TYPE STATIC)
endif ()

string(TOLOWER ${EVENT_LOOP_TYPE} LOWERCASE_EVENT_LOOP_TYPE)
if (LOWERCASE_EVENT_LOOP_TYPE STREQUAL "glib")
    find_package(GLIB 2.36 REQUIRED COMPONENTS gio gobject)
    SET_AND_EXPOSE_TO_BUILD(USE_GLIB 1)
    SET_AND_EXPOSE_TO_BUILD(USE_GLIB_EVENT_LOOP 1)
    SET_AND_EXPOSE_TO_BUILD(WTF_DEFAULT_EVENT_LOOP 0)
else ()
    SET_AND_EXPOSE_TO_BUILD(USE_GENERIC_EVENT_LOOP 1)
    SET_AND_EXPOSE_TO_BUILD(WTF_DEFAULT_EVENT_LOOP 0)
endif ()

#if (NOT APPLE)
#    # find_package(ICU REQUIRED)
#else ()
#    set(ICU_INCLUDE_DIRS
#        "${WEBCORE_DIR}/icu"
#        "${JAVASCRIPTCORE_DIR}/icu"
#        "${WTF_DIR}/icu"
#    )
#    set(ICU_LIBRARIES libicucore.dylib)
#endif ()

set(ICU_INCLUDE_DIRS
    "${COMMON_ICU_DIR}"
    "${WTF_DIR}/icu"
)
message("bbbbbbbbbbbbbbbbb ${COMMON_ICU_DIR}")

# From OptionsGTK.cmake
if (CMAKE_MAJOR_VERSION LESS 3)
    # Before CMake 3 it was necessary to use a build script instead of using cmake --build directly
    # to preserve colors and pretty-printing.

    build_command(COMMAND_LINE_TO_BUILD)
    # build_command unconditionally adds -i (ignore errors) for make, and there's
    # no reasonable way to turn that off, so we just replace it with -k, which has
    # the same effect, except that the return code will indicate that an error occurred.
    # See: http://www.cmake.org/cmake/help/v3.0/command/build_command.html
    string(REPLACE " -i" " -k" COMMAND_LINE_TO_BUILD ${COMMAND_LINE_TO_BUILD})
    file(WRITE
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build.sh
        "#!/bin/sh\n"
        "${COMMAND_LINE_TO_BUILD} $@"
    )
    file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build.sh
        DESTINATION ${CMAKE_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
    )
endif ()
