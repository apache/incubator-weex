include(CMakeParseArguments)
include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)

macro(WEEX_INCLUDE_CONFIG_FILES_IF_EXISTS)
    set(_file ${CMAKE_CURRENT_SOURCE_DIR}/Platform${PORT}.cmake)
    if (EXISTS ${_file})
        message(STATUS "Using platform-specific CMakeLists: ${_file}")
        include(${_file})
    else ()
        message(STATUS "Platform-specific CMakeLists not found: ${_file}")
    endif ()
endmacro()

# Append the given dependencies to the source file
macro(ADD_SOURCE_DEPENDENCIES _source _deps)
    set(_tmp)
    get_source_file_property(_tmp ${_source} OBJECT_DEPENDS)
    if (NOT _tmp)
        set(_tmp "")
    endif ()

    foreach (f ${_deps})
        list(APPEND _tmp "${f}")
    endforeach ()

    set_source_files_properties(${_source} PROPERTIES OBJECT_DEPENDS "${_tmp}")
endmacro()

macro(ADD_PRECOMPILED_HEADER _header _cpp _source)
    if (MSVC)
        get_filename_component(PrecompiledBasename ${_cpp} NAME_WE)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_source}")
        set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${_source}/${PrecompiledBasename}.pch")
        set(_sources ${${_source}})

        set_source_files_properties(${_cpp}
            PROPERTIES COMPILE_FLAGS "/Yc\"${_header}\" /Fp\"${PrecompiledBinary}\""
            OBJECT_OUTPUTS "${PrecompiledBinary}")
        set_source_files_properties(${_sources}
            PROPERTIES COMPILE_FLAGS "/Yu\"${_header}\" /FI\"${_header}\" /Fp\"${PrecompiledBinary}\""
            OBJECT_DEPENDS "${PrecompiledBinary}")
        list(APPEND ${_source} ${_cpp})
    endif ()
    #FIXME: Add support for Xcode.
endmacro()

option(SHOW_BINDINGS_GENERATION_PROGRESS "Show progress of generating bindings" OFF)

