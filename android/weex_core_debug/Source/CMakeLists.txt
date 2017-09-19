cmake_minimum_required(VERSION 3.4.1)

add_compile_options(-std=c++11)
add_compile_options(-fexceptions)

add_definitions(-DOS_ANDROID=1)
add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)

## add_subdirectory for subdirectory has a CMakeLists.txt
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/IPC)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/yoga)


## include_directories for include head file
## include_directories(${CMAKE_SOURCE_DIR}/Source/IPC)


add_library(weexjsc SHARED

    ./prebuild/jni_load.cc
    ./base/android/android_jni.cpp
    ./base/android/scoped_java_ref.cpp

    ./base/Compatible.cpp
    ./base/base64/base64.cpp
    ./base/base64/modp_base64/modp_b64.cc
    ./base/ScopedJString.cpp
    ./base/ScopedJStringUTF8.cpp
    ./base/fpconv.cpp
    #./base/timer/timer.cpp
    #./base/timer/time_utils.cpp
    #./base/timer/timer_heap.cpp
    #./base/threading/message_loop.cpp
    #./base/threading/message_pump_android.cpp
    #./base/threading/message_pump_posix.cpp
    #./base/threading/thread.cpp


    ./WeexCore/layout/WXStyleUtil.cpp
    ./WeexCore/platform/android/WeexJSConnection.cpp
    ./WeexCore/platform/android/BridgeAndroid.cpp
    ./WeexCore/platform/android/ExtendJSApi.cpp
    ./WeexCore/platform/android/WeexProxy.cpp
    ./WeexCore/platform/android/WeexCore.cpp
    ./WeexCore/render/action/RenderAction.cpp
    ./WeexCore/render/RenderManager.cpp
    ./WeexCore/render/RenderObject.cpp
    ./WeexCore/render/RenderPage.cpp
    ./rapidjson/weexjsontools.cpp
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
                                yoga
                                z)
#                                ${CMAKE_SOURCE_DIR}/libyogacore.so)

#    target_link_libraries(weexjsc ${CMAKE_SOURCE_DIR}/libyogacore.so)
    
