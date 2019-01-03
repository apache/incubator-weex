add_definitions(-DBUILDING_WITH_CMAKE=1)

# CODE_GENERATOR_PREPROCESSOR_WITH_LINEMARKERS only matters with GCC >= 4.7.0.  Since this
# version, -P does not output empty lines, which currently breaks make_names.pl in
# WebCore. Investigating whether make_names.pl should be changed instead is left as an exercise to
# the reader.
if (MSVC)
    # FIXME: Some codegenerators don't support paths with spaces. So use the executable name only.
    get_filename_component(CODE_GENERATOR_PREPROCESSOR_EXECUTABLE ${CMAKE_CXX_COMPILER} ABSOLUTE)

    set(CODE_GENERATOR_PREPROCESSOR_ARGUMENTS "/nologo /EP")
    set(CODE_GENERATOR_PREPROCESSOR "\"${CODE_GENERATOR_PREPROCESSOR_EXECUTABLE}\" ${CODE_GENERATOR_PREPROCESSOR_ARGUMENTS}")

    set(CODE_GENERATOR_PREPROCESSOR_WITH_LINEMARKERS ${CODE_GENERATOR_PREPROCESSOR})
else ()
    set(CODE_GENERATOR_PREPROCESSOR_EXECUTABLE ${CMAKE_CXX_COMPILER})

    set(CODE_GENERATOR_PREPROCESSOR_ARGUMENTS "-E -P -x c++")
    set(CODE_GENERATOR_PREPROCESSOR "${CODE_GENERATOR_PREPROCESSOR_EXECUTABLE} ${CODE_GENERATOR_PREPROCESSOR_ARGUMENTS}")

    set(CODE_GENERATOR_PREPROCESSOR_WITH_LINEMARKERS_ARGUMENTS "-E -x c++")
    set(CODE_GENERATOR_PREPROCESSOR_WITH_LINEMARKERS "${CODE_GENERATOR_PREPROCESSOR_EXECUTABLE} ${CODE_GENERATOR_PREPROCESSOR_WITH_LINEMARKERS_ARGUMENTS}")
endif ()

option(USE_THIN_ARCHIVES "Produce all static libraries as thin archives" ON)
if (USE_THIN_ARCHIVES)
    execute_process(COMMAND ${CMAKE_AR} -V OUTPUT_VARIABLE AR_VERSION)
    if ("${AR_VERSION}" MATCHES "^GNU ar")
        set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> crT <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> crT <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> rT <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> rT <TARGET> <LINK_FLAGS> <OBJECTS>")
    endif ()
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
define_property(TARGET PROPERTY FOLDER INHERITED BRIEF_DOCS "folder" FULL_DOCS "IDE folder name")

