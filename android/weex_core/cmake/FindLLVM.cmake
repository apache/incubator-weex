#
# Check if the llvm-config gives us the path for the llvm libs.
#
# The following variables are set:
#  LLVM_CONFIG_EXE
#  LLVM_VERSION
#  LLVM_INCLUDE_DIRS - include directories for the llvm headers.
#  LLVM_STATIC_LIBRARIES - list of paths for the static llvm libraries.


foreach (_program_name llvm-config llvm-config-3.7 llvm-config-3.6 llvm-config-3.5)
    find_program(LLVM_CONFIG_EXE NAMES ${_program_name})
    if (LLVM_CONFIG_EXE)
        execute_process(COMMAND ${LLVM_CONFIG_EXE} --version OUTPUT_VARIABLE LLVM_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        if ("${LLVM_VERSION}" VERSION_LESS "${LLVM_FIND_VERSION}")
            unset(LLVM_CONFIG_EXE CACHE)
        else ()
            break ()
        endif ()
    endif ()
endforeach ()

execute_process(COMMAND ${LLVM_CONFIG_EXE} --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXE} --libfiles OUTPUT_VARIABLE LLVM_STATIC_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXE} --system-libs OUTPUT_VARIABLE LLVM_SYSTEM_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXE} --libdir OUTPUT_VARIABLE LLVM_LIBS_DIRECTORY OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXE} --libs OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXE} --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

# Depending on how llvm was built, we could have either a global .so file when built using autotools,
# or multiple .so files for each static library when built using CMake. So, we set the LLVM_LIBS_DIRECTORY
# variable here accordingly for each case.
# We need to build the soname manually in any case, since there's currently no way to get it from llvm-config.
set(LLVM_SONAME "LLVM-${LLVM_VERSION}")
if (EXISTS "${LLVM_LIBS_DIRECTORY}/lib${LLVM_SONAME}.so")
    set(LLVM_LIBRARIES "${LLVM_LDFLAGS} -l${LLVM_SONAME}")
else ()
    set(LLVM_LIBRARIES "${LLVM_LDFLAGS} ${LLVM_LIBS}")
endif ()

# convert the list of paths into a cmake list
separate_arguments(LLVM_STATIC_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LLVM
                                  REQUIRED_VARS LLVM_VERSION LLVM_INCLUDE_DIRS LLVM_LIBRARIES LLVM_STATIC_LIBRARIES
                                  VERSION_VAR LLVM_VERSION)

mark_as_advanced(LLVM_VERSION LLVM_INCLUDE_DIRS LLVM_LIBRARIES LLVM_STATIC_LIBRARIES)
