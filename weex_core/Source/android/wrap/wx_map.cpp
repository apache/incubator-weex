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

#include "android/wrap/wx_map.h"

#include "android/base/string/string_utils.h"
#include "android/utils/cache_utils.h"
#include "base/android/jni/android_jni.h"

namespace WeexCore {
const char kWXMapClassPath[] = "org/apache/weex/utils/WXMap";
jclass g_WXMap_clazz = nullptr;

static intptr_t g_WXMap_constructor = 0;
static base::android::ScopedLocalJavaRef<jobject> Java_WXMap_constructor(JNIEnv* env) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXMap_clazz, base::android::INSTANCE_METHOD, "<init>", "()V",
      &g_WXMap_constructor);

  auto result = env->NewObject(g_WXMap_clazz, method_id);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jobject>(env, result);
}

static intptr_t g_WXMap_put = 0;
static base::android::ScopedLocalJavaRef<jstring> Java_WXMap_put(JNIEnv* env, jobject obj, jstring key,
                              jbyteArray value) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WXMap_clazz, base::android::INSTANCE_METHOD, "put",
      "(Ljava/lang/String;[B)Ljava/lang/String;", &g_WXMap_put);

  auto result = env->CallObjectMethod(obj, method_id, key, value);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jstring>(env, (jstring)result);
}

bool WXMap::RegisterJNIUtils(JNIEnv* env) {
  g_WXMap_clazz = reinterpret_cast<jclass>(
      env->NewGlobalRef(base::android::GetClass(env, kWXMapClassPath).Get()));
  return true;
}

WXMap::WXMap() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Reset(env, Java_WXMap_constructor(env).Get());
}

WXMap::~WXMap() {}

void WXMap::Put(
    JNIEnv* env,
    const std::vector<std::pair<std::string, std::string>>& vector) {
  jstring jni_key;
  jbyteArray jni_value;

  for (int i = 0; i < vector.size(); ++i) {
    jni_key = env->NewStringUTF(vector[i].first.c_str());
    jni_value = newJByteArray(env, vector[i].second.c_str());
    Java_WXMap_put(env, jni_object(), jni_key, jni_value);
    env->DeleteLocalRef(jni_value);
    env->DeleteLocalRef(jni_key);
  }
}

void WXMap::Put(JNIEnv* env, const std::map<std::string, std::string>& map) {
  std::map<std::string, std::string>::const_iterator it = map.begin();
  std::map<std::string, std::string>::const_iterator end = map.end();
  jstring jni_key;
  jbyteArray jni_value;

  for (; it != end; ++it) {
    jni_key = env->NewStringUTF(it->first.c_str());
    jni_value = newJByteArray(env, it->second.c_str());
    Java_WXMap_put(env, jni_object(), jni_key, jni_value);
    env->DeleteLocalRef(jni_value);
    env->DeleteLocalRef(jni_key);
  }
}
}  // namespace WeexCore
