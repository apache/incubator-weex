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

#ifndef LogDefines_h
#define LogDefines_h
#include <cstring>
#include <string>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <mutex>

namespace WeexCore {

enum class LogLevel : int {
  All = 0,
  Verbose,
  Debug,
  Info,
  Warn,
  Error,
  Tlog,
  Wtf,
  Off,
  Performance,
};

void PrintLog(LogLevel level,
              const char *tag,
              const char *file,
              unsigned long line,
              const char *format,
              ...);
}

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

  inline void setDebugMode(bool isDebug) {
    if (isDebug) {
      printLevel = WeexCore::LogLevel::Debug;
    } else {
      printLevel = WeexCore::LogLevel::Error;
    }
    m_debugMode = isDebug;
  }

  inline bool is_debug_mode() { return m_debugMode; }

  inline void setPerfMode(bool isPerf) {
    m_perfMode = isPerf;
  }

  inline bool is_perf_mode() { return m_perfMode; }

  inline void setPrintLevel(WeexCore::LogLevel l) {
    printLevel = l;
  }

  inline WeexCore::LogLevel print_level() { return printLevel; }

  inline bool shouldPrint(WeexCore::LogLevel l) {
    if (m_perfMode && l == WeexCore::LogLevel::Performance) {
      return true;
    }

    return l != WeexCore::LogLevel::Performance && l >= printLevel;
  }

 private:
  WeexCore::LogLevel printLevel = WeexCore::LogLevel::Error;
  bool m_perfMode = false;
  bool m_debugMode = false;
};

class LogImplement {
 public:
  static LogImplement *getLog() {
    static std::once_flag once_flag;
    std::call_once(once_flag, []() { g_instance = new LogImplement(); });
    return g_instance;
  }

  void setLogImplement(weex::base::LogBase *logVirtual) {
    m_log = logVirtual;
  }

  inline void setPrintLevel(WeexCore::LogLevel level) {
    if (m_log) {
      m_log->setPrintLevel(level);
    }
  }

  inline void setDebugMode(bool debugFlag) {
    if (m_log) {
      m_log->setDebugMode(debugFlag);
    }
  }

  inline bool debugMode() {
    if (m_log)
      return m_log->is_debug_mode();

    return false;
  }

  inline bool perfMode() {
    if (m_log)
      return m_log->is_perf_mode();

    return false;
  }

  inline void setPerfMode(bool perfFlag) {
    if (m_log)
      m_log->setPerfMode(perfFlag);
  }

  inline bool can_print_log(WeexCore::LogLevel l) {
    if (m_log)
      return m_log->shouldPrint(l);

    return false;
  }

  bool log(WeexCore::LogLevel level,
           const char *tag,
           const char *file,
           unsigned long line,
           const char *log) {
    if (m_log == nullptr) {
      return false;
    }

    if (file == nullptr
        || log == nullptr) {
      return true;
    }

    if (!m_log->shouldPrint(level)) {
      return true;
    }

    return m_log->log(level, tag, file, line, log);
  }

 private:
  weex::base::LogBase *m_log;
  static LogImplement *g_instance;
  LogImplement() : m_log(nullptr) {}
};
}
}

#if defined(LOGE)
#undef LOGE
#endif

#if defined(LOGW)
#undef LOGW
#endif

#if defined(LOGI)
#undef LOGI
#endif

#if defined(LOGD)
#undef LOGD
#endif

#if defined(LOGV)
#undef LOGV
#endif

#define WEEX_CORE_LOG_TAG "WeexCore"
#define WEEX_CORE_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define WEEX_CORE_LOG(level, TAG, format, ...) \
        do{ \
        if(weex::base::LogImplement::getLog()->can_print_log((level))) \
          WeexCore::PrintLog((level), TAG, WEEX_CORE_FILENAME, __LINE__, (format), ##__VA_ARGS__);\
        }while(0);\

#define LOGE_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Error, TAG, format, ##__VA_ARGS__)
#define LOGE(format, ...)                LOGE_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#define LOGW_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Warn, TAG, format, ##__VA_ARGS__)

#define LOGW(format, ...)                LOGW_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#define LOGI_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Info, TAG, format, ##__VA_ARGS__)

#define LOGI(format, ...)                LOGI_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#define LOG_Performance(INSTANCE_ID, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Performance, INSTANCE_ID, format, ##__VA_ARGS__)

#define LOG_TLOG(TAG, format, ...)  WEEX_CORE_LOG(WeexCore::LogLevel::Tlog, TAG, format, ##__VA_ARGS__)
#ifdef __ANDROID__
#define LOGE_FOR_DEBUG(TAG, format, ...)  __android_log_print(ANDROID_LOG_ERROR,TAG,format,##__VA_ARGS__)
#endif


#ifdef DEBUG
#define LOGD_TAG(TAG, format, ...) WEEX_CORE_LOG(WeexCore::LogLevel::Debug, TAG, format, ##__VA_ARGS__)

#define LOGD(format, ...) LOGD_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#else
#define LOGD_TAG(TAG, format, ...)       ((void) 0)
#define LOGD(format, ...)                ((void) 0)

#endif

#define LOGV                    LOGD
#define LOGV_TAG                LOGD_TAG
#define LOG_LINE                ((void) 0)

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
TypeName(const TypeName&) = delete;      \
void operator=(const TypeName&) = delete
#endif

#endif /* LogDefines_h */
