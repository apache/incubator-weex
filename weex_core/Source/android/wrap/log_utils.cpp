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

#include "android/wrap/log_utils.h"

#include "android/base/string/string_utils.h"
#include "base/android/jni/android_jni.h"

namespace WeexCore {

const char kWXLogUtilsClassPath[] = "com/taobao/weex/utils/WXLogUtils";
jclass g_WXLogUtils_clazz = nullptr;

static intptr_t g_WXLogUtils_d = 0;
static void Java_WXLogUtils_d(JNIEnv* env, jstring tag, jbyteArray msg) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXLogUtils_clazz, base::android::STATIC_METHOD, "d",
      "(Ljava/lang/String;[B)V", &g_WXLogUtils_d);

  env->CallStaticVoidMethod(g_WXLogUtils_clazz, method_id, tag, msg);
  base::android::CheckException(env);
}

bool LogUtils::RegisterJNIUtils(JNIEnv* env) {
  g_WXLogUtils_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kWXLogUtilsClassPath).Get()));
  return true;
}

void LogUtils::NativeLog(JNIEnv* env, const char* str_array) {
  auto tag = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF("jsLog"));
  auto msg = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, str_array));
  Java_WXLogUtils_d(env, tag.Get(), msg.Get());
}
}  // namespace WeexCore
