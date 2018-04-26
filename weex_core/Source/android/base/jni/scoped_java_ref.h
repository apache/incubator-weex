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
// Copyright 2017 The WeexCore Authors. All rights reserved.

#ifndef WeexCore_BASE_ANDROID_SCOPED_JAVA_REF_H_
#define WeexCore_BASE_ANDROID_SCOPED_JAVA_REF_H_

#include <jni.h>

namespace base {
namespace android {
class JavaRef {
public:
    JavaRef();

    JavaRef(JNIEnv *env, jobject obj);

    ~JavaRef() {}

    void ResetNewLocalRef(JNIEnv *env, jobject obj);

    void ReleaseLocalRef(JNIEnv *env);

    void ResetNewGlobalRef(JNIEnv *env, jobject obj);

    void ReleaseGlobalRef(JNIEnv *env);

    jobject Release();

    jobject Get() const { return obj_; }

    bool IsNull() const { return obj_ == nullptr; }

protected:
    jobject obj_;
};

template<typename T>
class ScopedLocalJavaRef : public JavaRef {
public:
    ScopedLocalJavaRef() : env_(nullptr) {}

    ScopedLocalJavaRef(JNIEnv *env, T obj) : env_(env), JavaRef(env, obj) {
    }

    ScopedLocalJavaRef(const ScopedLocalJavaRef<T> &other) : env_(other.env_) {
      Reset(env_, other.Get());
    }

    void operator=(const ScopedLocalJavaRef<T> &other) {
      env_ = other.env_;
      Reset(env_, other.Get());
    }

    ~ScopedLocalJavaRef() {
      ReleaseLocalRef(env_);
    }

    void Reset(JNIEnv *env, jobject obj) {
      ResetNewLocalRef(env, obj);
    }

private:
    JNIEnv *env_;
};

template<typename T>
class ScopedGlobalJavaRef : public JavaRef {
public:
    ScopedGlobalJavaRef() {}

    ScopedGlobalJavaRef(JNIEnv *env, T obj) {
      Reset(env, obj);
    }

    ScopedGlobalJavaRef(const ScopedGlobalJavaRef<T> &other) {
      Reset(nullptr, other.Get());
    }

    ScopedGlobalJavaRef(
            const ScopedLocalJavaRef<T> &other) {
      Reset(nullptr, other.Get());
    }

    ~ScopedGlobalJavaRef() {
      ReleaseGlobalRef(nullptr);
    }

    void operator=(const ScopedGlobalJavaRef<T> &other) {
      Reset(nullptr, other.Get());
    }

    void Reset(JNIEnv *env, const ScopedLocalJavaRef<T> &other) {
      ResetNewGlobalRef(env, other.Get());
    }

    void Reset(JNIEnv *env, jobject obj) {
      ResetNewGlobalRef(env, obj);
    }
};

}  // namespace android
}  // namespace base


#endif  // WeexCore_BASE_ANDROID_SCOPED_JAVA_REF_H_
