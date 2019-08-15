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

#include "base/log_defines.h"
#include "core/network/android/default_request_handler.h"
#include "android/base/string/scoped_jstring_utf8.h"
#include "base/android/jniprebuild/jniheader/RequestHandler_jni.h"
#include "base/android/jni/android_jni.h"
#include "core/manager/weex_core_manager.h"

using namespace weex::core::network;

static void InvokeOnSuccess(JNIEnv* env, jobject jcaller, jlong callback,
                            jstring script,
                            jstring bundleType) {
  CallbackWrapper* callback_wrapper =
      reinterpret_cast<CallbackWrapper*>(callback);
  WeexCore::ScopedJStringUTF8 jni_result(env, script);
  WeexCore::ScopedJStringUTF8 bundleTypeStr(env, bundleType);
  callback_wrapper->Invoke(
      jni_result.getChars() ? jni_result.getChars() : "",
      bundleTypeStr.getChars());
  delete callback_wrapper;
}

static void InvokeOnFailed(JNIEnv* env, jobject jcaller, jlong callback) {
  LOGE_TAG("Eagle", "Download js file using src failed.");
  CallbackWrapper* callback_wrapper =
      reinterpret_cast<CallbackWrapper*>(callback);
  delete callback_wrapper;
  WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(
      "", "JsfmNotInitInEagleMode",
      "JSFramework is not initialized when executing bundle JS in eagle mode");
}

namespace weex {
namespace core {
namespace network {

bool DefaultRequestHandler::RegisterJNIUtils(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

DefaultRequestHandler::DefaultRequestHandler() {
  JNIEnv* env = ::base::android::AttachCurrentThread();
  Reset(env, Java_RequestHandler_create(env).Release());
}

DefaultRequestHandler::~DefaultRequestHandler() {}

void DefaultRequestHandler::Send(const char* instance_id, const char* url,
                                 Callback callback) {
  JNIEnv* env = ::base::android::AttachCurrentThread();
  if (!env) return;
  CallbackWrapper* callback_wrapper = new CallbackWrapper(callback);
  ::base::android::ScopedLocalJavaRef<jstring> jni_url(env,
                                                     env->NewStringUTF(url));
  ::base::android::ScopedLocalJavaRef<jstring> jni_id(
      env, env->NewStringUTF(instance_id));
  Java_RequestHandler_send(env, jni_object(), jni_id.Get(), jni_url.Get(),
                           reinterpret_cast<jlong>(callback_wrapper));
}

void DefaultRequestHandler::GetBundleType(const char *instance_id, const char *content, Callback callback){
  JNIEnv* env = ::base::android::AttachCurrentThread();
  if (!env) return;
  CallbackWrapper* callback_wrapper = new CallbackWrapper(callback);
  ::base::android::ScopedLocalJavaRef<jstring> jni_id(env, env->NewStringUTF(instance_id));
  ::base::android::ScopedLocalJavaRef<jstring> jni_content(env,env->NewStringUTF(content));
  Java_RequestHandler_getBundleType(env, jni_object(), jni_id.Get(), jni_content.Get(),
                           reinterpret_cast<jlong>(callback_wrapper));
}

RequestHandler* RequestHandler::CreateDefaultHandler() {
  return new DefaultRequestHandler();
}
}  // namespace network
}  // namespace core
}  // namespace weex
#endif
