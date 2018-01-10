cmake_minimum_required(VERSION 3.4.1)

add_compile_options(-std=c++11)
add_compile_options(-fexceptions)

add_definitions(-DOS_ANDROID=1)
add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)
add_definitions(-DDEBUG=1)

## add_subdirectory for subdirectory has a CMakeLists.txt
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/IPC)

## include_directories for include head file
## include_directories(${CMAKE_SOURCE_DIR}/Source/IPC)


add_library(weexjsc SHARED

    ./base/Compatible.cpp
    ./base/fpconv.cpp
    ./WeexCore/platform/android/jniprebuild/jni_load.cc
    ./WeexCore/platform/android/base/jni/android_jni.cpp
    ./WeexCore/platform/android/base/jni/scoped_java_ref.cpp
    ./WeexCore/platform/android/base/base64/base64.cpp
    ./WeexCore/platform/android/base/base64/modp_base64/modp_b64.cc
    ./WeexCore/platform/android/base/string/ScopedJString.cpp
    ./WeexCore/platform/android/base/string/ScopedJStringUTF8.cpp

    ./WeexCore/platform/android/jsengine/multiprocess/WeexJSConnection.cpp
    ./WeexCore/platform/android/jsengine/multiprocess/WeexProxy.cpp
    ./WeexCore/platform/android/jsengine/multiprocess/ExtendJSApi.cpp


    ./WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.cpp
    ./WeexCore/platform/android/bridge/impl/WeexCore_Impl_Android.cpp
    ./WeexCore/platform/android/bridge/impl/MeasureMode_Impl_Android.cpp
    ./WeexCore/platform/android/bridge/impl/ContentBoxMeasurement_Impl_Android.cpp
    ./WeexCore/platform/android/bridge/impl/WXSDKInstance_Impl_Android.cpp
    ./WeexCore/platform/android/bridge/impl/WXComponent_Impl_Android.cpp
    ./WeexCore/platform/android/bridge/impl/MeasureSize_Impl_Android.cpp

    ./WeexCore/render/manager/RenderManager.cpp
    ./WeexCore/render/page/RenderPage.cpp
    ./WeexCore/render/node/RenderObject.cpp

    ./WeexCore/render/node/RenderRoot.cpp
    ./WeexCore/render/node/RenderText.cpp
    ./WeexCore/render/node/RenderCell.cpp

    ./WeexCore/render/action/RenderActionAddElement.cpp
    ./WeexCore/render/action/RenderActionRemoveElement.cpp
    ./WeexCore/render/action/RenderActionMoveElement.cpp
    ./WeexCore/render/action/RenderActionCreateBody.cpp
    ./WeexCore/render/action/RenderActionCreateFinish.cpp
    ./WeexCore/render/action/RenderActionLayout.cpp
    ./WeexCore/render/action/RenderActionUpdateAttr.cpp
    ./WeexCore/render/action/RenderActionUpdateStyle.cpp

    ./WeexCore/parser/DomParser.cpp

    ./WeexCore/layout/WXCoreLayout.cpp
    ./WeexCore/layout/WXCoreStyle.cpp

    ./WeexCore/css/CssValueGetter.cpp

    ./WeexCore/env/CoreEnvironment.cpp
    ./WeexCore/env/RenderPerformance.cpp
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
