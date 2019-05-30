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

#include "log_defines.h"
#include <string>
#include <array>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "core/manager/weex_core_manager.h"

namespace WeexCore {
    
    struct LogFlattenHelper {
        LogFlattenHelper() : mLargeBuf() {}
        LogFlattenHelper(const char *fmt, va_list args) : LogFlattenHelper() {
            set(fmt, args);
        }
        ~LogFlattenHelper() {
            if (mLargeBuf)
                free(mLargeBuf);
        }
        
        const char *str() const { return mLargeBuf ? mLargeBuf : mSmallBuf.data(); }
        LogFlattenHelper &set(const char *fmt, va_list args);
        
    private:
        LogFlattenHelper(const LogFlattenHelper &) = delete;
        void operator=(const LogFlattenHelper &) = delete;
        
        std::array<char, 4096> mSmallBuf;
        char *mLargeBuf;
    };
    
    LogFlattenHelper &LogFlattenHelper::set(const char *fmt, va_list args) {
        va_list argsCopy;
        va_copy(argsCopy, args);
        int len = 1 + vsnprintf(nullptr, 0, fmt, argsCopy);
        va_end(argsCopy);
        if (len <= 1) {
            mSmallBuf[0] = 0;
            return *this;
        }
        if (len > (int)mSmallBuf.size())
            mLargeBuf = static_cast<char *>(malloc(len));
        int rv;
        if (mLargeBuf) {
            rv = vsnprintf(mLargeBuf, len, fmt, args);
        } else {
            rv = vsnprintf(mSmallBuf.data(), mSmallBuf.size(), fmt, args);
        }
        (void)rv;
        return *this;
    }
    
    void PrintLog(LogLevel level, const char* tag, const char* file, unsigned long line, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        LogFlattenHelper log(fmt, args);
        va_end(args);
        
        LogBridge* logBridge = WeexCore::WeexCoreManager::Instance()->get_log_bridge();
        if (logBridge) {
            // Log to bridge
            logBridge->log(level, tag, file, line, log.str());
        }
        else {
            // Log to console by default
#ifdef __ANDROID__
            switch (level) {
                case LogLevel::Error:
                    __android_log_print(ANDROID_LOG_ERROR, tag, "%s:%lu, %s", file, line, log.str());
                    break;
                case LogLevel::Warn:
                    __android_log_print(ANDROID_LOG_WARN, tag, "%s:%lu, %s", file, line, log.str());
                    break;
                case LogLevel::Info:
                    __android_log_print(ANDROID_LOG_INFO, tag, "%s:%lu, %s", file, line, log.str());
                    break;
                case LogLevel::Debug:
                    __android_log_print(ANDROID_LOG_DEBUG, tag, "%s:%lu, %s", file, line, log.str());
                    break;
                default:
                    break;
            }
#elif __APPLE__
            switch (level) {
                case LogLevel::Error:
                    printf("<%s:Error|%s:%lu> %s\n", tag, file, line, log.str());
                    break;
                case LogLevel::Warn:
                    printf("<%s:Warn|%s:%lu> %s\n", tag, file, line, log.str());
                    break;
                case LogLevel::Info:
                    printf("<%s:Info|%s:%lu> %s\n", tag, file, line, log.str());
                    break;
                case LogLevel::Debug:
                    printf("<%s:Debug|%s:%lu> %s\n", tag, file, line, log.str());
                    break;
                default:
                    break;
            }
#endif
        }
    }
    
}
