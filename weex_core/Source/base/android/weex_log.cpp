/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "base/android/weex_log.h"
#include <stdint.h>

#if defined(OS_ANDROID)
#include <android/log.h>
#endif

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stddef.h>
#include <pthread.h>
#include <cstring>
#include <ostream>
#include <iostream>

namespace base {
static int CurrentProcessID() {
    int pid = (int)getpid();
    return pid;
}

static int CurrentThreadID() {
    int tid = (int)gettid();
    return tid;
}

static const char* PriorityName(int priority) {
    switch(priority) {
      case VERBOSE:
        return "VERBOSE";
      case INFO:
        return "INFO";
      case WARNING:
        return "WARNING";
      case ERROR:
        return "ERROR";
      case FATAL:
        return "FATAL";
    }
    return "Unknow";
}

#if defined(OS_ANDROID)
static android_LogPriority NativeToAndroidLogPriority(int priority) {
    android_LogPriority a_priority = ANDROID_LOG_UNKNOWN;
    switch(priority) {
      case VERBOSE:
        a_priority = ANDROID_LOG_VERBOSE;
        break;
      case INFO:
        a_priority = ANDROID_LOG_INFO;
        break;
      case WARNING:
        a_priority = ANDROID_LOG_WARN;
        break;
      case ERROR:
        a_priority = ANDROID_LOG_ERROR;
        break;
      case FATAL:
        a_priority = ANDROID_LOG_FATAL;
        break;
    }
    return a_priority;
}
#endif

LogMessage::LogMessage(const char* filename, int line, int priority)
    : file_name_(filename),
      line_(line),
      log_priority_(priority) {
    InitializeLog(file_name_, line_, log_priority_);   
}

void LogMessage::InitializeLog(const char* filename, int line, int priority) {
    int pid = CurrentProcessID();
    int tid = CurrentThreadID();
    log_stream_ << '[';
    log_stream_ << pid << ':' << tid << ':';

    //get time
    timeval tv;
    gettimeofday(&tv, nullptr);
    time_t sec = tv.tv_sec;
    struct tm local_time;
    localtime_r(&sec, &local_time);
    struct tm* tm_time = &local_time;
    log_stream_ << std::setfill('0')                                                   
            << std::setw(2) << 1 + tm_time->tm_mon
            << '-'
            << std::setw(2) << tm_time->tm_mday 
            << '/'
            << std::setw(2) << tm_time->tm_hour 
            << ':'
            << std::setw(2) << tm_time->tm_min
            << ':'
            << std::setw(2) << tm_time->tm_sec 
            << '.' 
            << std::setw(6) << tv.tv_usec 
            << ':';
    //input priority
    log_stream_ << PriorityName(priority) << ':';

    //remove the path of file and input file name and line number
    std::string fullname(file_name_);
    size_t last_pos = fullname.find_last_of("\\/");
    std::string pure_file_name;
    if (last_pos != std::string::npos)
        pure_file_name = fullname.substr(last_pos + 1);
    else
        pure_file_name.assign(file_name_);

    log_stream_ << pure_file_name << ':' << std::setw(2) << line_ << ']';
}

LogMessage::~LogMessage() {
    log_stream_ << std::endl;
#if defined(OS_ANDROID)
    __android_log_write(NativeToAndroidLogPriority(log_priority_), "WeexCore", log_stream_.str().c_str());
#else
    std::cout << log_stream_.str();
#endif
}

} //namespace base
