include(WebKitCommon)
set_property(DIRECTORY . PROPERTY FOLDER "bmalloc")

set(bmalloc_INCLUDE_DIRECTORIES
    "${BMALLOC_DIR}"
)

set(bmalloc_SOURCES
    bmalloc/Allocator.cpp
    bmalloc/AvailableMemory.cpp
    bmalloc/Cache.cpp
    bmalloc/Deallocator.cpp
    bmalloc/DebugHeap.cpp
    bmalloc/Environment.cpp
    bmalloc/Heap.cpp
    bmalloc/LargeMap.cpp
    bmalloc/Logging.cpp
    bmalloc/ObjectType.cpp
    bmalloc/StaticMutex.cpp
    bmalloc/VMHeap.cpp
    bmalloc/mbmalloc.cpp
)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND bmalloc_SOURCES
        bmalloc/Zone.cpp
    )
endif ()

set(bmalloc_LIBRARIES
    ${CMAKE_DL_LIBS}
)

WEBKIT_WRAP_SOURCELIST(${bmalloc_SOURCES})
include_directories(${bmalloc_INCLUDE_DIRECTORIES})
add_library(bmalloc STATIC ${bmalloc_SOURCES})
target_link_libraries(bmalloc ${bmalloc_LIBRARIES})
set_target_properties(bmalloc PROPERTIES COMPILE_DEFINITIONS "BUILDING_bmalloc")
