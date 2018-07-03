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

#ifndef WEEX_PROJECT_SO_UTILS_H
#define WEEX_PROJECT_SO_UTILS_H

#include <jni.h>
#include <functional>
#include <string>

namespace WeexCore {
class SoUtils {
 public:
  static void Init(JNIEnv* env);
  static std::string FindLibJssSoPath();
  static void RegisterExceptionHanler(
      const std::function<void(const char*, const char*)>&
          ReportNativeInitStatus);

  inline static void set_cache_dir(char* cache_dir) { g_cache_dir = cache_dir; }

  inline static const char* cache_dir() { return g_cache_dir; }

  inline static void set_jss_so_path(char* jss_so_path) {
    g_jss_so_path = jss_so_path;
  }

  inline static const char* jss_so_path() { return g_jss_so_path; }

  inline static const char* jss_so_name() { return g_jss_so_name; }

  inline static bool pie_support() { return g_pie_support; }

  inline static void set_pie_support(bool startup_pie) {
    g_pie_support = startup_pie;
  }

 private:
  static const char* GetDefaultCacheDir(JNIEnv* env);
  static char* g_cache_dir;
  static char* g_jss_so_path;
  static char* g_jss_so_name;
  static bool g_pie_support;
    static std::function<void(const char*, const char*)> g_exception_handler;
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_SO_UTILS_H
