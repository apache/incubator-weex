add_definitions(-DSTATICALLY_LINKED_WITH_WTF)

if (USE_GLIB)
    list(APPEND JavaScriptCore_SYSTEM_INCLUDE_DIRECTORIES
        ${GLIB_INCLUDE_DIRS}
    )
    list(APPEND JavaScriptCore_LIBRARIES
        ${GLIB_LIBRARIES}
    )
endif ()
