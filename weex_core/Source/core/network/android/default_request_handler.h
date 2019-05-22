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
#if OS_ANDROID

#ifndef CORE_NETWORK_DEFAULT_REQUEST_HANDLER_H
#define CORE_NETWORK_DEFAULT_REQUEST_HANDLER_H

#include <jni.h>
#include "base/android/jni/jni_object_wrap.h"
#include "core/network/request_handler.h"

namespace weex {
namespace core {
namespace network {

class DefaultRequestHandler : public RequestHandler,
                              public WeexCore::JNIObjectWrap {
 public:
  static bool RegisterJNIUtils(JNIEnv* env);
  DefaultRequestHandler();
  ~DefaultRequestHandler() override;
  void Send(const char* instance_id, const char* url,
            Callback callback) override;
};

class CallbackWrapper {
 public:
  CallbackWrapper(Callback callback) : callback_(callback) {}
  ~CallbackWrapper() {}
  void Invoke(const std::string& result, const std::string& bundleType) { callback_(result, bundleType); }

 private:
  Callback callback_;
};

}  // namespace network
}  // namespace core
}  // namespace weex
#endif  // CORE_NETWORK_DEFAULT_REQUEST_HANDLER_H
#endif
