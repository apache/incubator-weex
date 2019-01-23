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

#ifndef WEEX_PROJECT_WML_BRIDGE_H
#define WEEX_PROJECT_WML_BRIDGE_H

#include <jni.h>

#include "base/android/jni/jni_object_wrap.h"
#include "base/common.h"

namespace WeexCore {
class WMLBridge : public JNIObjectWrap {
 public:
  static WMLBridge* Instance() {
    if (g_instance == nullptr) {
      g_instance = new WMLBridge();
    }
    return g_instance;
  }
  static bool RegisterJNIUtils(JNIEnv* env);
  WMLBridge() {}
  virtual ~WMLBridge() {}
  void PostMessage(JNIEnv* env, const char* vm_id, const char* data,
                   int dataLength);
  void DispatchMessage(JNIEnv* env, const char* client_id, const char* data,
                       int dataLength, const char* callback, const char* vm_id);
  base::android::ScopedLocalJavaRef<jbyteArray> DispatchMessageSync(
      JNIEnv* env, const char* client_id, const char* data, int dataLength,
      const char* vm_id);

 private:
  static WMLBridge* g_instance;
  DISALLOW_COPY_AND_ASSIGN(WMLBridge);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_WML_BRIDGE_H