if (COMPILER_IS_GCC_OR_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fno-strict-aliasing")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-strict-aliasing -fno-rtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
endif ()

if (COMPILER_IS_CLANG AND CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif ()

if (WIN32 AND COMPILER_IS_GCC_OR_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields -Wno-unknown-pragmas")
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif ()

# Ensure that the default include system directories are added to the list of CMake implicit includes.
# This workarounds an issue that happens when using GCC 6 and using system includes (-isystem).
# For more details check: https://bugs.webkit.org/show_bug.cgi?id=161697
macro(DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang _compiler _flags _result)
    file(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy" "\n")
    separate_arguments(_buildFlags UNIX_COMMAND "${_flags}")
    execute_process(COMMAND ${_compiler} ${_buildFlags} -v -E -x ${_lang} -dD dummy
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles OUTPUT_QUIET
                    ERROR_VARIABLE _gccOutput)
    file(REMOVE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy")
    if ("${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+) *\n *End of (search) list")
        set(${_result} ${CMAKE_MATCH_1})
        string(REPLACE "\n" " " ${_result} "${${_result}}")
        separate_arguments(${_result})
    endif ()
endmacro()

if (CMAKE_COMPILER_IS_GNUCC)
   DETERMINE_GCC_SYSTEM_INCLUDE_DIRS("c" "${CMAKE_C_COMPILER}" "${CMAKE_C_FLAGS}" SYSTEM_INCLUDE_DIRS)
   set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES} ${SYSTEM_INCLUDE_DIRS})
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
   DETERMINE_GCC_SYSTEM_INCLUDE_DIRS("c++" "${CMAKE_CXX_COMPILER}" "${CMAKE_CXX_FLAGS}" SYSTEM_INCLUDE_DIRS)
   set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES} ${SYSTEM_INCLUDE_DIRS})
endif ()

# Detect Cortex-A53 core if CPU is ARM64 and OS is Linux.
# Query /proc/cpuinfo for each available core and check reported CPU part number: 0xd03 signals Cortex-A53.
# (see Main ID Register in ARM Cortex-A53 MPCore Processor Technical Reference Manual)
set(WTF_CPU_ARM64_CORTEXA53_INITIALVALUE OFF)
if (WTF_CPU_ARM64 AND (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
    execute_process(COMMAND nproc OUTPUT_VARIABLE PROC_COUNT)
    math(EXPR PROC_MAX ${PROC_COUNT}-1)
    foreach (PROC_ID RANGE ${PROC_MAX})
        execute_process(COMMAND taskset -c ${PROC_ID} grep "^CPU part" /proc/cpuinfo OUTPUT_VARIABLE PROC_PART)
        if (PROC_PART MATCHES "0xd03")
            set(WTF_CPU_ARM64_CORTEXA53_INITIALVALUE ON)
            break ()
        endif ()
    endforeach ()
endif ()
option(WTF_CPU_ARM64_CORTEXA53 "Enable Cortex-A53-specific code paths" ${WTF_CPU_ARM64_CORTEXA53_INITIALVALUE})

if (WTF_CPU_ARM64_CORTEXA53)
    if (NOT WTF_CPU_ARM64)
        message(FATAL_ERROR "WTF_CPU_ARM64_CORTEXA53 set without WTF_CPU_ARM64")
    endif ()
    include(TestCXXAcceptsFlag)
    CHECK_CXX_ACCEPTS_FLAG(-mfix-cortex-a53-835769 CXX_ACCEPTS_MFIX_CORTEX_A53_835769)
    if (CXX_ACCEPTS_MFIX_CORTEX_A53_835769)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfix-cortex-a53-835769")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfix-cortex-a53-835769")
        message(STATUS "Enabling Cortex-A53 workaround for compiler and disabling GNU gold linker, because it doesn't support this workaround.")
    endif ()
endif ()

EXPOSE_VARIABLE_TO_BUILD(WTF_CPU_ARM64_CORTEXA53)

set(ARM_TRADITIONAL_DETECTED FALSE)
if (WTF_CPU_ARM)
    set(ARM_THUMB2_TEST_SOURCE
    "
    #if !defined(thumb2) && !defined(__thumb2__)
    #error \"Thumb2 instruction set isn't available\"
    #endif
    int main() {}
   ")

    include(CheckCXXSourceCompiles)
    CHECK_CXX_SOURCE_COMPILES("${ARM_THUMB2_TEST_SOURCE}" ARM_THUMB2_DETECTED)
    if (NOT ARM_THUMB2_DETECTED)
        set(ARM_TRADITIONAL_DETECTED TRUE)
        # See https://bugs.webkit.org/show_bug.cgi?id=159880#c4 for details.
        message(STATUS "Disabling GNU gold linker, because it doesn't support ARM instruction set properly.")
    endif ()
endif ()

# Use ld.gold if it is available and isn't disabled explicitly
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(USE_LD_GOLD "Use GNU gold linker" ON
                       "NOT CXX_ACCEPTS_MFIX_CORTEX_A53_835769;NOT ARM_TRADITIONAL_DETECTED;NOT WIN32;NOT APPLE" OFF)
if (USE_LD_GOLD)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
    if ("${LD_VERSION}" MATCHES "GNU gold")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    else ()
        message(WARNING "GNU gold linker isn't available, using the default system linker.")
        set(USE_LD_GOLD OFF)
    endif ()
endif ()

set(ENABLE_DEBUG_FISSION_DEFAULT OFF)
if (USE_LD_GOLD AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(TestCXXAcceptsFlag)
    CHECK_CXX_ACCEPTS_FLAG(-gsplit-dwarf CXX_ACCEPTS_GSPLIT_DWARF)
    if (CXX_ACCEPTS_GSPLIT_DWARF)
        set(ENABLE_DEBUG_FISSION_DEFAULT ON)
    endif ()
endif ()

option(DEBUG_FISSION "Use Debug Fission support" ${ENABLE_DEBUG_FISSION_DEFAULT})

if (DEBUG_FISSION)
    if (NOT USE_LD_GOLD)
        message(FATAL_ERROR "Need GNU gold linker for Debug Fission support")
    endif ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gsplit-dwarf")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gsplit-dwarf")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gdb-index")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gdb-index")
endif ()

if (COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
endif ()

string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} LOWERCASE_CMAKE_HOST_SYSTEM_PROCESSOR)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND "${LOWERCASE_CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "(i[3-6]86|x86|armhf)")
    # To avoid out of memory when building with debug option in 32bit system.
    # See https://bugs.webkit.org/show_bug.cgi?id=77327
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "-Wl,--no-keep-memory ${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
endif ()

if (NOT MSVC)
    string(REGEX MATCHALL "-fsanitize=[^ ]*" ENABLED_COMPILER_SANITIZERS ${CMAKE_CXX_FLAGS})
endif ()

if (UNIX AND NOT APPLE AND NOT ENABLED_COMPILER_SANITIZERS)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
endif ()

if (USE_ARM_LLVM_DISASSEMBLER)
    find_package(LLVM REQUIRED)
    SET_AND_EXPOSE_TO_BUILD(HAVE_LLVM TRUE)
endif ()

# Enable the usage of OpenMP.
#  - At this moment, OpenMP is only used as an alternative implementation
#    to native threads for the parallelization of the SVG filters.
if (USE_OPENMP)
    find_package(OpenMP REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

# GTK uses the GNU installation directories as defaults.
if (NOT PORT STREQUAL "GTK")
    set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Absolute path to library installation directory")
    set(EXEC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Absolute path to executable installation directory")
    set(LIBEXEC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Absolute path to install executables executed by the library")
endif ()

# The Ninja generator does not yet know how to build archives in pieces, and so response
# files must be used to deal with very long linker command lines.
# See https://bugs.webkit.org/show_bug.cgi?id=129771
# The Apple Toolchain doesn't support response files.
if (NOT APPLE)
   # If using Ninja with cmake >= 3.6.0 and icecream, then the build is broken
   # if enable the response files. See https://bugs.webkit.org/show_bug.cgi?id=168770
   if (NOT ((${CMAKE_CXX_COMPILER} MATCHES ".*icecc.*") AND (CMAKE_GENERATOR STREQUAL "Ninja") AND (${CMAKE_VERSION} VERSION_GREATER 3.5)))
      set(CMAKE_NINJA_FORCE_RESPONSE_FILE 1)
   endif ()
endif ()

# Macros for determining HAVE values.
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckStructHasMember)

macro(_HAVE_CHECK_INCLUDE _variable _header)
    check_include_file(${_header} ${_variable}_value)
    SET_AND_EXPOSE_TO_BUILD(${_variable} ${_variable}_value)
endmacro()

macro(_HAVE_CHECK_FUNCTION _variable _function)
    check_function_exists(${_function} ${_variable}_value)
    SET_AND_EXPOSE_TO_BUILD(${_variable} ${_variable}_value)
endmacro()

macro(_HAVE_CHECK_SYMBOL _variable _symbol _header)
    check_symbol_exists(${_symbol} ${_header} ${_variable}_value)
    SET_AND_EXPOSE_TO_BUILD(${_variable} ${_variable}_value)
endmacro()

macro(_HAVE_CHECK_STRUCT _variable _struct _member _header)
    check_struct_has_member(${_struct} ${_member} ${_header} ${_variable}_value)
    SET_AND_EXPOSE_TO_BUILD(${_variable} ${_variable}_value)
endmacro()

# Check whether features.h header exists.
# Including glibc's one defines __GLIBC__, that is used in Platform.h
_HAVE_CHECK_INCLUDE(HAVE_FEATURES_H features.h)

# Check for headers
_HAVE_CHECK_INCLUDE(HAVE_ERRNO_H errno.h)
_HAVE_CHECK_INCLUDE(HAVE_LANGINFO_H langinfo.h)
_HAVE_CHECK_INCLUDE(HAVE_MMAP sys/mman.h)
_HAVE_CHECK_INCLUDE(HAVE_PTHREAD_NP_H pthread_np.h)
_HAVE_CHECK_INCLUDE(HAVE_STRINGS_H strings.h)
_HAVE_CHECK_INCLUDE(HAVE_SYS_PARAM_H sys/param.h)
_HAVE_CHECK_INCLUDE(HAVE_SYS_TIME_H sys/time.h)
_HAVE_CHECK_INCLUDE(HAVE_SYS_TIMEB_H sys/timeb.h)

# Check for functions
_HAVE_CHECK_FUNCTION(HAVE_ALIGNED_MALLOC _aligned_malloc)
_HAVE_CHECK_FUNCTION(HAVE_ISDEBUGGERPRESENT IsDebuggerPresent)
_HAVE_CHECK_FUNCTION(HAVE_LOCALTIME_R localtime_r)
_HAVE_CHECK_FUNCTION(HAVE_STRNSTR strnstr)
_HAVE_CHECK_FUNCTION(HAVE_TIMEGM timegm)
_HAVE_CHECK_FUNCTION(HAVE_VASPRINTF vasprintf)

# Check for symbols
# Windows has signal.h but is missing symbols that are used in calls to signal.
_HAVE_CHECK_SYMBOL(HAVE_SIGNAL_H SIGTRAP signal.h)

# Check for struct members
_HAVE_CHECK_STRUCT(HAVE_STAT_BIRTHTIME "struct stat" st_birthtime sys/stat.h)
_HAVE_CHECK_STRUCT(HAVE_TM_GMTOFF "struct tm" tm_gmtoff time.h)
_HAVE_CHECK_STRUCT(HAVE_TM_ZONE "struct tm" tm_zone time.h)
