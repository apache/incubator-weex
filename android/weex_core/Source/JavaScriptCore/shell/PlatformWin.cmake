include_directories(./ ${JavaScriptCore_INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${JavaScriptCore_SYSTEM_INCLUDE_DIRECTORIES})
add_library(jscLib SHARED ${JSC_SOURCES})

list(APPEND JSC_LIBRARIES
    Winmm
)

target_link_libraries(jscLib ${JSC_LIBRARIES})

if (${WTF_PLATFORM_WIN_CAIRO})
    add_definitions(-DWIN_CAIRO)
endif ()

set(JSC_SOURCES ${JAVASCRIPTCORE_DIR}/shell/DLLLauncherMain.cpp)
set(JSC_LIBRARIES shlwapi)
add_definitions(-DUSE_CONSOLE_ENTRY_POINT)

set(JSC_OUTPUT_NAME "jsc${DEBUG_SUFFIX}")

add_library(testRegExpLib SHARED ../testRegExp.cpp)
add_executable(testRegExp ${JSC_SOURCES})
set_target_properties(testRegExp PROPERTIES OUTPUT_NAME "testRegExp${DEBUG_SUFFIX}")
target_link_libraries(testRegExp shlwapi)
add_dependencies(testRegExp testRegExpLib)
target_link_libraries(testRegExpLib JavaScriptCore)

add_library(testapiLib SHARED
    ../API/tests/CompareAndSwapTest.cpp
    ../API/tests/CustomGlobalObjectClassTest.c
    ../API/tests/ExecutionTimeLimitTest.cpp
    ../API/tests/FunctionOverridesTest.cpp
    ../API/tests/GlobalContextWithFinalizerTest.cpp
    ../API/tests/JSONParseTest.cpp
    ../API/tests/PingPongStackOverflowTest.cpp
    ../API/tests/testapi.c
    ../API/tests/TypedArrayCTest.cpp
)
set_source_files_properties(../API/tests/CustomGlobalObjectClassTest.c PROPERTIES COMPILE_FLAGS "/TP /MT")
set_source_files_properties(../API/tests/testapi.c PROPERTIES COMPILE_FLAGS "/TP /MT")
add_executable(testapi ${JSC_SOURCES})
set_target_properties(testapi PROPERTIES OUTPUT_NAME "testapi${DEBUG_SUFFIX}")
target_link_libraries(testapi shlwapi)
add_dependencies(testapi testapiLib)
target_link_libraries(testapiLib JavaScriptCore)

file(COPY
    "${JAVASCRIPTCORE_DIR}/API/tests/testapi.js"
    DESTINATION
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
file(COPY
    "${JAVASCRIPTCORE_DIR}/API/tests/testapi-function-overrides.js"
    DESTINATION
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
