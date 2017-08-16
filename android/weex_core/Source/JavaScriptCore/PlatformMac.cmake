list(APPEND JavaScriptCore_SOURCES
    API/JSAPIWrapperObject.mm
    API/JSContext.mm
    API/JSManagedValue.mm
    API/JSRemoteInspector.cpp
    API/JSStringRefCF.cpp
    API/JSValue.mm
    API/JSVirtualMachine.mm
    API/JSWrapperMap.mm
    API/ObjCCallbackFunction.mm

    inspector/remote/RemoteAutomationTarget.cpp
    inspector/remote/RemoteControllableTarget.cpp
    inspector/remote/RemoteInspectionTarget.cpp
    inspector/remote/RemoteInspector.cpp

    inspector/remote/cocoa/RemoteConnectionToTargetCocoa.mm
    inspector/remote/cocoa/RemoteInspectorCocoa.mm
    inspector/remote/cocoa/RemoteInspectorXPCConnection.mm
)
add_definitions(-DSTATICALLY_LINKED_WITH_WTF -D__STDC_WANT_LIB_EXT1__)

find_library(SECURITY_LIBRARY Security)
list(APPEND JavaScriptCore_LIBRARIES
    ${SECURITY_LIBRARY}
)

list(APPEND JavaScriptCore_INCLUDE_DIRECTORIES
    ${JAVASCRIPTCORE_DIR}/disassembler/udis86
    ${JAVASCRIPTCORE_DIR}/icu
    ${JAVASCRIPTCORE_DIR}/inspector/remote/cocoa
)

set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-compatibility_version 1 -current_version ${WEBKIT_MAC_VERSION}")

# FIXME: Make including these files consistent in the source so these forwarding headers are not needed.
if (NOT EXISTS ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InspectorBackendDispatchers.h)
    file(WRITE ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InspectorBackendDispatchers.h "#include \"inspector/InspectorBackendDispatchers.h\"")
endif ()
if (NOT EXISTS ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InspectorFrontendDispatchers.h)
    file(WRITE ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InspectorFrontendDispatchers.h "#include \"inspector/InspectorFrontendDispatchers.h\"")
endif ()
if (NOT EXISTS ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InspectorProtocolObjects.h)
    file(WRITE ${DERIVED_SOURCES_JAVASCRIPTCORE_DIR}/InspectorProtocolObjects.h "#include \"inspector/InspectorProtocolObjects.h\"")
endif ()
