// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     /Users/miomin/Desktop/workspace/alibaba/WeexCore-dev/WeexCore-
//    dev/android/weex_core_debug/Source/WeexCore/platform/android/jniprebuild/jni_generator.py
// For
//     com/taobao/weex/ui/component/WXComponent

#ifndef com_taobao_weex_ui_component_WXComponent_JNI
#define com_taobao_weex_ui_component_WXComponent_JNI

#include <jni.h>

//#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kWXComponentClassPath[] = "com/taobao/weex/ui/component/WXComponent";
// Leaking this jclass as we cannot use LazyInstance from some threads.
jclass g_WXComponent_clazz = NULL;
#define WXComponent_clazz(env) g_WXComponent_clazz

}  // namespace

static void BindMeasurementToWXCore(JNIEnv* env, jobject jcaller,
    jint instanceId,
    jint ref,
    jobject contentBoxMeasurement);

// Step 2: method stubs.

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsWXComponent[] = {
    { "nativeBindMeasurementToWXCore",
"("
"I"
"I"
"Lcom/taobao/weex/layout/ContentBoxMeasurement;"
")"
"V", reinterpret_cast<void*>(BindMeasurementToWXCore) },
};

static bool RegisterNativesImpl(JNIEnv* env) {

  g_WXComponent_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kWXComponentClassPath).Get()));

  const int kMethodsWXComponentSize =
      sizeof(kMethodsWXComponent)/sizeof(kMethodsWXComponent[0]);

  if (env->RegisterNatives(WXComponent_clazz(env),
                           kMethodsWXComponent,
                           kMethodsWXComponentSize) < 0) {
    //jni_generator::HandleRegistrationError(
    //    env, WXComponent_clazz(env), __FILE__);
    return false;
  }

  return true;
}

#endif  // com_taobao_weex_ui_component_WXComponent_JNI
