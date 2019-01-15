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
#ifndef WEEX_LOG_H_
#define WEEX_LOG_H_
#include <stddef.h>                                                                
                                                                                   
#include <cassert>                                                                 
#include <cstring>                                                                 
#include <sstream>                                                                 
#include <string>                                                                  
#include <iomanip>
#include <type_traits>                                                             
#include <utility>

#include "base/common.h"

namespace base {

const int VERBOSE = 0;
const int INFO = 1;
const int WARNING = 2;
const int ERROR = 3;
const int FATAL = 4;

#define LOG_ON 1

#define LOG_VERBOSE base::DestructorHelper() |( base::LogMessage(__FILE__, __LINE__, base::VERBOSE).stream())
#define LOG_INFO base::DestructorHelper() |( base::LogMessage(__FILE__, __LINE__, base::INFO).stream())
#define LOG_WARNING base::DestructorHelper() |( base::LogMessage(__FILE__, __LINE__, base::WARNING).stream())
#define LOG_ERROR base::DestructorHelper() |( base::LogMessage(__FILE__, __LINE__, base::ERROR).stream())
#define LOG_FATAL base::DestructorHelper() |( base::LogMessage(__FILE__, __LINE__, base::FATAL).stream())

// use log like this: LOG(INFO) << "abc";
#if LOG_ON
#define LOG(priority) LOG_##priority
#else
#define LOG(priority) 0
#endif

class LogMessage {
 public:
  LogMessage(const char* filename, int line, int level);
  ~LogMessage();

  std::ostream& stream() { return log_stream_; }

  int log_priority() const {return log_priority_; }

  std::string str() { return log_stream_.str(); }

 private:
  void InitializeLog(const char* filename, int line, int priority);

  int log_priority_;
  std::ostringstream log_stream_;

  const char* file_name_;
  const int line_;

  DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

class DestructorHelper {
 public:
  DestructorHelper() = default;
  void operator|(std::ostream&) { }
};

} //namespace base
#endif
