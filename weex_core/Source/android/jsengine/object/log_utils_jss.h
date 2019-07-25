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
// Created by Darin on 2019-06-24.
//

#ifndef WEEX_PROJECT_LOG_UTILS_JSS_H
#define WEEX_PROJECT_LOG_UTILS_JSS_H

#include "base/log_defines.h"
#include "weex_env.h"
class LogUtilsJSS : public weex::base::LogBase {
 public:
  bool log(WeexCore::LogLevel level,
           const char *tag,
           const char *file,
           unsigned long line,
           const char *log) override {

    //Please do not add Any LOGE in this function.

    // for performance, only send TLog & performance Log to main process
    if (level == WeexCore::LogLevel::Performance
        || level == WeexCore::LogLevel::Tlog) {
      return WeexEnv::getEnv()->sendLog((int) level, tag, file, line, log);
    }
    return false;
  }
};
#endif //WEEX_PROJECT_LOG_UTILS_JSS_H