# Helper macro which wraps generate-bindings-all.pl script.
#   target is a new target name to be added
#   OUTPUT_SOURCE is a list name which will contain generated sources.(eg. WebCore_SOURCES)
#   INPUT_FILES are IDL files to generate.
#   BASE_DIR is base directory where script is called.
#   IDL_INCLUDES is value of --include argument. (eg. ${WEBCORE_DIR}/bindings/js)
#   FEATURES is a value of --defines argument.
#   DESTINATION is a value of --outputDir argument.
#   GENERATOR is a value of --generator argument.
#   SUPPLEMENTAL_DEPFILE is a value of --supplementalDependencyFile. (optional)
#   PP_EXTRA_OUTPUT is extra outputs of preprocess-idls.pl. (optional)
#   PP_EXTRA_ARGS is extra arguments for preprocess-idls.pl. (optional)
function(GENERATE_BINDINGS target)
    set(options)
    set(oneValueArgs OUTPUT_SOURCE BASE_DIR FEATURES DESTINATION GENERATOR SUPPLEMENTAL_DEPFILE)
    set(multiValueArgs INPUT_FILES IDL_INCLUDES PP_EXTRA_OUTPUT PP_EXTRA_ARGS)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(binding_generator ${WEBCORE_DIR}/bindings/scripts/generate-bindings-all.pl)
    set(idl_attributes_file ${WEBCORE_DIR}/bindings/scripts/IDLAttributes.txt)
    set(idl_files_list ${CMAKE_CURRENT_BINARY_DIR}/idl_files_${target}.tmp)
    set(_supplemental_dependency)

    set(content)
    foreach (f ${arg_INPUT_FILES})
        if (NOT IS_ABSOLUTE ${f})
            set(f ${CMAKE_CURRENT_SOURCE_DIR}/${f})
        endif ()
        set(content "${content}${f}\n")
    endforeach ()
    file(WRITE ${idl_files_list} ${content})

    set(args
        --defines ${arg_FEATURES}
        --generator ${arg_GENERATOR}
        --outputDir ${arg_DESTINATION}
        --idlFilesList ${idl_files_list}
        --preprocessor "${CODE_GENERATOR_PREPROCESSOR}"
        --idlAttributesFile ${idl_attributes_file})
    if (arg_SUPPLEMENTAL_DEPFILE)
        list(APPEND args --supplementalDependencyFile ${arg_SUPPLEMENTAL_DEPFILE})
    endif ()
    if (PROCESSOR_COUNT)
        list(APPEND args --numOfJobs ${PROCESSOR_COUNT})
    endif ()
    foreach (i IN LISTS arg_IDL_INCLUDES)
        if (IS_ABSOLUTE ${i})
            list(APPEND args --include ${i})
        else ()
            list(APPEND args --include ${CMAKE_CURRENT_SOURCE_DIR}/${i})
        endif ()
    endforeach ()
    foreach (i IN LISTS arg_PP_EXTRA_OUTPUT)
        list(APPEND args --ppExtraOutput ${i})
    endforeach ()
    foreach (i IN LISTS arg_PP_EXTRA_ARGS)
        list(APPEND args --ppExtraArgs ${i})
    endforeach ()

    set(common_generator_dependencies
        ${WEBCORE_DIR}/bindings/scripts/generate-bindings.pl
        ${SCRIPTS_BINDINGS}
        # Changing enabled features should trigger recompiling all IDL files
        # because some of them use #if.
        ${CMAKE_BINARY_DIR}/cmakeconfig.h
    )
    if (EXISTS ${WEBCORE_DIR}/bindings/scripts/CodeGenerator${arg_GENERATOR}.pm)
        list(APPEND common_generator_dependencies ${WEBCORE_DIR}/bindings/scripts/CodeGenerator${arg_GENERATOR}.pm)
    endif ()
    if (EXISTS ${arg_BASE_DIR}/CodeGenerator${arg_GENERATOR}.pm)
        list(APPEND common_generator_dependencies ${arg_BASE_DIR}/CodeGenerator${arg_GENERATOR}.pm)
    endif ()
    foreach (i IN LISTS common_generator_dependencies)
        list(APPEND args --generatorDependency ${i})
    endforeach ()

    set(gen_sources)
    set(gen_headers)
    foreach (_file ${arg_INPUT_FILES})
        get_filename_component(_name ${_file} NAME_WE)
        list(APPEND gen_sources ${arg_DESTINATION}/JS${_name}.cpp)
        list(APPEND gen_headers ${arg_DESTINATION}/JS${_name}.h)
    endforeach ()
    set(${arg_OUTPUT_SOURCE} ${${arg_OUTPUT_SOURCE}} ${gen_sources} PARENT_SCOPE)
    set(act_args)
    if (SHOW_BINDINGS_GENERATION_PROGRESS)
        list(APPEND args --showProgress)
    endif ()
    if (${CMAKE_VERSION} VERSION_LESS 3.2)
        set_source_files_properties(${gen_sources} ${gen_headers} PROPERTIES GENERATED 1)
    else ()
        list(APPEND act_args BYPRODUCTS ${gen_sources} ${gen_headers})
        if (SHOW_BINDINGS_GENERATION_PROGRESS)
            list(APPEND act_args USES_TERMINAL)
        endif ()
    endif ()
    add_custom_target(${target}
        COMMAND ${PERL_EXECUTABLE} ${binding_generator} ${args}
        WORKING_DIRECTORY ${arg_BASE_DIR}
        COMMENT "Generate bindings (${target})"
        VERBATIM ${act_args})
endfunction()

