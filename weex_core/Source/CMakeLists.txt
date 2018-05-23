cmake_minimum_required(VERSION 3.4.1)

add_compile_options(-std=c++11)
add_compile_options(-fexceptions)

add_definitions(-DOS_ANDROID=1)
add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)
add_definitions(-DDEBUG=1)

add_definitions(-DRENDER_LOG=0)
add_definitions(-DPERFORMANCE_LOG=0)
add_definitions(-DLAYOUT_LOG=0)
add_definitions(-DJSAPI_LOG=0)
add_definitions(-DDOM_PARSER_LOG=0)

## add_subdirectory for subdirectory has a CMakeLists.txt
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/IPC)

## include_directories for include head file
## include_directories(${CMAKE_SOURCE_DIR}/Source/IPC)

include_directories("include")

add_library(weexjsc SHARED

    ./android/jniprebuild/jni_load.cc
    ./android/base/jni/android_jni.cpp
    ./android/base/jni/scoped_java_ref.cpp
    ./android/base/base64/base64.cpp
    ./android/base/base64/modp_base64/modp_b64.cc
    ./android/base/string/scoped_jstring.cpp
    ./android/base/string/scoped_jstring_utf8.cpp
    ./android/bridge/impl/jsc_bridge_impl_android.cpp
    ./android/bridge/impl/bridge_impl_android.cpp
    ./android/bridge/impl/weexcore_impl_android.cpp
    ./android/bridge/impl/measure_mode_impl_android.cpp
    ./android/bridge/impl/content_box_measurement_impl_android.cpp
    ./android/bridge/impl/jsfunction_impl_android.cpp
    ./android/bridge/impl/native_render_object_utils_impl_android.cpp

    ./android/jsengine/multiprocess/WeexJSConnection.cpp
    ./android/jsengine/multiprocess/WeexProxy.cpp
    ./android/jsengine/multiprocess/ExtendJSApi.cpp

    ./core/api/WeexJSCoreApi.cpp

    ./core/render/manager/render_manager.cpp
    ./core/render/page/render_page.cpp
    ./core/render/node/render_object.cpp
    ./core/render/node/factory/render_creator.cpp
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

    ./core/parser/dom_parser.cpp

    ./core/layout/layout.cpp
    ./core/layout/style.cpp

    ./core/css/css_value_getter.cpp

    ./core/config/core_environment.cpp
    ./core/moniter/render_performance.cpp
    ./core/bridge/js_bridge.cpp
    ./core/manager/wx_core_manager.cpp

        )

 #    set(weexjsc_LIBRARY_TYPE SHARED)

 #   set_target_properties(weexjsc
 #   PROPERTIES
 #   LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}//weexjsc_version_script.txt -Wl,--no-undefined -Wl,-soname,libweexjsc.so"
 #   )

 #   add_custom_command(
 #       TARGET weexjsc
 #       POST_BUILD
 #       COMMAND ${CMAKE_STRIP} $<TARGET_FILE:weexjsc> -o ${CMAKE_SOURCE_DIR}/libweexjsc.so
 #       VERBATIM
 #   )

    target_include_directories(weexjsc
                               PUBLIC
                               .)

    target_link_libraries(weexjsc 
                          android
                          IPC
                          log
                          z)
