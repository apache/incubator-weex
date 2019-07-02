#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

#cmake_minimum_required(VERSION 3.3)
#if (POLICY CMP0042)
#    cmake_policy(SET CMP0042 NEW)
#endif ()

set(PORT JSCOnly)
add_definitions(-DPRINT_LOG_CACHEFILE=0)
add_definitions(-DOS_ANDROID)
#add_definitions(-DNDEBUG)
add_definitions(-DUSE_SYSTEM_MALLOC=1)
if (${ANDROID_PLATFORM} STREQUAL "android-21")
else ()
    add_definitions(-D_POSIX_THREAD_KEYS_MAX=128)                              
    add_definitions(-DPTHREAD_KEYS_MAX=_POSIX_THREAD_KEYS_MAX)
endif ()

if ("${ANDROID_ABI}" STREQUAL "armeabi")
    add_definitions(-DWTF_CPU_ARM)
    set(WTF_CPU_ARM 1)

    if ("${ANDROID_TOOLCHAIN}" STREQUAL clang)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a")
    endif ()
elseif ("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
    add_definitions(-DWTF_CPU_ARM)
    set(WTF_CPU_ARM 1)
elseif("${ANDROID_ABI}" STREQUAL "arm64-v8a")
    add_definitions(-DWTF_CPU_ARM64)
    set(WTF_CPU_ARM64 1)
else ()
    add_definitions(-DWTF_CPU_X86)
    set(WTF_CPU_X86_64 1)
endif ()

set(USE_SYSTEM_MALLOC 1)
set(WEEXJSSERVER_NAME weexjss)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${WEEX_CORE_SOURCE_DIR})
include_directories(${WEEX_CORE_SOURCE_DIR}/include)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/API)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/ForwardingHeaders)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/assembler)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/bytecode)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/dfg)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/disassembler)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/domjit)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/runtime)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/heap)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/jit)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/interpreter)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/parser)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/profiler)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JavaScriptCore/yarr)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/wtf/icu)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/wtf/text)
include_directories(${WEEX_CORE_SOURCE_DIR}/include/wtf)
#include_directories(${WEEX_CORE_SOURCE_DIR}/android/jsengine/dependence/WTF/)
#include_directories(${WEEX_CORE_SOURCE_DIR}/android/jsengine/dependence/icu)

set(${WEEXJSSERVER_NAME}_SOURCES
        weex_ipc_client.h
        weex_ipc_client.cpp
        weex_ipc_server.h
        weex_ipc_server.cpp
        weex_jsc_utils.h
        weex_jsc_utils.cpp
        weex_runtime.h
        weex_runtime.cpp

        bridge/script/script_bridge_in_multi_so.cpp
        bridge/script/script_bridge_in_multi_process.cpp
        bridge/script/core_side_in_multi_so.cpp
        bridge/script/core_side_in_multi_process.cpp
        bridge/script/script_side_in_simple.cpp
        bridge/script/script_side_in_queue.cpp

        object/args/exe_js_args.cpp
        object/args/init_framework_args.cpp
        object/args.h
        object/args.cpp
        object/weex_console_object.h
        object/weex_console_object.cpp
        object/weex_env.h
        object/weex_env.cpp
        object/weex_global_object.h
        object/weex_global_object.cpp
        object/weex_object_holder.h
        object/weex_object_holder.cpp
        object/weex_simple_object.h
        object/weex_simple_object.cpp

        task/impl/init_framework_task.cpp
        task/impl/create_app_context_task.cpp
        task/impl/create_instance_task.cpp
        task/impl/call_js_on_app_context_task.cpp
        task/impl/destory_app_context_task.cpp
        task/impl/destory_instance_task.cpp
        task/impl/exe_js_on_app_with_result.cpp
        task/impl/update_global_config_task.cpp
        task/impl/ctime_callback_task.cpp
        task/impl/exe_js_services_task.cpp
        task/impl/exe_js_on_instance_task.cpp
        task/impl/exe_js_task.cpp
        task/impl/take_heap_snapshot.cpp
        task/impl/native_timer_task.cpp
        task/impl/update_init_framework_params_task.cpp
        task/weex_task.cpp
        task/weex_task_queue.cpp
        task/timer_task.cpp
        task/timer_queue.cpp
        task/back_to_weex_core_queue.cpp

        wson/wson_jsc.h
        wson/wson_jsc.cpp
)
link_directories(${LOCAL_LIBRARIES_DIR})
message("LOCAL_LIBRARIES_DIR:"${LOCAL_LIBRARIES_DIR})
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "toolchain_exelinkflags" FORCE)
SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -shared")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -fno-omit-frame-pointer")# -fsanitize=address -g3")

set(${WEEXJSSERVER_NAME}_LIBRARY_TYPE SHARED)

set(${WEEXJSSERVER_NAME}_LIBRARIES
weexbase
weexipc
wson
JavaScriptCore
WTF
log
z
-lc
-ldl
-pthread
)

add_library(${WEEXJSSERVER_NAME} SHARED
            ${${WEEXJSSERVER_NAME}_SOURCES})


target_link_libraries(${WEEXJSSERVER_NAME}
                      ${${WEEXJSSERVER_NAME}_LIBRARIES})
set_target_properties(${WEEXJSSERVER_NAME} PROPERTIES NO_SONAME TRUE)

set_target_properties(${WEEXJSSERVER_NAME}
PROPERTIES
LINK_FLAGS
"-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}//weexjsserver_version_script.txt -Wl,--no-undefined -Wl,-soname,libweexjss${SERVERSUFFIX}.so"
)

#add_custom_command(
#    TARGET ${WEEXJSSERVER_NAME}
#    POST_BUILD
#    COMMAND cp $<TARGET_FILE:${WEEXJSSERVER_NAME}> ${CMAKE_SOURCE_DIR}/libweexjss${SERVERSUFFIX}.so
#    VERBATIM
#)

#add_custom_command(
#    TARGET ${WEEXJSSERVERSTUB_NAME}
#    POST_BUILD
#    COMMAND cp $<TARGET_FILE:${WEEXJSSERVERSTUB_NAME}> ${CMAKE_SOURCE_DIR}/libweexjsb${SERVERSUFFIX}.so
#    VERBATIM
#)

# Compile libweexjsb.so
set(WEEXJSSERVERSTUB_NAME libweexjsb.so)
add_executable(${WEEXJSSERVERSTUB_NAME}
               weex_js_server_main_stub.cpp)

set_target_properties(${WEEXJSSERVERSTUB_NAME} 
PROPERTIES                                                                        
LINK_FLAGS                                                                        
"-pie -fPIE"
)

target_link_libraries(${WEEXJSSERVERSTUB_NAME}
${WEEXJSSERVER_NAME}
)

if (APPLE)
set_target_properties(${WEEXJSSERVERSTUB_NAME}
PROPERTIES LINK_FLAGS
"-Wl,--entry=_start"
)
endif()

# Compile libweexjst.so
set(WEEXJSSERVERSTUB_NAME_BELOW_L libweexjst.so)
add_executable(${WEEXJSSERVERSTUB_NAME_BELOW_L}
               weex_js_server_main_stub.cpp)

target_link_libraries(${WEEXJSSERVERSTUB_NAME_BELOW_L}
${WEEXJSSERVER_NAME}
)

if (APPLE)
set_target_properties(${WEEXJSSERVERSTUB_NAME_BELOW_L}
PROPERTIES LINK_FLAGS
"-Wl,--entry=_start"
)
endif()