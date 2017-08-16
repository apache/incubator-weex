list(APPEND JavaScriptCore_SOURCES
    API/JSStringRefBSTR.cpp
    API/JSStringRefCF.cpp
)

list(APPEND JavaScriptCore_INCLUDE_DIRECTORIES
    ${CMAKE_BINARY_DIR}/../include/private
)

if (WTF_PLATFORM_WIN_CAIRO)
    list(APPEND JavaScriptCore_LIBRARIES
        CFLite
    )
else ()
    list(APPEND JavaScriptCore_LIBRARIES
        CoreFoundation${DEBUG_SUFFIX}
        ${ICU_LIBRARIES}
        winmm
    )
endif ()

list(REMOVE_ITEM JavaScriptCore_SOURCES
    inspector/JSGlobalObjectInspectorController.cpp
)

file(COPY
    "${JAVASCRIPTCORE_DIR}/JavaScriptCore.vcxproj/JavaScriptCore.resources"
    DESTINATION
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

file(MAKE_DIRECTORY ${DERIVED_SOURCES_DIR}/ForwardingHeaders/JavaScriptCore)

set(JavaScriptCore_PRE_BUILD_COMMAND "${CMAKE_BINARY_DIR}/DerivedSources/JavaScriptCore/preBuild.cmd")
file(REMOVE "${JavaScriptCore_PRE_BUILD_COMMAND}")
foreach (_directory ${JavaScriptCore_FORWARDING_HEADERS_DIRECTORIES})
    file(APPEND "${JavaScriptCore_PRE_BUILD_COMMAND}" "@xcopy /y /d /f \"${JAVASCRIPTCORE_DIR}/${_directory}/*.h\" \"${DERIVED_SOURCES_DIR}/ForwardingHeaders/JavaScriptCore\" >nul 2>nul\n")
endforeach ()

set(JavaScriptCore_POST_BUILD_COMMAND "${CMAKE_BINARY_DIR}/DerivedSources/JavaScriptCore/postBuild.cmd")
file(WRITE "${JavaScriptCore_POST_BUILD_COMMAND}" "@xcopy /y /d /f \"${DERIVED_SOURCES_DIR}/JavaScriptCore/*.h\" \"${DERIVED_SOURCES_DIR}/ForwardingHeaders/JavaScriptCore\" >nul 2>nul\n")
file(APPEND "${JavaScriptCore_POST_BUILD_COMMAND}" "@xcopy /y /d /f \"${DERIVED_SOURCES_DIR}/JavaScriptCore/inspector/*.h\" \"${DERIVED_SOURCES_DIR}/ForwardingHeaders/JavaScriptCore\" >nul 2>nul\n")

set(JavaScriptCore_OUTPUT_NAME JavaScriptCore${DEBUG_SUFFIX})
