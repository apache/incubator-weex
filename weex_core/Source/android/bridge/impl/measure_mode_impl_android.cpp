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
#include <android/base/jni/android_jni.h>
#include "measure_mode_impl_android.h"
#include <android/jniprebuild/jniheader/MeasureMode_jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env) {
    RegisterNativesImpl(env);
  }

  jint Exactly(JNIEnv *env) {
    return Java_MeasureMode_getEXACTLY(env);
  }

  jint Unspecified(JNIEnv *env) {
    return Java_MeasureMode_getUNSPECIFIED(env);
  }
}

