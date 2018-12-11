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

message("${ANDROID_STL} ")
if ("${ANDROID_STL}" STREQUAL "c++_static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
elseif ("${ANDROID_STL}" STREQUAL "gnustl_static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y")
endif ()
add_compile_options(-fexceptions)


add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)
add_definitions(-DDEBUG=1)
add_definitions(-DOS_ANDROID=1)

add_definitions(-DRENDER_LOG=0)
add_definitions(-DPERFORMANCE_LOG=0)
add_definitions(-DLAYOUT_LOG=0)
add_definitions(-DJSAPI_LOG=0)
add_definitions(-DDOM_PARSER_LOG=0)

set(COMMON_SRCS
  ./base/time_point.cc
  ./base/thread/thread_impl_posix.cc
  ./base/thread/thread_impl_android.cc
  ./base/message_loop/message_loop.cc
  ./base/message_loop/message_pump_posix.cc
  ./base/message_loop/message_pump_android.cc
  ./base/third_party/icu/icu_utf.cpp
  ./core/render/manager/render_manager.cpp
  ./core/render/page/render_page.cpp

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

  ./core/layout/layout.cpp
  ./core/layout/style.cpp

  ./core/css/css_value_getter.cpp

  ./core/config/core_environment.cpp

  ./core/moniter/render_performance.cpp
  ./core/bridge/platform/core_side_in_platform.cpp
  ./core/bridge/script/core_side_in_script.cpp
  ./core/parser/dom_wson.cpp

   ./core/data_render/ast.cc
   ./core/data_render/ast_builder.cc
   ./core/data_render/ast_factory.cc
   ./core/data_render/class.cc
   ./core/data_render/class_array.cc
   ./core/data_render/class_json.cc
   ./core/data_render/class_string.cc
   ./core/data_render/class_regex.cc
   ./core/data_render/class_console.cc
   ./core/data_render/class_window.cc
   ./core/data_render/class_math.cc
   ./core/data_render/class_factory.cc
   ./core/data_render/class_function.cc
   ./core/data_render/class_object.cc
   ./core/data_render/code_generator.cc
   ./core/data_render/exec_state.cc
   ./core/data_render/object.cc
   ./core/data_render/parser.cc
   ./core/data_render/rax_jsx_ast.cc
   ./core/data_render/js_common_function.cc
   ./core/data_render/rax_parser.cc
   ./core/data_render/rax_parser_builder.cc
   ./core/data_render/rax_parser_context.cc
   ./core/data_render/rax_parser_scope.cc
   ./core/data_render/rax_parser_statistics.cc
   ./core/data_render/rax_source_locator.cc
   ./core/data_render/statement.cc
   ./core/data_render/string_table.cc
   ./core/data_render/table.cc
   ./core/data_render/token.cc
   ./core/data_render/tokenizer.cc
   ./core/data_render/op_code.cc
   ./core/data_render/vm.cc
   ./core/data_render/exec_state_binary.cc
   ./core/data_render/exec_state_section.cc
   ./core/data_render/vnode/vnode.cc
   ./core/data_render/vnode/vcomponent.cc
   ./core/data_render/vnode/vnode_exec_env.cc
   ./core/data_render/vnode/vnode_render_manager.cc
   ./core/data_render/vnode/vnode_render_context.cc
        )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/wson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wson)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/json11)

SET(FINAL_ADD_LIBRARY
        ${COMMON_SRCS}
)
SET(FINAL_TARGET_LINK_LIBRARIES wson json11)

if (ANDROID)
  ## add_subdirectory for subdirectory has a CMakeLists.txt
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/IPC)
  ## include_directories for include head file
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/IPC)

  set (ANDROID_SRCS
    ./android/jniprebuild/jni_load.cc
    ./android/base/jni/android_jni.cpp
    ./android/base/jni/jbytearray_ref.cpp
    ./android/base/jni/scoped_java_ref.cpp
    ./android/base/base64/base64.cpp
    ./android/base/base64/modp_base64/modp_b64.cc
    ./android/base/string/scoped_jstring.cpp
    ./android/base/string/scoped_jstring_utf8.cpp
    ./android/base/string/jstring_cache.cpp
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
    ./android/utils/IPCStringResult.cpp
    ./android/wrap/log_utils.cpp
    ./android/wrap/hash_set.cpp
    ./android/wrap/wx_map.cpp
    ./android/wrap/wx_js_object.cpp
    ./android/wrap/wx_bridge.cpp
    ./android/wrap/wx_params.cpp
    ./android/base/jni_type.cpp

    ./android/jsengine/multiprocess/WeexJSConnection.cpp
    ./android/jsengine/multiprocess/ExtendJSApi.cpp

  )
  SET(FINAL_ADD_LIBRARY
          ${FINAL_ADD_LIBRARY}
          ${ANDROID_SRCS}
          )

  SET(FINAL_TARGET_LINK_LIBRARIES
          ${FINAL_TARGET_LINK_LIBRARIES}
          wson
          android
          weexipc
          log
          z
          )

endif (ANDROID)

##if(iOS)
  add_definitions(-DTARGET_OS_IPHONE=1)

##endif(iOS)


add_library(${WEEXCORE_LIBRARY_NAME} SHARED ${FINAL_ADD_LIBRARY})
target_include_directories(${WEEXCORE_LIBRARY_NAME} PUBLIC .)
target_link_libraries(${WEEXCORE_LIBRARY_NAME} ${FINAL_TARGET_LINK_LIBRARIES})
