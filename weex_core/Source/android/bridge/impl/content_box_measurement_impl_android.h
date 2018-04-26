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
#ifndef WEEX_PROJECT_CONTENTBOXMEASUREMENT_IMPL_ANDROID_H
#define WEEX_PROJECT_CONTENTBOXMEASUREMENT_IMPL_ANDROID_H

#include <jni.h>
#include <android/base/jni/android_jni.h>

namespace WeexCore {
  bool RegisterJNIContentBoxMeasurement(JNIEnv *env);

  void cumsmeasure_Imple_Android(JNIEnv *env, jobject measureFunc_Impl_Android,
                                 float width, float height,
                                 int widthMeasureMode, int heightMeasureMode);

  void LayoutBeforeImplAndroid(JNIEnv *env, jobject measureFunc_impl_android);

  void LayoutAfterImplAndroid(JNIEnv *env, jobject measureFunc_impl_android, float width, float height);

  float GetLayoutWidth(JNIEnv *env, jobject measureFunc_impl_android);

  float GetLayoutHeight(JNIEnv *env, jobject measureFunc_impl_android);
}

#endif //WEEX_PROJECT_CONTENTBOXMEASUREMENT_IMPL_ANDROID_H
