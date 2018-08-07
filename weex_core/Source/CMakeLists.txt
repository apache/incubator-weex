cmake_minimum_required(VERSION 3.4.1)
set(WEEXCORE_LIBRARY_NAME weexcore)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#add_compile_options(-std=c++11)
add_compile_options(-fexceptions)

add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)
add_definitions(-DDEBUG=1)

add_definitions(-DRENDER_LOG=0)
add_definitions(-DPERFORMANCE_LOG=0)
add_definitions(-DLAYOUT_LOG=0)
add_definitions(-DJSAPI_LOG=0)
add_definitions(-DDOM_PARSER_LOG=0)

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

  ./core/layout/layout.cpp
  ./core/layout/style.cpp

  ./core/css/css_value_getter.cpp

  ./core/config/core_environment.cpp

  ./core/moniter/render_performance.cpp
  ./core/manager/weex_core_manager.cpp
  ./core/bridge/js_bridge.cpp
  ./core/parser/dom_wson.cpp
   ./core/bridge/bridge.cpp

   ./core/data_render/ast.cc
   ./core/data_render/ast_factory.cc
   ./core/data_render/parser.cc
   ./core/data_render/token.cc
   ./core/data_render/code_generator.cc
   ./core/data_render/exec_state.cc
   ./core/data_render/vm.cc
   ./core/data_render/statement.cc
   ./core/data_render/string_table.cc
   ./core/data_render/tokenizer.cc
   ./core/data_render/expression_parser.cc
   ./core/data_render/vm_mem.cc
   ./core/data_render/object.cc
   ./core/data_render/table.cc
   ./core/data_render/table_factory.cc
   ./core/data_render/vnode/vnode.cc
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
    ./android/bridge/impl/bridge_impl_android.cpp
    ./android/bridge/impl/weexcore_impl_android.cpp
    ./android/bridge/impl/content_box_measurement_impl_android.cpp
    ./android/bridge/impl/jsfunction_impl_android.cpp
    ./android/bridge/impl/native_render_object_utils_impl_android.cpp

    ./android/jsengine/multiprocess/WeexJSConnection.cpp
    ./android/jsengine/multiprocess/WeexProxy.cpp
    ./android/jsengine/multiprocess/ExtendJSApi.cpp


    ./android/jsengine/api/WeexJSCoreApi.cpp
     

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

add_library(${WEEXCORE_LIBRARY_NAME} SHARED ${FINAL_ADD_LIBRARY})
target_include_directories(${WEEXCORE_LIBRARY_NAME} PUBLIC .)
target_link_libraries(${WEEXCORE_LIBRARY_NAME} ${FINAL_TARGET_LINK_LIBRARIES})
