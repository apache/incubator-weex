cmake_minimum_required(VERSION 3.4.1)

add_compile_options(-std=c++11)
add_compile_options(-fexceptions)

add_definitions(-DOS_ANDROID=1)
add_definitions(-DGNU_SUPPORT=1)
add_definitions(-DJSONCPP_NO_LOCALE_SUPPORT=1)

#add_subdirectory(${CMAKE_SOURCE_DIR}/Source/IPC)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/CrashHandler)

include_directories(${CMAKE_SOURCE_DIR}/Source/IPC)
#                    ${CMAKE_SOURCE_DIR}/Source/CrashHandler)

add_library(weexjsc SHARED
    ./IPC/Serializing/IPCSerializer.cpp
    ./IPC/IPCResult.cpp
    ./IPC/IPCSender.cpp
    ./IPC/IPCException.cpp
    ./IPC/IPCCommunicator.cpp
    ./IPC/IPCHandler.cpp
    ./IPC/IPCListener.cpp
    ./IPC/IPCFutexPageQueue.cpp
    ./IPC/IPCFutexPageQueue.cpp
    ./IPC/ashmem.c
    ./IPC/IPCCheck.cpp
    ./base/Compatible.cpp
    ./base/base64/base64.cpp
    ./base/base64/modp_base64/modp_b64.cc
    ./base/ScopedJString.cpp
    ./base/ScopedJStringUTF8.cpp
    ./WeexCore/platform/android/WeexJSConnection.cpp
    ./WeexCore/platform/android/BridgeAndroid.cpp
    ./WeexCore/platform/android/ExtendJSApi.cpp
    ./WeexCore/platform/android/WeexProxy.cpp
    ./WeexCore/platform/android/WeexCore.cpp
    )

#    set(weexjsc_LIBRARY_TYPE SHARED)

    set(weexjsc_LIBRARIES
    CrashHandler
    log)

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
                                log
                                z)
#                                ${CMAKE_SOURCE_DIR}/libyogacore.so)
#    target_link_libraries(weexjsc ${CMAKE_SOURCE_DIR}/libyogacore.so)
    
