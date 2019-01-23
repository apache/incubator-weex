include(CMakeParseArguments)
# Sets extra compile flags for a target, depending on the compiler being used.
# Currently, only GCC is supported.
macro(WEEX_SET_EXTRA_COMPILER_FLAGS _target)
    set(options ENABLE_WERROR IGNORECXX_WARNINGS)
    CMAKE_PARSE_ARGUMENTS("OPTION" "${options}" "" "" ${ARGN})
    if (COMPILER_IS_GCC_OR_CLANG)
        get_target_property(OLD_COMPILE_FLAGS ${_target} COMPILE_FLAGS)
        if (${OLD_COMPILE_FLAGS} STREQUAL "OLD_COMPILE_FLAGS-NOTFOUND")
            set(OLD_COMPILE_FLAGS "")
        endif ()

        if (NOT WIN32)
            get_target_property(TARGET_TYPE ${_target} TYPE)
            if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY") # -fPIC is automatically added to shared libraries
                set(OLD_COMPILE_FLAGS "-fPIC ${OLD_COMPILE_FLAGS}")
            endif ()
        endif ()

        # Suppress -Wparentheses-equality warning of Clang
        if (COMPILER_IS_CLANG)
            set(OLD_COMPILE_FLAGS "-Wno-parentheses-equality ${OLD_COMPILE_FLAGS}")
        endif ()

        # Suppress -Wmissing-field-initializers due to a GCC bug, see https://bugs.webkit.org/show_bug.cgi?id=157888 for details.
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION MATCHES "4\\.9")
            set(OLD_COMPILE_FLAGS "-Wno-missing-field-initializers ${OLD_COMPILE_FLAGS}")
        endif ()

        # Enable warnings by default
        if (NOT ${OPTION_IGNORECXX_WARNINGS})
            set(OLD_COMPILE_FLAGS "-Wall -Wextra -Wcast-align -Wformat-security -Wmissing-format-attribute -Wpointer-arith -Wundef -Wwrite-strings ${OLD_COMPILE_FLAGS}")
        endif ()

        # Enable errors on warning
        if (OPTION_ENABLE_WERROR)
            set(OLD_COMPILE_FLAGS "-Werror ${OLD_COMPILE_FLAGS}")
        endif ()

        set_target_properties(${_target} PROPERTIES
            COMPILE_FLAGS "${OLD_COMPILE_FLAGS}")

        unset(OLD_COMPILE_FLAGS)
    endif ()
endmacro()


# Append the given flag to the target property.
# Builds on top of get_target_property() and set_target_properties()
macro(ADD_TARGET_PROPERTIES _target _property _flags)
    get_target_property(_tmp ${_target} ${_property})
    if (NOT _tmp)
        set(_tmp "")
    endif (NOT _tmp)

    foreach (f ${_flags})
        set(_tmp "${_tmp} ${f}")
    endforeach (f ${_flags})

    set_target_properties(${_target} PROPERTIES ${_property} ${_tmp})
    unset(_tmp)
endmacro(ADD_TARGET_PROPERTIES _target _property _flags)


# Append the given dependencies to the source file
macro(ADD_SOURCE_DEPENDENCIES _source _deps)
    get_source_file_property(_tmp ${_source} OBJECT_DEPENDS)
    if (NOT _tmp)
        set(_tmp "")
    endif ()

    foreach (f ${_deps})
        list(APPEND _tmp "${f}")
    endforeach ()

    set_source_files_properties(${_source} PROPERTIES OBJECT_DEPENDS "${_tmp}")
    unset(_tmp)
endmacro()


# Append the given dependencies to the source file
# This one consider the given dependencies are in ${DERIVED_SOURCES_WEBCORE_DIR}
# and prepends this to every member of dependencies list
macro(ADD_SOURCE_WEBCORE_DERIVED_DEPENDENCIES _source _deps)
    set(_tmp "")
    foreach (f ${_deps})
        list(APPEND _tmp "${DERIVED_SOURCES_WEBCORE_DIR}/${f}")
    endforeach ()

    ADD_SOURCE_DEPENDENCIES(${_source} ${_tmp})
    unset(_tmp)
endmacro()

macro(CALCULATE_LIBRARY_VERSIONS_FROM_LIBTOOL_TRIPLE library_name current revision age)
    math(EXPR ${library_name}_VERSION_MAJOR "${current} - ${age}")
    set(${library_name}_VERSION_MINOR ${age})
    set(${library_name}_VERSION_MICRO ${revision})
    set(${library_name}_VERSION ${${library_name}_VERSION_MAJOR}.${age}.${revision})
endmacro()

macro(POPULATE_LIBRARY_VERSION library_name)
if (NOT DEFINED ${library_name}_VERSION_MAJOR)
    set(${library_name}_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
endif ()
if (NOT DEFINED ${library_name}_VERSION_MINOR)
    set(${library_name}_VERSION_MINOR ${PROJECT_VERSION_MINOR})
endif ()
if (NOT DEFINED ${library_name}_VERSION_MICRO)
    set(${library_name}_VERSION_MICRO ${PROJECT_VERSION_MICRO})
endif ()
if (NOT DEFINED ${library_name}_VERSION)
    set(${library_name}_VERSION ${PROJECT_VERSION})
endif ()
endmacro()
