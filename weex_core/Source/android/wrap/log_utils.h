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

#ifndef WEEX_PROJECT_LOG_UTILS_H
#define WEEX_PROJECT_LOG_UTILS_H

#include <jni.h>
#include "base/log_defines.h"
#include "base/log_defines.h"

namespace WeexCore {
class LogUtils {
 public:
  static bool RegisterJNIUtils(JNIEnv *env);
  static void NativeLog(JNIEnv *env, const char *str_array);
};

class LogUtilsWeexCore : public weex::base::LogBase {
 public:
  bool log(WeexCore::LogLevel level,
           const char *tag,
           const char *file,
           unsigned long line,
           const char *log) override;
};

}  // namespace WeexCore


#endif  // WEEX_PROJECT_LOG_UTILS_H
