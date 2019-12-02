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
cmake_minimum_required(VERSION 3.4.1)
set(WEEXCORE_LIBRARY_NAME weexcore)
set(WEEX_CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

message("${ANDROID_STL} ")
if ("${ANDROID_STL}" STREQUAL "c++_static" OR "${ANDROID_STL}" STREQUAL "c++_shared")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
elseif ("${ANDROID_STL}" STREQUAL "gnustl_static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y")
endif ()
add_compile_options(-fexceptions)
add_compile_options(-fstack-protector)
add_compile_options(-funwind-tables)
add_compile_options(-fno-omit-frame-pointer)

add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)

add_definitions(-DRENDER_LOG=0)
add_definitions(-DPERFORMANCE_LOG=0)
add_definitions(-DLAYOUT_LOG=0)
add_definitions(-DJSAPI_LOG=0)
add_definitions(-DDOM_PARSER_LOG=0)
add_definitions(-DDEBUG=1)

message("ANDROID_PROJECT_DIR:"${ANDROID_PROJECT_DIR})
set(LOCAL_LIBRARIES_DIR ${ANDROID_PROJECT_DIR}/src/main/jniLibs/${ANDROID_ABI})

if ("${ANDROID_ABI}" STREQUAL "x86")
    # todo
else ()
    if ("${ENABLE_ASAN}" STREQUAL "true")
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g3 -fPIC")
    endif ()
endif ()

add_compile_options(-fexceptions)

set(COMMON_SRCS
  ./core/render/manager/render_manager.cpp
  ./core/render/page/render_page.cpp
  ./core/render/page/render_page_base.cpp
  ./core/render/page/render_page_custom.cpp
  ./core/render/target/render_target.cpp

  ./core/render/node/render_object.cpp
  ./core/render/node/factory/render_creator.cpp
  ./core/render/node/render_appbar.cpp
  ./core/render/node/render_list.cpp
  ./core/render/node/render_mask.cpp
  ./core/render/node/render_scroller.cpp
  ./core/render/node/render_text.cpp

  ./core/render/action/render_action_add_event.cpp
  ./core/render/action/render_action_remove_event.cpp
  ./core/render/action/render_action_add_element.cpp
  ./core/render/action/render_action_remove_element.cpp
  ./core/render/action/render_action_move_element.cpp
  ./core/render/action/render_action_createbody.cpp
  ./core/render/action/render_action_createfinish.cpp
  ./core/render/action/render_action_appendtree_createfinish.cpp
  ./core/render/action/render_action_layout.cpp
  ./core/render/action/render_action_update_attr.cpp
  ./core/render/action/render_action_update_style.cpp
  ./core/render/action/render_action_render_success.cpp
  ./core/render/action/render_action_trigger_vsync.cpp
  ./core/json/JsonRenderManager.cpp
  ./core/json/JsonPage.cpp
  ./core/render/action/render_action_add_child_to_richtext.cpp
  ./core/render/action/render_action_remove_child_from_richtext.cpp
  ./core/render/action/render_action_update_richtext_child_style.cpp
  ./core/render/action/render_action_update_richtext_child_attr.cpp
  ./core/layout/layout.cpp
  ./core/layout/style.cpp

  ./core/css/css_value_getter.cpp

  ./core/config/core_environment.cpp

  ./core/moniter/render_performance.cpp
  ./core/bridge/platform/core_side_in_platform.cpp
  ./core/bridge/script/core_side_in_script.cpp
  ./core/parser/dom_wson.cpp
  ./core/parser/action_args_check.cpp

   ./core/network/http_module.cc
        )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/base)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/wson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wson)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/json11)
#link_directories(${LOCAL_LIBRARIES_DIR})

SET(FINAL_ADD_LIBRARY
        ${COMMON_SRCS}
)
SET(FINAL_TARGET_LINK_LIBRARIES wson json11)

if(ANDROID)
  add_definitions(-DOS_ANDROID=1)
  ## add_subdirectory for subdirectory has a CMakeLists.txt
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/IPC)

  if (DEFINED BUILD_RUNTIME_API)
    message("Variable BUILD_RUNTIME_API is defined with value: ${BUILD_RUNTIME_API}")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/js_runtime)
  else()
    message("Variable BUILD_RUNTIME_API is not defined")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/android/jsengine)
  endif()

  ## include_directories for include head file
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/IPC)

  set (ANDROID_SRCS
    ./android/weex_extend_js_api.h
    ./android/weex_extend_js_api.cpp
    ./android/base/string/scoped_jstring_utf8.cpp
    ./android/base/string/jstring_cache.cpp
    ./android/base/string/scoped_jstring.cpp
    ./android/wrap/content_box_measurement_impl_android.cpp
    ./android/wrap/wx_debug_js_bridge.cpp
    ./android/wrap/native_render_object_utils_impl_android.cpp
    ./android/bridge/platform/android_side.cpp
    ./android/bridge/platform/android_bridge.cpp
    ./android/bridge/script/script_side_in_multi_so.cpp
    ./android/bridge/script/script_side_in_multi_process.cpp
    ./android/bridge/script_bridge_in_multi_so.cpp
    ./android/bridge/script_bridge_in_multi_process.cpp

    ./android/bridge/multi_process_and_so_initializer.cpp
    ./android/bridge/multi_so_initializer.cpp
    ./android/wrap/wml_bridge.cpp
    ./android/utils/cache_utils.cpp
    ./android/utils/params_utils.cpp
    ./android/utils/so_utils.cpp
    ./android/utils/ipc_string_result.cpp
    ./android/wrap/log_utils.cpp
    ./android/wrap/hash_set.cpp
    ./android/wrap/wx_map.cpp
    ./android/wrap/wx_js_object.cpp
    ./android/wrap/wx_bridge.cpp
    ./android/wrap/wx_params.cpp
    ./android/multiprocess/weex_js_connection.cpp
    ./android/utils/jni_load.cc
    ./core/bridge/eagle_bridge.cpp
    ./core/network/android/default_request_handler.cc
  )
  SET(FINAL_ADD_LIBRARY
          ${FINAL_ADD_LIBRARY}
          ${ANDROID_SRCS}
          )

  SET(FINAL_TARGET_LINK_LIBRARIES
          ${FINAL_TARGET_LINK_LIBRARIES}
          weexbase
          wson
          android
          weexipc
          #weexjss
          log
          c
          z
          )
endif(ANDROID)

#if(iOS)
#add_definitions(-DTARGET_OS_IPHONE=1)
#endif(iOS)


add_library(${WEEXCORE_LIBRARY_NAME} SHARED ${FINAL_ADD_LIBRARY})
target_include_directories(${WEEXCORE_LIBRARY_NAME} PUBLIC .)
target_link_libraries(${WEEXCORE_LIBRARY_NAME} ${FINAL_TARGET_LINK_LIBRARIES})
# Enable it for smaller so.
# set_target_properties(${WEEXCORE_LIBRARY_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
