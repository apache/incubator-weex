set(JSC_SOURCES
    ../jsc.cpp
)

set(JSC_LIBRARIES
    ${CMAKE_DL_LIBS}
    JavaScriptCore${DEBUG_SUFFIX}
    WTF${DEBUG_SUFFIX}
)

if (WTF_OS_MAC_OS_X)
    list(APPEND JSC_LIBRARIES edit)
endif ()

if ("${JavaScriptCore_LIBRARY_TYPE}" MATCHES "STATIC")
    add_definitions(-DSTATICALLY_LINKED_WITH_JavaScriptCore)
endif ()

WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS()

WEBKIT_WRAP_SOURCELIST(${JSC_SOURCES})
include_directories(./ ${JavaScriptCore_INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${JavaScriptCore_SYSTEM_INCLUDE_DIRECTORIES})
#add_library(jscshell SHARED ${JSC_SOURCES})
#target_link_libraries(jscshell ${JSC_LIBRARIES} log)
#set_target_properties(jscshell
#PROPERTIES
#LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../jsc_version_script.txt -Wl,--no-undefined"
#)
#add_custom_command(
#    TARGET jscshell
#    POST_BUILD
#    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:jscshell> -o ${CMAKE_SOURCE_DIR}/libjscshell.so
#    VERBATIM
#)
#
#add_executable(jsc ../jsc_stub.cpp)
#target_link_libraries(jsc jscshell)
#add_custom_command(
#    TARGET jsc
#    POST_BUILD
#    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:jsc> -o ${CMAKE_SOURCE_DIR}/jsc
#    VERBATIM
#)

if (JSC_OUTPUT_NAME)
    set_target_properties(jsc PROPERTIES OUTPUT_NAME ${JSC_OUTPUT_NAME})
endif ()

if (JSC_LINK_FLAGS)
    ADD_TARGET_PROPERTIES(jsc LINK_FLAGS "${JSC_LINK_FLAGS}")
endif ()

if (SHOULD_INSTALL_JS_SHELL)
    install(TARGETS jsc DESTINATION "${LIBEXEC_INSTALL_DIR}")
endif ()

if (TARGET jscLib)
    add_dependencies(jsc jscLib)
endif ()

if (NOT WIN32)
    set(TESTB3_SOURCES
        ../b3/testb3.cpp
    )

    set(TESTAIR_SOURCES
        ../b3/air/testair.cpp
    )

    #add_executable(testb3 ${TESTB3_SOURCES})
    #target_link_libraries(testb3 ${JSC_LIBRARIES})

    #add_executable(testair ${TESTAIR_SOURCES})
    #target_link_libraries(testair ${JSC_LIBRARIES})

endif ()
