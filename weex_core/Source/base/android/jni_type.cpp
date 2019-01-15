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

#include "base/android/jni_type.h"

#include "base/android/jni/android_jni.h"

namespace base {
namespace android {

const char kDoubleClassPath[] = "java/lang/Double";
jclass g_Double_clazz = nullptr;

static intptr_t g_Double_doubleValue = 0;
static jdouble Java_Double_doubleValue(JNIEnv* env, jobject obj) {
  jmethodID method_id = base::android::GetMethod(
      env, g_Double_clazz, base::android::INSTANCE_METHOD, "doubleValue", "()D",
      &g_Double_doubleValue);
  auto result = env->CallDoubleMethod(obj, method_id);
  base::android::CheckException(env);
  return result;
}

jdouble JNIType::DoubleValue(JNIEnv* env, jobject value) {
  return Java_Double_doubleValue(env, value);
}

ScopedLocalJavaRef<jfloatArray> JNIType::NewFloatArray(JNIEnv* env, int length,
                                                       float* array) {
  jfloatArray jni_array = env->NewFloatArray(length);
  env->SetFloatArrayRegion(jni_array, 0, length, array);
  return ScopedLocalJavaRef<jfloatArray>(env, jni_array);
}

bool JNIType::RegisterJNIUtils(JNIEnv* env) {
  g_Double_clazz = reinterpret_cast<jclass>(
      env->NewGlobalRef(base::android::GetClass(env, kDoubleClassPath).Get()));
  return true;
}

}  // namespace android
}  // namespace base
