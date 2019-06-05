#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

cmake_minimum_required(VERSION 3.4.1)

set(BASE_LIBRARY_NAME weexbase)
include_directories(${WEEX_CORE_SOURCE_DIR})

set(${BASE_LIBRARY_NAME}_SOURCES
    common.h
    closure.h
    make_copyable.h
    time_calculator.h
    time_point.h
    time_point.cc
    time_unit.h
    #CoreConstants.h
    core_constants.h
    log_defines.h
    log_defines.cpp
    time_utils.h
    base64/base64.h
    base64/base64.cpp
    base64/modp_base64/modp_b64.h
    base64/modp_base64/modp_b64.cc
    base64/modp_base64/modp_b64_data.h
    crash/crash_handler.h
    crash/crash_handler.cpp
    message_loop/message_loop.h
    message_loop/message_loop.cc
    message_loop/message_pump.h
    message_loop/message_pump_android.h
    message_loop/message_pump_android.cc
    message_loop/message_pump_darwin.h
    message_loop/message_pump_darwin.cc
    message_loop/message_pump_posix.h
    message_loop/message_pump_posix.cc
    thread/thread.h
    thread/thread_impl.h
    thread/thread_local.h
    thread/thread_impl_android.h
    thread/thread_impl_android.cc
    thread/thread_impl_darwin.h
    thread/thread_impl_darwin.cc
    thread/thread_impl_posix.h
    thread/thread_impl_posix.cc
    thread/waitable_event.h
    third_party/icu/icu_utf.cpp
    utils/log_utils.h
    utils/log_utils.cpp
    utils/ThreadLocker.h
    utils/ThreadLocker.cpp   
    )

if (ANDROID)
    add_definitions(-DOS_ANDROID=1)
    if ("${ANDROID_ABI}" STREQUAL "x86")
    else ()
      list(APPEND ${BASE_LIBRARY_NAME}_SOURCES
          crash/backtrace.h
          crash/backtrace_arm.c
          )
    endif ()

    set(BASE_ANDROID_SOURCES
        android/call_stack.h
        android/call_stack.cpp
        android/weex_log.h
        android/weex_log.cpp
        android/log_utils.h
        android/jni_type.h
        android/jni_type.cpp
        android/jni/android_jni.h
        android/jni/android_jni.cpp
        android/jni/jbytearray_ref.h
        android/jni/jbytearray_ref.cpp
        android/jni/scoped_java_ref.h
        android/jni/scoped_java_ref.cpp
       )
    list(APPEND ${BASE_LIBRARY_NAME}_SOURCES
        ${BASE_ANDROID_SOURCES}
        )
endif (ANDROID)

add_library(${BASE_LIBRARY_NAME} STATIC
    ${${BASE_LIBRARY_NAME}_SOURCES}
)

target_include_directories(${BASE_LIBRARY_NAME} PUBLIC .)
