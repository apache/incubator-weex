cmake_minimum_required(VERSION 2.8.12)

if (NOT USE_SYSTEM_MALLOC)
    add_subdirectory(bmalloc)
endif ()
add_subdirectory(WTF)
add_subdirectory(JavaScriptCore)
add_subdirectory(IPC)
add_subdirectory(CrashHandler)

include(WebKitCommon)

if (${CMAKE_TARGET_WORD_BITS} EQUAL 32)
    set(SERVERSUFFIX "")
    set(weexjsc_SOURCES
            ./base/Compatible.cpp
            ./base/base64/base64.cpp
            ./base/base64/modp_base64/modp_b64.cc
            ./base/ScopedJString.cpp
            ./base/ScopedJStringUTF8.cpp
            ./base/fpconv.cpp
            ./rapidjson/weexjsontools.cpp
            ./WeexCore/platform/android/WeexJSConnection.cpp
            ./WeexCore/platform/android/BridgeAndroid.cpp
            ./WeexCore/platform/android/ExtendJSApi.cpp
            ./WeexCore/platform/android/WeexProxy.cpp
            ./WeexCore/platform/android/WeexCore.cpp
            ./WeexCore/render/RenderObject.cpp
            ./WeexCore/render/RenderPage.cpp
            ./WeexCore/render/RenderManager.cpp
            ./WeexCore/render/action/RenderAction.cpp
            ./WeexCore/layout/WXStyleUtil.cpp
            )

    set(weexjsc_LIBRARY_TYPE SHARED)

    set(weexjsc_LIBRARIES
            IPC
            WTF
            log)

    WEBKIT_FRAMEWORK(weexjsc)


    set_target_properties(weexjsc
            PROPERTIES
            LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}//weexjsc_version_script.txt -Wl,--no-undefined -Wl,-soname,libweexjsc.so"
            )

    add_custom_command(
            TARGET weexjsc
            POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:weexjsc> -o ${CMAKE_SOURCE_DIR}/libweexjsc.so
            VERBATIM
    )

    target_link_libraries(weexjsc ${CMAKE_SOURCE_DIR}/libyogacore.so)
else ()
    set(SERVERSUFFIX 64)
endif ()

set(WEEXJSSERVER_NAME weexjss${SERVERSUFFIX})
set(${WEEXJSSERVER_NAME}_SOURCES
        ./base/Compatible.cpp
        ./base/base64/modp_base64/modp_b64.cc
        ./base/base64/base64.cpp
        ./WeexJsServer/WeexJSServer.cpp
        ./WeexJsServer/WeexJSServerMain.cpp
        )

set(${WEEXJSSERVER_NAME}_LIBRARY_TYPE SHARED)

set(${WEEXJSSERVER_NAME}_LIBRARIES
        JavaScriptCore
        IPC
        CrashHandler
        log)

WEBKIT_FRAMEWORK(${WEEXJSSERVER_NAME})


set_target_properties(${WEEXJSSERVER_NAME}
        PROPERTIES
        LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}//weexjss_version_script.txt -Wl,--no-undefined -Wl,-soname,libweexjss${SERVERSUFFIX}.so"
        )

add_custom_command(
        TARGET ${WEEXJSSERVER_NAME}
        POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${WEEXJSSERVER_NAME}> -o ${CMAKE_SOURCE_DIR}/libweexjss${SERVERSUFFIX}.so
        VERBATIM
)

set(WEEXJSSERVERSTUB_NAME weexjsstub${SERVERSUFFIX})
add_executable(${WEEXJSSERVERSTUB_NAME}
        ./WeexJsServer/WeexJSServerMainStub.cpp
        )

target_link_libraries(${WEEXJSSERVERSTUB_NAME}
        ${WEEXJSSERVER_NAME}
        )

if (APPLE)
    set_target_properties(${WEEXJSSERVERSTUB_NAME}
            PROPERTIES LINK_FLAGS
            "-Wl,--entry=_start"
            )
endif ()

add_custom_command(
        TARGET ${WEEXJSSERVERSTUB_NAME}
        POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${WEEXJSSERVERSTUB_NAME}> -o ${CMAKE_SOURCE_DIR}/libweexjsb${SERVERSUFFIX}.so
        VERBATIM
)
