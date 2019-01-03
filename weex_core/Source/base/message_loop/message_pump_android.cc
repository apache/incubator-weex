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

#include "base/message_loop/message_pump_android.h"

#include "base/android/jniprebuild/jniheader/SystemMessageHandler_jni.h"
#include "base/android/jni/android_jni.h"

void RunWork(JNIEnv* env, jobject jcaller, jlong delegateNative) {
  weex::base::MessagePump::Delegate* delegate =
      reinterpret_cast<weex::base::MessagePump::Delegate*>(delegateNative);
  if (delegate != NULL) {
    delegate->DoWork();
  }
}

namespace weex {
namespace base {

MessagePumpAndroid::MessagePumpAndroid() {}

MessagePumpAndroid::~MessagePumpAndroid() {}

bool MessagePumpAndroid::RegisterJNIUtils(JNIEnv* env) {
  if(!env) {
    return false; 
  }

  return RegisterNativesImpl(env);
}

void MessagePumpAndroid::Run(Delegate* delegate) {
  JNIEnv* env = ::base::android::AttachCurrentThread();

  if(!env) {
    return;
  }

  Reset(env, Java_SystemMessageHandler_create(
                 env, reinterpret_cast<int64_t>(delegate))
                 .Release());
}

void MessagePumpAndroid::Stop() {
  JNIEnv* env = ::base::android::AttachCurrentThread();
  if(!env || !jni_object()) {
    return;
  }

  Java_SystemMessageHandler_stop(env, jni_object());
}

void MessagePumpAndroid::ScheduleWork() {
  JNIEnv* env = ::base::android::AttachCurrentThread();

  if(!env || !jni_object()) {
    return;
  }

  Java_SystemMessageHandler_scheduleWork(env, jni_object());
}

void MessagePumpAndroid::ScheduleDelayedWork(TimeUnit delayed_time) {
  JNIEnv* env = ::base::android::AttachCurrentThread();

  if(!env || !jni_object()) {
    return;
  }

  Java_SystemMessageHandler_scheduleDelayedWork(env, jni_object(),
                                                delayed_time.ToMilliseconds());
}
}  // namespace base
}  // namespace weex
