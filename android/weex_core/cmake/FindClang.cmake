#
# Check for Clang.
#
# The following variables are set:
#  CLANG_EXE
#  CLANG_VERSION

find_program(CLANG_EXE NAMES "clang")

execute_process(COMMAND ${CLANG_EXE} --version OUTPUT_VARIABLE CLANG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CLANG DEFAULT_MSG
                                  CLANG_VERSION)

mark_as_advanced(CLANG_VERSION)
