cmake_minimum_required(VERSION 3.4.1)
set(WEEXCORE_LIBRARY_NAME weexcore)
set(WEEX_CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

add_definitions(-DRENDER_LOG=0)
add_definitions(-DPERFORMANCE_LOG=0)
add_definitions(-DLAYOUT_LOG=0)
add_definitions(-DJSAPI_LOG=0)
add_definitions(-DDOM_PARSER_LOG=0)

set(LOCAL_LIBRARIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI})

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
   ./core/data_render/class_factory.cc
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
   ./core/data_render/vm.cc
   ./core/data_render/binary_file.cc
   ./core/data_render/class_console.cc
   ./core/data_render/class_function.cc
   ./core/data_render/class_math.cc
   ./core/data_render/class_regex.cc
   ./core/data_render/class_window.cc
   ./core/data_render/exec_state_binary.cc
   ./core/data_render/exec_state_section.cc
   ./core/data_render/op_code.cc
   ./core/data_render/vnode/vnode.cc
   ./core/data_render/vnode/vcomponent.cc
   ./core/data_render/vnode/vnode_exec_env.cc
   ./core/data_render/vnode/vnode_render_manager.cc
   ./core/data_render/vnode/vnode_render_context.cc
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
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/android/jsengine)
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

    ./core/data_render/vnode/android/vcomponent_lifecycle_listener.cc
    ./core/data_render/vnode/android/vnode_on_event_listener.cc
    ./core/data_render/vnode/android/conversion.cc
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
