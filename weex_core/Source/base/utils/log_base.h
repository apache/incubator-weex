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
//
// Created by Darin on 2019-07-01.
//

#ifndef WEEX_PROJECT_LOG_BASE_H
#define WEEX_PROJECT_LOG_BASE_H

#include <string>
#include <mutex>
#include "base/log_defines.h"

namespace weex {
namespace base {
class LogBase {
 public:
  virtual bool log(WeexCore::LogLevel level,
                   const char *tag,
                   const char *file,
                   unsigned long line,
                   const char *log) = 0;

  inline void setPrintLevel(WeexCore::LogLevel level) { this->printLevel = level; }

  LogBase() : printLevel(WeexCore::LogLevel::Error) {};

 public:
  WeexCore::LogLevel printLevel = WeexCore::LogLevel::Error;
  bool
};

class LogImplement {
 public:
  static LogImplement *getLog() {
    static weex::base::LogImplement *mySelf;
    static std::once_flag once_flag;
    std::call_once(once_flag, []() { mySelf = new LogImplement(); });
    return mySelf;
  }

  void setLogImplement(weex::base::LogBase *logVirtual) {
    m_log = logVirtual;
  }

  bool log(WeexCore::LogLevel level,
           const char *tag,
           const char *file,
           unsigned long line,
           const char *log) {
    if (m_log == nullptr) {
      return false;
    }

    if (!WeexCore::DebugMode && level < m_log->printLevel) {
      return true;
    }

    return m_log->log(level, tag, file, line, log);
  }

 private:
  weex::base::LogBase *m_log;
  LogImplement() : m_log(nullptr) {}
};
}
}

#endif //WEEX_PROJECT_LOG_BASE_H
