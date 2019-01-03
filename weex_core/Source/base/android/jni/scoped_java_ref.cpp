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
// Copyright 2017 The Weex Authors. All rights reserved.

#include "base/android/jni/scoped_java_ref.h"

#include "base/android/jni/android_jni.h"

namespace base {
namespace android {

JavaRef::JavaRef() : obj_(nullptr) {}

JavaRef::JavaRef(JNIEnv *env, jobject obj) : obj_(obj) {}

void JavaRef::ResetNewLocalRef(JNIEnv *env, jobject obj) {
  if (!env) {
    env = AttachCurrentThread();
  }

  if (env == nullptr)
    return;

  if (obj) {
    obj = env->NewLocalRef(obj);
  }

  if (obj_)
    env->DeleteLocalRef(obj_);
  obj_ = obj;
}

void JavaRef::ReleaseLocalRef(JNIEnv *env) {
  if (!env) {
    env = AttachCurrentThread();
  }

  if (env == nullptr)
    return;

  env->DeleteLocalRef(obj_);
}

void JavaRef::ResetNewGlobalRef(JNIEnv *env, jobject obj) {
  if (!env) {
    env = AttachCurrentThread();
  }
  if (obj)
    obj = env->NewGlobalRef(obj);
  if (obj_)
    env->DeleteGlobalRef(obj_);
  obj_ = obj;
}

void JavaRef::ReleaseGlobalRef(JNIEnv *env) {
  if (!env) {
    env = AttachCurrentThread();
  }

  if (env == nullptr)
    return;

  env->DeleteGlobalRef(obj_);
}

jobject JavaRef::Release() {
  jobject obj = obj_;
  obj_ = nullptr;
  return obj;
}
}  // namespace android
}  // namespace base
