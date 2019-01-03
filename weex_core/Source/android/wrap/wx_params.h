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

#ifndef WEEX_PROJECT_WX_PARAMS_H
#define WEEX_PROJECT_WX_PARAMS_H

#include <string>
#include "base/android/jni/jni_object_wrap.h"

namespace WeexCore {
class WXParams : public JNIObjectWrap {
 public:
  static bool RegisterJNIUtils(JNIEnv* env);
  WXParams(JNIEnv* env, jobject jni_object);
  virtual ~WXParams();

  base::android::ScopedLocalJavaRef<jstring> GetOsVersion(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetPlatform(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetUseSingleProcess(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetLibJssPath(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetCacheDir(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetWeexVersion(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetDeviceModel(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetAppVersion(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetAppName(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetDeviceWidth(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jstring> GetDeviceHeight(JNIEnv* env);
  base::android::ScopedLocalJavaRef<jobject> GetOptions(JNIEnv* env);

 private:
  DISALLOW_COPY_AND_ASSIGN(WXParams);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_WX_PARAMS_H
