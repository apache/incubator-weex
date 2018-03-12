// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     /Users/miomin/Desktop/workspace/alibaba/WeexCore-dev/WeexCore-
//    dev/android/weex_core_debug/Source/WeexCore/platform/android/jniprebuild/jni_generator.py
// For
//     com/taobao/weex/WXSDKInstance

#ifndef com_taobao_weex_WXSDKInstance_JNI
#define com_taobao_weex_WXSDKInstance_JNI

#include <jni.h>

//#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kWXSDKInstanceClassPath[] = "com/taobao/weex/WXSDKInstance";
// Leaking this jclass as we cannot use LazyInstance from some threads.
jclass g_WXSDKInstance_clazz = NULL;
#define WXSDKInstance_clazz(env) g_WXSDKInstance_clazz

}  // namespace

static jboolean NotifyLayout(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static void ForceLayout(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static void BindComponentToWXCore(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jobject component,
    jstring ref);

static void OnInstanceClose(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static void SetDefaultHeightAndWidthIntoRootDom(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jfloat defaultWidth,
    jfloat defaultHeight);

static jint PrintFirstScreenRenderTime(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static jint PrintRenderFinishTime(JNIEnv* env, jobject jcaller,
    jstring instanceId);

// Step 2: method stubs.

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsWXSDKInstance[] = {
    { "nativeNotifyLayout",
"("
"Ljava/lang/String;"
")"
"Z", reinterpret_cast<void*>(NotifyLayout) },
    { "nativeForceLayout",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(ForceLayout) },
    { "nativeBindComponentToWXCore",
"("
"Ljava/lang/String;"
"Lcom/taobao/weex/ui/component/WXComponent;"
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(BindComponentToWXCore) },
    { "nativeOnInstanceClose",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(OnInstanceClose) },
    { "nativeSetDefaultHeightAndWidthIntoRootDom",
"("
"Ljava/lang/String;"
"F"
"F"
")"
"V", reinterpret_cast<void*>(SetDefaultHeightAndWidthIntoRootDom) },
    { "nativePrintFirstScreenRenderTime",
"("
"Ljava/lang/String;"
")"
"I", reinterpret_cast<void*>(PrintFirstScreenRenderTime) },
    { "nativePrintRenderFinishTime",
"("
"Ljava/lang/String;"
")"
"I", reinterpret_cast<void*>(PrintRenderFinishTime) },
};

static bool RegisterNativesImpl(JNIEnv* env) {

  g_WXSDKInstance_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kWXSDKInstanceClassPath).Get()));

  const int kMethodsWXSDKInstanceSize =
      sizeof(kMethodsWXSDKInstance)/sizeof(kMethodsWXSDKInstance[0]);

  if (env->RegisterNatives(WXSDKInstance_clazz(env),
                           kMethodsWXSDKInstance,
                           kMethodsWXSDKInstanceSize) < 0) {
    //jni_generator::HandleRegistrationError(
    //    env, WXSDKInstance_clazz(env), __FILE__);
    return false;
  }

  return true;
}

#endif  // com_taobao_weex_WXSDKInstance_JNI
