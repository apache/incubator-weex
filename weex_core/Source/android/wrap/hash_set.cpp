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

#include "android/wrap/hash_set.h"

#include "base/android/jni/android_jni.h"

namespace WeexCore {
const char kHashSetClassPath[] = "java/util/HashSet";
jclass g_HashSet_clazz = nullptr;

static intptr_t g_HashSet_constructor = 0;
static base::android::ScopedLocalJavaRef<jobject> Java_HashSet_constructor(JNIEnv* env) {
  jmethodID method_id = base::android::GetMethod(
      env, g_HashSet_clazz, base::android::INSTANCE_METHOD, "<init>", "()V",
      &g_HashSet_constructor);

  auto result = env->NewObject(g_HashSet_clazz, method_id);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jobject>(env, result);
}

static intptr_t g_HashSet_add = 0;
static jboolean Java_HashSet_add(JNIEnv* env, jobject obj, jobject value) {
  jmethodID method_id = base::android::GetMethod(
      env, g_HashSet_clazz, base::android::INSTANCE_METHOD, "add",
      "(Ljava/lang/Object;)Z", &g_HashSet_add);

  auto result = env->CallBooleanMethod(obj, method_id, value);
  base::android::CheckException(env);
  return result;
}

bool HashSet::RegisterJNIUtils(JNIEnv* env) {
  g_HashSet_clazz = reinterpret_cast<jclass>(
      env->NewGlobalRef(base::android::GetClass(env, kHashSetClassPath).Get()));
  return true;
}

HashSet::HashSet() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Reset(env, Java_HashSet_constructor(env).Get());
}

HashSet::~HashSet() {}

void HashSet::Add(JNIEnv* env, const std::set<std::string>& set) {
  jstring jni_value;
  auto it = set.begin();
  auto end = set.end();

  for (; it != end; ++it) {
    jni_value = env->NewStringUTF((*it).c_str());
    Java_HashSet_add(env, jni_object(), jni_value);
    env->DeleteLocalRef(jni_value);
  }
}
}  // namespace WeexCore
