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

  LogBase() : printLevel(WeexCore::LogLevel::Error), m_debugMode(false), m_perfMode(false) {};

 public:
  WeexCore::LogLevel printLevel = WeexCore::LogLevel::Error;
  bool m_perfMode = false;
  bool m_debugMode = false;
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

  inline void setPrintLevel(WeexCore::LogLevel level) {
    if (m_log)
      m_log->printLevel = level;
  }

  inline void setDebugMode(bool debugFlag) {
    if (m_log)
      m_log->m_debugMode = debugFlag;
  }

  inline bool debugMode() {
    if(m_log)
      return m_log->m_debugMode;

    return false;
  }

  inline bool ferfMode() {
    if(m_log)
      return m_log->m_perfMode;

    return false;
  }

  inline void setPerfMode(bool perfFlag) {
    if (m_log)
      m_log->m_perfMode = perfFlag;
  }

  bool log(WeexCore::LogLevel level,
           const char *tag,
           const char *file,
           unsigned long line,
           const char *log) {
    if (m_log == nullptr) {
      return false;
    }

    if (!m_log->m_debugMode && level < m_log->printLevel) {
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
