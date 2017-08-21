cmake_minimum_required(VERSION 3.4.1)

set(CrashHandlerSource
CrashHandler.cpp
)

if (WTF_CPU_ARM)
    LIST(APPEND CrashHandlerSource Backtrace_ARM.c)
endif ()

add_library(CrashHandler
STATIC
${CrashHandlerSource})


target_include_directories(CrashHandler
PUBLIC
.)