macro(GENERATE_FONT_NAMES _infile)
    set(NAMES_GENERATOR ${WEBCORE_DIR}/dom/make_names.pl)
    set(_arguments  --fonts ${_infile})
    set(_outputfiles ${DERIVED_SOURCES_WEBCORE_DIR}/WebKitFontFamilyNames.cpp ${DERIVED_SOURCES_WEBCORE_DIR}/WebKitFontFamilyNames.h)

    add_custom_command(
        OUTPUT  ${_outputfiles}
        MAIN_DEPENDENCY ${_infile}
        DEPENDS ${MAKE_NAMES_DEPENDENCIES} ${NAMES_GENERATOR} ${SCRIPTS_BINDINGS}
        COMMAND ${PERL_EXECUTABLE} ${NAMES_GENERATOR} --outputDir ${DERIVED_SOURCES_WEBCORE_DIR} ${_arguments}
        VERBATIM)
endmacro()


macro(GENERATE_EVENT_FACTORY _infile _outfile)
    set(NAMES_GENERATOR ${WEBCORE_DIR}/dom/make_event_factory.pl)

    add_custom_command(
        OUTPUT  ${DERIVED_SOURCES_WEBCORE_DIR}/${_outfile}
        MAIN_DEPENDENCY ${_infile}
        DEPENDS ${NAMES_GENERATOR} ${SCRIPTS_BINDINGS}
        COMMAND ${PERL_EXECUTABLE} ${NAMES_GENERATOR} --input ${_infile} --outputDir ${DERIVED_SOURCES_WEBCORE_DIR}
        VERBATIM)
endmacro()


macro(GENERATE_EXCEPTION_CODE_DESCRIPTION _infile _outfile)
    set(NAMES_GENERATOR ${WEBCORE_DIR}/dom/make_dom_exceptions.pl)

    add_custom_command(
        OUTPUT  ${DERIVED_SOURCES_WEBCORE_DIR}/${_outfile}
        MAIN_DEPENDENCY ${_infile}
        DEPENDS ${NAMES_GENERATOR} ${SCRIPTS_BINDINGS}
        COMMAND ${PERL_EXECUTABLE} ${NAMES_GENERATOR} --input ${_infile} --outputDir ${DERIVED_SOURCES_WEBCORE_DIR}
        VERBATIM)
endmacro()


macro(GENERATE_SETTINGS_MACROS _infile _outfile)
    set(NAMES_GENERATOR ${WEBCORE_DIR}/page/make_settings.pl)

    # Do not list the output in more than one independent target that may
    # build in parallel or the two instances of the rule may conflict.
    # <https://cmake.org/cmake/help/v3.0/command/add_custom_command.html>
    set(_extra_output
        ${DERIVED_SOURCES_WEBCORE_DIR}/InternalSettingsGenerated.h
        ${DERIVED_SOURCES_WEBCORE_DIR}/InternalSettingsGenerated.cpp
        ${DERIVED_SOURCES_WEBCORE_DIR}/InternalSettingsGenerated.idl
    )
    set(_args BYPRODUCTS ${_extra_output})
    if (${CMAKE_VERSION} VERSION_LESS 3.2)
        set_source_files_properties(${_extra_output} PROPERTIES GENERATED 1)
        set(_args)
    endif ()
    add_custom_command(
        OUTPUT ${DERIVED_SOURCES_WEBCORE_DIR}/${_outfile}
        MAIN_DEPENDENCY ${_infile}
        DEPENDS ${NAMES_GENERATOR} ${SCRIPTS_BINDINGS}
        COMMAND ${PERL_EXECUTABLE} ${NAMES_GENERATOR} --input ${_infile} --outputDir ${DERIVED_SOURCES_WEBCORE_DIR}
        VERBATIM ${_args})
endmacro()


