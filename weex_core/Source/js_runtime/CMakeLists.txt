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

cmake_minimum_required(VERSION 3.4.1)

if(DEFINED ENV{WEEX_CORE_SOURCE_DIR})
    message('WEEX_CORE_SOURCE_DIR has set')
else()
#    message("rewrite WEEX_CORE_SOURCE_DIR before ${WEEX_CORE_SOURCE_DIR}")
#    set(WEEX_CORE_SOURCE_DIR ../)
endif()


if(DEFINED ENV{ANDROID_PLATFORM})
    message('ANDROID_PLATFORM has set')
else()
    message('rewrite ANDROID_PLATFORM')
    set(ANDROID_PLATFORM " ")
endif()


set(PORT JSCOnly)
add_definitions(-DPRINT_LOG_CACHEFILE=0)
add_definitions(-DOS_ANDROID)

# log switch for debug in file "js_runtime/utils/log_utils.h"
#add_definitions(-DLOG_JS_RUNTIME_DEBUG=1)
#add_definitions(-DLOG_WEEX_RUNTIME_DEBUG=1)
#add_definitions(-DLOG_CONVERSION_DEBUG=0)
#add_definitions(-DLOG_WEEX_BINDING_DEBUG=1)
add_definitions(-DLOG_JS_DEBUG=0)
add_definitions(-DUSE_JS_RUNTIME=1)


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
include_directories(${WEEX_CORE_SOURCE_DIR}/include/JSCHeaderNew)






set(JS_RUNTIME_API_SRCS
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/runtime_vm.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/runtime_context_android.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/runtime_values.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/binding_macro.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/engine_context_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/jsc_utils.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/runtime_object_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/runtime_values_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/vm_jsc.cc
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/jsc/binding_macro_jsc.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/js_runtime_conversion.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/runtime/js_runtime_conversion.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/utils/log_utils.h
        )
set(WEEX_BINDING
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_instance_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_instance_binding.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_global_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_global_binding.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/app_context_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/app_context_binding.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_console_binding.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_console_binding.h

        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/wson_for_runtime.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/wson_for_runtime.h

        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_binding_utils.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/binding/weex_binding_utils.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/weex_conversion_utils.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/weex_conversion_utils.h

        )

set(JSON_SRC
        ${WEEX_CORE_SOURCE_DIR}/third_party/json11/json11.hpp
        ${WEEX_CORE_SOURCE_DIR}/third_party/json11/json11.cc
        )
set(WEEX_RUNTIME_V2
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_runtime_v2.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_runtime_v2.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_object_holder_v2.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_object_holder_v2.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_global_object_v2.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/object/weex_global_object_v2.h
        )




set(${WEEXJSSERVER_NAME}_SOURCES
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/weex_ipc_client.h
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/weex_ipc_client.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/weex_ipc_server.h
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/weex_ipc_server.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/weex_jsc_utils.h
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/utils/weex_jsc_utils.cpp

        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/bridge/script/script_side_in_simple.cpp
        ${WEEX_CORE_SOURCE_DIR}/js_runtime/weex/bridge/script/script_side_in_queue.cpp

        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/bridge/script/script_bridge_in_multi_so.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/bridge/script/script_bridge_in_multi_process.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/bridge/script/core_side_in_multi_so.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/bridge/script/core_side_in_multi_process.cpp
        #${WEEX_CORE_SOURCE_DIR}/android/jsengine/bridge/script/script_side_in_simple.cpp
        #${WEEX_CORE_SOURCE_DIR}/android/jsengine/bridge/script/script_side_in_queue.cpp

        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/object/args/exe_js_args.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/object/args/init_framework_args.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/object/args.h
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/object/args.cpp
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/object/weex_env.h
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/object/weex_env.cpp

        weex/task/impl/init_framework_task.cpp
        weex/task/impl/create_app_context_task.cpp
        weex/task/impl/create_instance_task.cpp
        weex/task/impl/call_js_on_app_context_task.cpp
        weex/task/impl/destory_app_context_task.cpp
        weex/task/impl/destory_instance_task.cpp
        weex/task/impl/exe_js_on_app_with_result.cpp
        weex/task/impl/update_global_config_task.cpp
        weex/task/impl/ctime_callback_task.cpp
        weex/task/impl/exe_js_services_task.cpp
        weex/task/impl/exe_js_on_instance_task.cpp
        weex/task/impl/exe_js_task.cpp
        weex/task/impl/take_heap_snapshot.cpp
        weex/task/impl/native_timer_task.cpp
        weex/task/impl/update_init_framework_params_task.cpp
        weex/task/weex_task.cpp
        weex/task/weex_task_queue.cpp
        weex/task/timer_task.cpp
        weex/task/timer_queue.cpp
        weex/task/back_to_weex_core_queue.cpp

      #  ${JS_RUNTIME_API_SRCS}
        ${JS_RUNTIME_API_SRCS}
        ${DOM_API_SRCS}
        ${WEEX_BINDING}
        ${JSON_SRC}
        ${WEEX_RUNTIME_V2}
)
link_directories(${LOCAL_LIBRARIES_DIR})
message("LOCAL_LIBRARIES_DIR:"${LOCAL_LIBRARIES_DIR})
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "toolchain_exelinkflags" FORCE)
SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -shared")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -fno-omit-frame-pointer")# -fsanitize=address -g3")

set(${WEEXJSSERVER_NAME}_LIBRARY_TYPE SHARED)

message("${LOCAL_LIBRARIES_DIR}")
set(${WEEXJSSERVER_NAME}_LIBRARIES
weexbase
weexipc
wson
log
z
-lc
-ldl
-pthread
)

if (DEFINED BUILD_RUNTIME_API)
    set(${WEEXJSSERVER_NAME}_LIBRARIES
        ${${WEEXJSSERVER_NAME}_LIBRARIES}
        c++_shared
        jsc
    )
else()
    set(${WEEXJSSERVER_NAME}_LIBRARIES
        ${${WEEXJSSERVER_NAME}_LIBRARIES}
        JavaScriptCore
        WTF
    )
endif()

add_library(${WEEXJSSERVER_NAME} SHARED
            ${${WEEXJSSERVER_NAME}_SOURCES} )


target_link_libraries(${WEEXJSSERVER_NAME}
                      ${${WEEXJSSERVER_NAME}_LIBRARIES})
set_target_properties(${WEEXJSSERVER_NAME} PROPERTIES NO_SONAME TRUE)

set_target_properties(${WEEXJSSERVER_NAME}
PROPERTIES
LINK_FLAGS
"-Wl,--version-script=${WEEX_CORE_SOURCE_DIR}/android/jsengine/weexjsserver_version_script.txt -Wl,--no-undefined -Wl,-soname,libweexjss${SERVERSUFFIX}.so"
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
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/weex_js_server_main_stub.cpp)

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
        ${WEEX_CORE_SOURCE_DIR}/android/jsengine/weex_js_server_main_stub.cpp)

target_link_libraries(${WEEXJSSERVERSTUB_NAME_BELOW_L}
${WEEXJSSERVER_NAME}
)

if (APPLE)
set_target_properties(${WEEXJSSERVERSTUB_NAME_BELOW_L}
PROPERTIES LINK_FLAGS
"-Wl,--entry=_start"
)
endif()