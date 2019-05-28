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

namespace WeexCore {
    
    enum class LogLevel : int {
        Debug,
        Info,
        Warn,
        Error
    };
    
    void PrintLog(LogLevel level, const char* file, unsigned long line, const char* format, ...);
    
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
#define WEEX_CORE_LOG(level, format, ...)   WeexCore::PrintLog((level), WEEX_CORE_FILENAME, __LINE__, (format), ##__VA_ARGS__)

#define LOGE(format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Error, format, ##__VA_ARGS__)
#define LOGW(format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Warn, format, ##__VA_ARGS__)
#define LOGI(format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Info, format, ##__VA_ARGS__)

#ifdef DEBUG

#define LOGD(format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Debug, format, ##__VA_ARGS__)

#else

#define LOGD(format, ...)       ((void) 0)

#endif

#define LOGV                    LOGD
#define LOG_LINE                ((void) 0)

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
TypeName(const TypeName&) = delete;      \
void operator=(const TypeName&) = delete
#endif

#endif /* LogDefines_h */