macro(GENERATE_DOM_NAMES _namespace _attrs)
    set(NAMES_GENERATOR ${WEBCORE_DIR}/dom/make_names.pl)
    set(_arguments  --attrs ${_attrs})
    set(_outputfiles ${DERIVED_SOURCES_WEBCORE_DIR}/${_namespace}Names.cpp ${DERIVED_SOURCES_WEBCORE_DIR}/${_namespace}Names.h)
    set(_extradef)
    set(_tags)

    foreach (f ${ARGN})
        if (_tags)
            set(_extradef "${_extradef} ${f}")
        else ()
            set(_tags ${f})
        endif ()
    endforeach ()

    if (_tags)
        set(_arguments "${_arguments}" --tags ${_tags} --factory --wrapperFactory)
        set(_outputfiles "${_outputfiles}" ${DERIVED_SOURCES_WEBCORE_DIR}/${_namespace}ElementFactory.cpp ${DERIVED_SOURCES_WEBCORE_DIR}/${_namespace}ElementFactory.h ${DERIVED_SOURCES_WEBCORE_DIR}/JS${_namespace}ElementWrapperFactory.cpp ${DERIVED_SOURCES_WEBCORE_DIR}/JS${_namespace}ElementWrapperFactory.h)
    endif ()

    if (_extradef)
        set(_additionArguments "${_additionArguments}" --extraDefines=${_extradef})
    endif ()

    add_custom_command(
        OUTPUT  ${_outputfiles}
        DEPENDS ${MAKE_NAMES_DEPENDENCIES} ${NAMES_GENERATOR} ${SCRIPTS_BINDINGS} ${_attrs} ${_tags}
        COMMAND ${PERL_EXECUTABLE} ${NAMES_GENERATOR} --preprocessor "${CODE_GENERATOR_PREPROCESSOR_WITH_LINEMARKERS}" --outputDir ${DERIVED_SOURCES_WEBCORE_DIR} ${_arguments} ${_additionArguments}
        VERBATIM)
endmacro()

macro(MAKE_HASH_TOOLS _source)
    get_filename_component(_name ${_source} NAME_WE)

    if (${_source} STREQUAL "DocTypeStrings")
        set(_hash_tools_h "${DERIVED_SOURCES_WEBCORE_DIR}/HashTools.h")
    else ()
        set(_hash_tools_h "")
    endif ()

    add_custom_command(
        OUTPUT ${DERIVED_SOURCES_WEBCORE_DIR}/${_name}.cpp ${_hash_tools_h}
        MAIN_DEPENDENCY ${_source}.gperf
        COMMAND ${PERL_EXECUTABLE} ${WEBCORE_DIR}/make-hash-tools.pl ${DERIVED_SOURCES_WEBCORE_DIR} ${_source}.gperf ${GPERF_EXECUTABLE}
        VERBATIM)

    unset(_name)
    unset(_hash_tools_h)
endmacro()

macro(WEEX_WRAP_SOURCELIST)
    foreach (_file ${ARGN})
        get_filename_component(_basename ${_file} NAME_WE)
        get_filename_component(_path ${_file} PATH)

        if (NOT _file MATCHES "${DERIVED_SOURCES_WEBCORE_DIR}")
            string(REGEX REPLACE "/" "\\\\\\\\" _sourcegroup "${_path}")
            source_group("${_sourcegroup}" FILES ${_file})
        endif ()
    endforeach ()

    source_group("DerivedSources" REGULAR_EXPRESSION "${DERIVED_SOURCES_WEBCORE_DIR}")
endmacro()

macro(WEEX_FRAMEWORK _target)
    include_directories(SYSTEM ${${_target}_SYSTEM_INCLUDE_DIRECTORIES})
    add_library(${_target} ${${_target}_LIBRARY_TYPE}
        ${${_target}_HEADERS}
        ${${_target}_SOURCES}
    )
    target_include_directories(${_target} PUBLIC "$<BUILD_INTERFACE:${${_target}_INCLUDE_DIRECTORIES}>")
    target_link_libraries(${_target} ${${_target}_LIBRARIES})
    set_target_properties(${_target} PROPERTIES COMPILE_DEFINITIONS "BUILDING_${_target}")
    target_compile_definitions(${_target} PRIVATE "${${_target}_DEFINITIONS}")

    if (${_target}_OUTPUT_NAME)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${${_target}_OUTPUT_NAME})
    endif ()

    if (${_target}_PRE_BUILD_COMMAND)
        add_custom_target(_${_target}_PreBuild COMMAND ${${_target}_PRE_BUILD_COMMAND} VERBATIM)
        add_dependencies(${_target} _${_target}_PreBuild)
    endif ()

    if (${_target}_POST_BUILD_COMMAND)
        add_custom_command(TARGET ${_target} POST_BUILD COMMAND ${${_target}_POST_BUILD_COMMAND} VERBATIM)
    endif ()

