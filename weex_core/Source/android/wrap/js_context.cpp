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

#include <android/base/string/scoped_jstring_utf8.h>
#include <android/base/string/string_utils.h>
#include <IPCMessageJS.h>
#include "js_context.h"
#include "core/manager/weex_core_manager.h"
#include "base/android/jniprebuild/jniheader/JSContext_jni.h"
#include "js_processer.h"

bool WeexCore::JSContext::RegisterJNIUtils(JNIEnv *env) {
  return RegisterNativesImpl(env);
}

static intptr_t g_JSContext_JSActionException = 0;
static void Java_JSContext_JSActionException(JNIEnv *env,
                                             jobject obj,
                                             jstring exception) {
  jmethodID method_id =
      base::android::GetMethod(
          env, g_JSContext_clazz,
          base::android::INSTANCE_METHOD,
          "Exception",
          "("
          "Ljava/lang/String;"
          ")"
          "V",
          &g_JSContext_JSActionException);
  if (method_id == nullptr) {
    return;
  }
  env->CallVoidMethod(obj, method_id, exception);
  base::android::CheckException(env);
}

static intptr_t g_JSContext_JSActionCallBack = 0;
static jstring Java_JSContext_JSActionCallBack(JNIEnv *env,
                                               jobject obj,
                                               jstring method,
                                               jstring arg) {
  jmethodID method_id =
      base::android::GetMethod(
          env, g_JSContext_clazz,
          base::android::INSTANCE_METHOD,
          "Invoke",
          "("
          "Ljava/lang/String;"
          "Ljava/lang/String;"
          ")"
          "Ljava/lang/String;",
          &g_JSContext_JSActionCallBack);
  if (method_id == nullptr) {
    return nullptr;
  }
  jstring ret = (jstring) env->CallObjectMethod(obj, method_id, method, arg);
  base::android::CheckException(env);
  return ret;
}
void JSContext::JsActionException(const char *exception) {
  JNIEnv *env = ::base::android::AttachCurrentThread();
  auto exception_str = base::android::ScopedLocalJavaRef<jstring>(
      env, newJString(env, exception));
  Java_JSContext_JSActionException(env, jni_object(), exception_str.Get());
}
char *JSContext::JsActionCallBack(const char *method, const char *args) {
  JNIEnv *env = ::base::android::AttachCurrentThread();
  auto method_str = base::android::ScopedLocalJavaRef<jstring>(
      env, newJString(env, method));
  auto args_str = base::android::ScopedLocalJavaRef<jstring>(
      env, newJString(env, args));
  jstring pJstring =
      Java_JSContext_JSActionCallBack(env, jni_object(), method_str.Get(), args_str.Get());
  if (pJstring != nullptr) {
    ScopedJStringUTF8 func_str(env, pJstring);
    char *m = new char[strlen(func_str.getChars()) + 1];
    memset(m, 0, strlen(func_str.getChars()) + 1);
    memcpy(m, func_str.getChars(), strlen(func_str.getChars()));
    return const_cast<char *>(m);
  } else {
    return nullptr;
  }
}

static jlong nativeCreateContext(JNIEnv *env, jobject jcaller) {
  JNIEnv *env_tmp = ::base::android::AttachCurrentThread();

  auto ptr_js_context = new WeexCore::JSContext();
  ptr_js_context->Reset(env_tmp, jcaller);

  auto ptr_container = android::JSContainerProcesser::CreateJSPtrContainer();
  (*ptr_container)->m_ptr_weexcore_js_context = (long) ptr_js_context;
  long long_ptr_ptr_container = (long) ptr_container;
  int64_t jssPtr = WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()->JsAction(long_ptr_ptr_container, static_cast<int32_t>(JSACTION::CREATE), "");
  (*ptr_container)->m_ptr_jss_js_action = (long) jssPtr;
  return long_ptr_ptr_container;
}

static void nativeDestroyContext(JNIEnv *env, jobject jcaller,
                                 jlong ctxContainer) {
  JNIEnv *env_tmp = ::base::android::AttachCurrentThread();

  auto pContext = android::JSContainerProcesser::ExtraJsContext((long) ctxContainer);
  if (pContext == nullptr) {
    return;
  }
  if (!pContext->jni_object()) {
    pContext->Reset(env_tmp, jcaller);
  }
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->JsAction(
      (long) ctxContainer, static_cast<int32_t>(JSACTION::DESTROY), "");

  android::JSContainerProcesser::DestroyJSPtrContainer((long) ctxContainer);

}

static void nativeBindFunc(JNIEnv *env, jobject jcaller,
                           jlong ctxContainer,
                           jstring func) {
  JNIEnv *env_tmp = ::base::android::AttachCurrentThread();
  ScopedJStringUTF8 func_str(env_tmp, func);

  JSContext *pContext = android::JSContainerProcesser::ExtraJsContext((long) ctxContainer);
  if (pContext == nullptr) {
    return;
  }

  if (!pContext->jni_object()) {
    pContext->Reset(env_tmp, jcaller);
  }

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()->JsAction((long) ctxContainer,
                              static_cast<int32_t >(JSACTION::BIND),
                              func_str.getChars());
}

static void nativeUnBindFunc(JNIEnv *env, jobject jcaller,
                             jlong ctxContainer,
                             jstring func) {
  JNIEnv *env_tmp = ::base::android::AttachCurrentThread();

  JSContext *pContext = android::JSContainerProcesser::ExtraJsContext((long) ctxContainer);

  if (pContext == nullptr) {
    return;
  }

  if (!pContext->jni_object()) {
    pContext->Reset(env_tmp, jcaller);
  }

  ScopedJStringUTF8 func_str(env_tmp, func);
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()->JsAction((long) ctxContainer,
                              static_cast<int32_t >(JSACTION::UNBIND),
                              func_str.getChars());
}

static void nativeExecJS(JNIEnv *env, jobject jcaller,
                         jlong ctxContainer,
                         jstring script) {
  JNIEnv *env_tmp = ::base::android::AttachCurrentThread();
  JSContext *pContext = android::JSContainerProcesser::ExtraJsContext((long) ctxContainer);

  if (pContext == nullptr) {
    return;
  }

  if (!pContext->jni_object()) {
    pContext->Reset(env_tmp, jcaller);
  }

  ScopedJStringUTF8 func_str(env_tmp, script);
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()->JsAction((long) ctxContainer,
                              static_cast<int32_t >(JSACTION::EXEJS),
                              func_str.getChars());
}
