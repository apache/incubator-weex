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

#ifndef WEEX_PROJECT_JNI_OBJECT_WRAP_H
#define WEEX_PROJECT_JNI_OBJECT_WRAP_H

#include <jni.h>
#include "android/base/jni/scoped_java_ref.h"
#include "base/common.h"

namespace WeexCore {
class JNIObjectWrap {
 public:
  JNIObjectWrap() {}

  JNIObjectWrap(JNIEnv* env, jobject jni_object)
      : jni_object_(env, jni_object) {}

  virtual ~JNIObjectWrap() {}

  inline void Reset(JNIEnv* env, jobject jni_object) {
    jni_object_.Reset(env, jni_object);
  }

  inline jobject jni_object() { return jni_object_.Get(); }

 private:
  base::android::ScopedGlobalJavaRef<jobject> jni_object_;
  DISALLOW_COPY_AND_ASSIGN(JNIObjectWrap);
};
}  // namespace WeexCore
#endif  // WEEX_PROJECT_JNI_OBJECT_WRAP_H