#    if (APPLE AND NOT PORT STREQUAL "GTK" AND NOT ${${_target}_LIBRARY_TYPE} MATCHES STATIC)
#        set_target_properties(${_target} PROPERTIES FRAMEWORK TRUE)
#        install(TARGETS ${_target} FRAMEWORK DESTINATION ${LIB_INSTALL_DIR})
#    endif ()
endmacro()

macro(WEEX_CREATE_FORWARDING_HEADER _target_directory _file)
    get_filename_component(_source_path "${CMAKE_SOURCE_DIR}/Source/" ABSOLUTE)
    get_filename_component(_absolute "${_file}" ABSOLUTE)
    get_filename_component(_name "${_file}" NAME)
    set(_target_filename "${_target_directory}/${_name}")

    # Try to make the path in the forwarding header relative to the Source directory
    # so that these forwarding headers are compatible with the ones created by the
    # WebKit2 generate-forwarding-headers script.
    string(REGEX REPLACE "${_source_path}/" "" _relative ${_absolute})

    set(_content "#include \"${_relative}\"\n")

    if (EXISTS "${_target_filename}")
        file(READ "${_target_filename}" _old_content)
    endif ()

    if (NOT _old_content STREQUAL _content)
        file(WRITE "${_target_filename}" "${_content}")
    endif ()
endmacro()

macro(WEEX_CREATE_FORWARDING_HEADERS _framework)
    # On Windows, we copy the entire contents of forwarding headers.
    if (NOT WIN32)
        set(_processing_directories 0)
        set(_processing_files 0)
        set(_target_directory "${DERIVED_SOURCES_DIR}/ForwardingHeaders/${_framework}")

        file(GLOB _files "${_target_directory}/*.h")
        foreach (_file ${_files})
            file(READ "${_file}" _content)
            string(REGEX MATCH "^#include \"([^\"]*)\"" _matched ${_content})
            if (_matched AND NOT EXISTS "${CMAKE_SOURCE_DIR}/Source/${CMAKE_MATCH_1}")
               file(REMOVE "${_file}")
            endif ()
        endforeach ()

        foreach (_currentArg ${ARGN})
            if ("${_currentArg}" STREQUAL "DIRECTORIES")
                set(_processing_directories 1)
                set(_processing_files 0)
            elseif ("${_currentArg}" STREQUAL "FILES")
                set(_processing_directories 0)
                set(_processing_files 1)
            elseif (_processing_directories)
                file(GLOB _files "${_currentArg}/*.h")
                foreach (_file ${_files})
                    WEEX_CREATE_FORWARDING_HEADER(${_target_directory} ${_file})
                endforeach ()
            elseif (_processing_files)
                WEEX_CREATE_FORWARDING_HEADER(${_target_directory} ${_currentArg})
            endif ()
        endforeach ()
    endif ()
endmacro()

