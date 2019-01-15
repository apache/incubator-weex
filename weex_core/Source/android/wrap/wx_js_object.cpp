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

#include "android/wrap/wx_js_object.h"

#include "base/android/jni/android_jni.h"

namespace WeexCore {
const char kWXJSObjectClassPath[] = "com/taobao/weex/bridge/WXJSObject";
jclass g_WXJSObject_clazz = nullptr;

jfieldID g_WXJSObject_filedID_type = nullptr;
static jint Java_WXJSObject_get_filedID_type(JNIEnv* env, jobject obj) {
  if (g_WXJSObject_filedID_type == nullptr) {
    g_WXJSObject_filedID_type =
        env->GetFieldID(g_WXJSObject_clazz, "type", "I");
  }
  auto result = env->GetIntField(obj, g_WXJSObject_filedID_type);
  base::android::CheckException(env);
  return result;
}

jfieldID g_WXJSObject_filedID_data = nullptr;
static base::android::ScopedLocalJavaRef<jobject>
Java_WXJSObject_get_filedID_data(JNIEnv* env, jobject obj) {
  if (g_WXJSObject_filedID_data == nullptr) {
    g_WXJSObject_filedID_data =
        env->GetFieldID(g_WXJSObject_clazz, "data", "Ljava/lang/Object;");
  }
  auto result = env->GetObjectField(obj, g_WXJSObject_filedID_data);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jobject>(env, result);
}

jfieldID g_WXJSObject_filedID_key = nullptr;
static base::android::ScopedLocalJavaRef<jstring>
Java_WXJSObject_get_filedID_key(JNIEnv* env, jobject obj) {
  if (g_WXJSObject_filedID_key == nullptr) {
    g_WXJSObject_filedID_key =
        env->GetFieldID(g_WXJSObject_clazz, "key", "Ljava/lang/String;");
  }
  auto result = env->GetObjectField(obj, g_WXJSObject_filedID_key);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jstring>(env, (jstring)result);
}

bool WXJSObject::RegisterJNIUtils(JNIEnv* env) {
  g_WXJSObject_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kWXJSObjectClassPath).Get()));
  return true;
}

WXJSObject::WXJSObject(JNIEnv* env, jobject jni_object) {
  Reset(env, jni_object);
}

WXJSObject::~WXJSObject() {}

base::android::ScopedLocalJavaRef<jobject> WXJSObject::GetData(JNIEnv* env) {
  if (jni_object() == nullptr) {
    return base::android::ScopedLocalJavaRef<jobject>();
  }
  return Java_WXJSObject_get_filedID_data(env, jni_object());
}

jint WXJSObject::GetType(JNIEnv* env) {
  if (jni_object() == nullptr) {
    return -1;
  }
  return Java_WXJSObject_get_filedID_type(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXJSObject::GetKey(JNIEnv* env) {
  if (jni_object() == nullptr) {
    return base::android::ScopedLocalJavaRef<jstring>();
  }
  return Java_WXJSObject_get_filedID_key(env, jni_object());
}
}  // namespace WeexCore
