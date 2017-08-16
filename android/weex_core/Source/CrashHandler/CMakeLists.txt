cmake_minimum_required(VERSION 2.8.12)

set(CrashHandlerSource
CrashHandler.cpp
)

if (WTF_CPU_ARM)
    LIST(APPEND CrashHandlerSource Backtrace_ARM.c)
endif ()

add_library(CrashHandler
STATIC
${CrashHandlerSource})


#target_link_libraries(IPC
#JavaScriptCore)

target_include_directories(CrashHandler
PUBLIC
.)