# Helper macro which wraps generate-message-receiver.py and generate-message-header.py scripts
#   _output_source is a list name which will contain generated sources.(eg. WebKit2_SOURCES)
#   _input_files are messages.in files to generate.
macro(GENERATE_WEEX2_MESSAGE_SOURCES _output_source _input_files)
    foreach (_file ${_input_files})
        get_filename_component(_name ${_file} NAME_WE)
        add_custom_command(
            OUTPUT ${DERIVED_SOURCES_WEEX2_DIR}/${_name}MessageReceiver.cpp ${DERIVED_SOURCES_WEEX2_DIR}/${_name}Messages.h
            MAIN_DEPENDENCY ${_file}
            DEPENDS ${WEEX2_DIR}/Scripts/webkit/__init__.py
                    ${WEEX2_DIR}/Scripts/webkit/messages.py
                    ${WEEX2_DIR}/Scripts/webkit/model.py
                    ${WEEX2_DIR}/Scripts/webkit/parser.py
            COMMAND ${PYTHON_EXECUTABLE} ${WEEX2_DIR}/Scripts/generate-message-receiver.py ${_file} > ${DERIVED_SOURCES_WEEX2_DIR}/${_name}MessageReceiver.cpp
            COMMAND ${PYTHON_EXECUTABLE} ${WEEX2_DIR}/Scripts/generate-messages-header.py ${_file} > ${DERIVED_SOURCES_WEEX2_DIR}/${_name}Messages.h
            WORKING_DIRECTORY ${WEEX2_DIR}
            VERBATIM)

        list(APPEND ${_output_source} ${DERIVED_SOURCES_WEEX2_DIR}/${_name}MessageReceiver.cpp)
    endforeach ()
endmacro()

macro(MAKE_JS_FILE_ARRAYS _output_cpp _output_h _scripts _scripts_dependencies)
    if (NOT CMAKE_VERSION VERSION_LESS 3.1)
        set(_python_path ${CMAKE_COMMAND} -E env "PYTHONPATH=${JavaScriptCore_SCRIPTS_DIR}")
    elseif (WIN32)
        set(_python_path set "PYTHONPATH=${JavaScriptCore_SCRIPTS_DIR}" &&)
    else ()
        set(_python_path "PYTHONPATH=${JavaScriptCore_SCRIPTS_DIR}")
    endif ()

    add_custom_command(
        OUTPUT ${_output_h} ${_output_cpp}
        MAIN_DEPENDENCY ${WEBCORE_DIR}/Scripts/make-js-file-arrays.py
        DEPENDS ${${_scripts}}
        COMMAND ${_python_path} ${PYTHON_EXECUTABLE} ${WEBCORE_DIR}/Scripts/make-js-file-arrays.py ${_output_h} ${_output_cpp} ${${_scripts}}
        VERBATIM)
    list(APPEND WebCore_DERIVED_SOURCES ${_output_cpp})
    ADD_SOURCE_DEPENDENCIES(${${_scripts_dependencies}} ${_output_h} ${_output_cpp})
endmacro()

# Helper macro for using all-in-one builds
# This macro removes the sources included in the _all_in_one_file from the input _file_list.
# _file_list is a list of source files
# _all_in_one_file is an all-in-one cpp file includes other cpp files
# _result_file_list is the output file list
macro(PROCESS_ALLINONE_FILE _file_list _all_in_one_file _result_file_list _no_compile)
    file(STRINGS ${_all_in_one_file} _all_in_one_file_content)
    set(${_result_file_list} ${_file_list})
    set(_allins "")
    foreach (_line ${_all_in_one_file_content})
        string(REGEX MATCH "^#include [\"<](.*)[\">]" _found ${_line})
        if (_found)
            list(APPEND _allins ${CMAKE_MATCH_1})
        endif ()
    endforeach ()

    foreach (_allin ${_allins})
        if (${_no_compile})
            # For DerivedSources.cpp, we still need the derived sources to be generated, but we do not want them to be compiled
            # individually. We add the header to the result file list so that CMake knows to keep generating the files.
            string(REGEX REPLACE "(.*)\\.cpp" "\\1" _allin_no_ext ${_allin})
            string(REGEX REPLACE ";([^;]*/)${_allin_no_ext}\\.cpp;" ";\\1${_allin_no_ext}.h;" _new_result "${${_result_file_list}};")
        else ()
            string(REGEX REPLACE ";[^;]*/${_allin};" ";" _new_result "${${_result_file_list}};")
        endif ()
        set(${_result_file_list} ${_new_result})
    endforeach ()

endmacro()

# Helper macros for debugging CMake problems.
macro(WEEX_DEBUG_DUMP_COMMANDS)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endmacro()

macro(WEEX_DEBUG_DUMP_VARIABLES)
    set_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
       message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
endmacro()
