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

#include "android/wrap/wx_params.h"
#include "android/base/string/scoped_jstring_utf8.h"
#include "base/android/jniprebuild/jniheader/WXParams_jni.h"

namespace WeexCore {

bool WXParams::RegisterJNIUtils(JNIEnv* env) { return RegisterNativesImpl(env); }

WXParams::WXParams(JNIEnv* env, jobject jni_object) { Reset(env, jni_object); }
WXParams::~WXParams() {}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetOsVersion(JNIEnv* env) {
  return Java_WXParams_getOsVersion(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetPlatform(JNIEnv* env) {
  return Java_WXParams_getPlatform(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetUseSingleProcess(
    JNIEnv* env) {
  return Java_WXParams_getUseSingleProcess(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetLibJssPath(
    JNIEnv* env) {
  return Java_WXParams_getLibJssPath(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetCacheDir(JNIEnv* env) {
  return Java_WXParams_getCacheDir(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetWeexVersion(
    JNIEnv* env) {
  return Java_WXParams_getWeexVersion(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetDeviceModel(
    JNIEnv* env) {
  return Java_WXParams_getDeviceModel(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetAppVersion(
    JNIEnv* env) {
  return Java_WXParams_getAppVersion(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetAppName(JNIEnv* env) {
  return Java_WXParams_getAppName(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetDeviceWidth(
    JNIEnv* env) {
  return Java_WXParams_getDeviceWidth(env, jni_object());
}

base::android::ScopedLocalJavaRef<jstring> WXParams::GetDeviceHeight(
    JNIEnv* env) {
  return Java_WXParams_getDeviceHeight(env, jni_object());
}

base::android::ScopedLocalJavaRef<jobject> WXParams::GetOptions(JNIEnv* env) {
  return Java_WXParams_getOptions(env, jni_object());
}
}  // namespace WeexCore
