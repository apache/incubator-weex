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

namespace WeexCore {
    
    enum class LogLevel : int {
        Debug,
        Info,
        Warn,
        Error
    };
    
    void PrintLog(LogLevel level, const char* tag, const char* file, unsigned long line, const char* format, ...);
    
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
#define WEEX_CORE_LOG(level, TAG, format, ...)   WeexCore::PrintLog((level), TAG, WEEX_CORE_FILENAME, __LINE__, (format), ##__VA_ARGS__)

#define LOGE_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Error, TAG, format, ##__VA_ARGS__)
#define LOGE(format, ...)                LOGE_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#define LOGI_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Info, TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...)                LOGI_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#ifdef DEBUG

#define LOGD_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Debug, TAG, format, ##__VA_ARGS__)
#define LOGD(format, ...)                LOGD_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#define LOGW_TAG(TAG, format, ...)       WEEX_CORE_LOG(WeexCore::LogLevel::Warn, TAG, format, ##__VA_ARGS__)
#define LOGW(format, ...)                LOGW_TAG(WEEX_CORE_LOG_TAG, format, ##__VA_ARGS__)

#else

#define LOGD_TAG(TAG, format, ...)       ((void) 0)
#define LOGD(format, ...)                ((void) 0)

#define LOGW_TAG(TAG, format, ...)       ((void) 0)
#define LOGW(format, ...)                ((void) 0)

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
