// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//
//    /Users/YorkShen/Documents/Code/Weex/weex/android/weex_core_debug/Source/WeexCore/platform/android/jniprebuild/jni_generator.py
// For
//     com/taobao/weex/layout/MeasureSize

#ifndef com_taobao_weex_layout_MeasureSize_JNI
#define com_taobao_weex_layout_MeasureSize_JNI

#include <jni.h>

//#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kMeasureSizeClassPath[] = "com/taobao/weex/layout/MeasureSize";
// Leaking this jclass as we cannot use LazyInstance from some threads.
jclass g_MeasureSize_clazz = NULL;
#define MeasureSize_clazz(env) g_MeasureSize_clazz

}  // namespace

// Step 2: method stubs.

static intptr_t g_MeasureSize_getWidth = 0;
static jfloat Java_MeasureSize_getWidth(JNIEnv* env, jobject obj) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    MeasureSize_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, MeasureSize_clazz(env),
      base::android::INSTANCE_METHOD,
      "getWidth",

"("
")"
"F",
      &g_MeasureSize_getWidth);

  jfloat ret =
      env->CallFloatMethod(obj,
          method_id);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_MeasureSize_getHeight = 0;
static jfloat Java_MeasureSize_getHeight(JNIEnv* env, jobject obj) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    MeasureSize_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, MeasureSize_clazz(env),
      base::android::INSTANCE_METHOD,
      "getHeight",

"("
")"
"F",
      &g_MeasureSize_getHeight);

  jfloat ret =
      env->CallFloatMethod(obj,
          method_id);
  base::android::CheckException(env);
  return ret;
}

// Step 3: RegisterNatives.

static bool RegisterNativesImpl(JNIEnv* env) {

  g_MeasureSize_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kMeasureSizeClassPath).Get()));

  return true;
}

#endif  // com_taobao_weex_layout_MeasureSize_JNI
