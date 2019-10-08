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

#include <sstream>
#include <android/log.h>
#include "android/wrap/log_utils.h"

#include "android/base/string/string_utils.h"
#include "base/android/jni/android_jni.h"
#include "log_utils.h"

namespace WeexCore {

const char kWXLogUtilsClassPath[] = "org/apache/weex/utils/WXLogUtils";
static jclass g_WXLogUtils_clazz = nullptr;

static intptr_t g_WXLogUtils_d = 0;
static void Java_WXLogUtils_d(JNIEnv *env, jstring tag, jbyteArray msg) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXLogUtils_clazz, base::android::STATIC_METHOD, "d",
      "(Ljava/lang/String;[B)V", &g_WXLogUtils_d);

  env->CallStaticVoidMethod(g_WXLogUtils_clazz, method_id, tag, msg);
  base::android::CheckException(env);
}

static intptr_t g_WXLogUtils_i = 0;
static void Java_WXLogUtils_i(JNIEnv *env, jstring tag, jbyteArray msg) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXLogUtils_clazz, base::android::STATIC_METHOD, "i",
      "(Ljava/lang/String;[B)V", &g_WXLogUtils_i);

  env->CallStaticVoidMethod(g_WXLogUtils_clazz, method_id, tag, msg);
  base::android::CheckException(env);
}

static intptr_t g_WXLogUtils_w = 0;
static void Java_WXLogUtils_w(JNIEnv *env, jstring tag, jbyteArray msg) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXLogUtils_clazz, base::android::STATIC_METHOD, "w",
      "(Ljava/lang/String;[B)V", &g_WXLogUtils_w);

  env->CallStaticVoidMethod(g_WXLogUtils_clazz, method_id, tag, msg);
  base::android::CheckException(env);
}

static intptr_t g_WXLogUtils_e = 0;
static void Java_WXLogUtils_e(JNIEnv *env, jstring tag, jbyteArray msg) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXLogUtils_clazz, base::android::STATIC_METHOD, "e",
      "(Ljava/lang/String;[B)V", &g_WXLogUtils_e);

  env->CallStaticVoidMethod(g_WXLogUtils_clazz, method_id, tag, msg);
  base::android::CheckException(env);
}

static intptr_t g_WXLogUtils_Msg = 0;
static void Java_WXLogUtils_Performance(JNIEnv *env, jstring tag, jbyteArray msg) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXLogUtils_clazz, base::android::STATIC_METHOD, "performance",
      "(Ljava/lang/String;[B)V", &g_WXLogUtils_Msg);

  env->CallStaticVoidMethod(g_WXLogUtils_clazz, method_id, tag, msg);
  base::android::CheckException(env);
}

bool LogUtils::RegisterJNIUtils(JNIEnv *env) {
  g_WXLogUtils_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kWXLogUtilsClassPath).Get()));
  return true;
}

void LogUtils::NativeLog(JNIEnv *env, const char *str_array) {
  auto tag = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF("jsLog"));
  auto msg = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, str_array));
  Java_WXLogUtils_d(env, tag.Get(), msg.Get());
}

bool LogUtilsWeexCore::log(WeexCore::LogLevel level,
                           const char *tag,
                           const char *file,
                           unsigned long line,
                           const char *log) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr) {
    return false;
  }

  if(!weex::base::LogImplement::getLog()->debugMode()) {
    if(level < WeexCore::LogLevel::Error) {
      return true;
    }
  }

  std::stringstream ss;
  ss << file << ":" << line << "," << log;
  auto tag_jstring = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(tag));
  auto msg_jbyteArray = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, WeexCore::newJByteArray(env, ss.str().c_str()));
  switch (level) {
    case WeexCore::LogLevel::Info:
      WeexCore::Java_WXLogUtils_i(env,
                                  tag_jstring.Get(),
                                  msg_jbyteArray.Get());
      break;
    case WeexCore::LogLevel::Warn:
      WeexCore::Java_WXLogUtils_w(env,
                                  tag_jstring.Get(),
                                  msg_jbyteArray.Get());
      break;
    case WeexCore::LogLevel::Error:
    case WeexCore::LogLevel::Tlog:
    case WeexCore::LogLevel::Wtf:
      WeexCore::Java_WXLogUtils_e(env,
                                  tag_jstring.Get(),
                                  msg_jbyteArray.Get());
      break;
    case WeexCore::LogLevel::Performance :
      WeexCore::Java_WXLogUtils_Performance(env,
                                            tag_jstring.Get(),
                                            msg_jbyteArray.Get());
      break;
    default:
      WeexCore::Java_WXLogUtils_d(env,
                                  tag_jstring.Get(),
                                  msg_jbyteArray.Get());
      break;
  }
  return true;
}

}  // namespace WeexCore

