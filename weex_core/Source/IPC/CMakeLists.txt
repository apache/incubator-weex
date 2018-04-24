cmake_minimum_required(VERSION 2.8.12)

add_library(IPC
STATIC
./Serializing/IPCSerializer.cpp
./IPCResult.cpp
./IPCSender.cpp
./IPCException.cpp
./IPCCommunicator.cpp
./IPCHandler.cpp
./IPCListener.cpp
./IPCFutexPageQueue.cpp
./IPCFutexPageQueue.cpp
./ashmem.c
./IPCCheck.cpp)

#target_link_libraries(IPC
#JavaScriptCore)

target_include_directories(IPC
PUBLIC
.)
