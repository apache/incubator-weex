// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     incubator-weex/weex_core/Source/android/jniprebuild/jni_generator.py
// For
//     com/taobao/weex/bridge/WXBridge

#ifndef com_taobao_weex_bridge_WXBridge_JNI
#define com_taobao_weex_bridge_WXBridge_JNI

#include <jni.h>
#include <android/base/jni/android_jni.h>

//#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kWXBridgeClassPath[] = "com/taobao/weex/bridge/WXBridge";
// Leaking this jclass as we cannot use LazyInstance from some threads.
jclass g_WXBridge_clazz = NULL;
#define WXBridge_clazz(env) g_WXBridge_clazz

}  // namespace

static jint InitFrameworkEnv(JNIEnv* env, jobject jcaller,
    jstring framework,
    jobject params,
    jstring cacheDir,
    jboolean pieSupport);

static jint InitFramework(JNIEnv* env, jobject jcaller,
    jstring framework,
    jobject params);

static void RefreshInstance(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring _namespace,
    jstring function,
    jobjectArray args);

static jint ExecJS(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring name,
    jstring function,
    jobjectArray args);

static jint ExecJSService(JNIEnv* env, jobject jcaller,
    jstring javascript);

static void ExecJSWithCallback(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring _namespace,
    jstring _function,
    jobjectArray args,
    jlong callbackId);

static jint CreateInstanceContext(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring name,
    jstring function,
    jobjectArray args);

static jint DestoryInstance(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring name,
    jstring function,
    jobjectArray args);

static jstring ExecJSOnInstance(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring script,
    jint type);

static void FireEventOnDataRenderNode(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jstring type,
    jstring data);

static void RegisterModuleOnDataRenderNode(JNIEnv* env, jobject jcaller,
    jstring data);

static void TakeHeapSnapshot(JNIEnv* env, jobject jcaller,
    jstring filename);

static void BindMeasurementToRenderObject(JNIEnv* env, jobject jcaller,
    jlong ptr);

static void SetRenderContainerWrapContent(JNIEnv* env, jobject jcaller,
    jboolean wrap,
    jstring instanceId);

static jlongArray GetFirstScreenRenderTime(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static jlongArray GetRenderFinishTime(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static void SetDefaultHeightAndWidthIntoRootDom(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jfloat defaultWidth,
    jfloat defaultHeight,
    jboolean isWidthWrapContent,
    jboolean isHeightWrapContent);

static void OnInstanceClose(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static void ForceLayout(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static jboolean NotifyLayout(JNIEnv* env, jobject jcaller,
    jstring instanceId);

static void SetStyleWidth(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jfloat value);

static void SetStyleHeight(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jfloat value);

static void SetMargin(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jint edge,
    jfloat value);

static void SetPadding(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jint edge,
    jfloat value);

static void SetPosition(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jint edge,
    jfloat value);

static void MarkDirty(JNIEnv* env, jobject jcaller,
    jstring instanceId,
    jstring ref,
    jboolean dirty);

static void RegisterCoreEnv(JNIEnv* env, jobject jcaller,
    jstring key,
    jstring value);

static void ResetWXBridge(JNIEnv* env, jobject jcaller,
    jobject bridge,
    jstring className);

static void UpdateGlobalConfig(JNIEnv* env, jobject jcaller,
    jstring config);

// Step 2: method stubs.

static intptr_t g_WXBridge_onReceivedResult = 0;
static void Java_WXBridge_onReceivedResult(JNIEnv* env, jobject obj, jlong
    callbackId,
    jbyteArray result) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "onReceivedResult",

"("
"J"
"[B"
")"
"V",
      &g_WXBridge_onReceivedResult);

     env->CallVoidMethod(obj,
          method_id, callbackId, result);
  base::android::CheckException(env);

}

static intptr_t g_WXBridge_callNative = 0;
static jint Java_WXBridge_callNative(JNIEnv* env, jobject obj, jstring
    instanceId,
    jbyteArray tasks,
    jstring callback) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callNative",

"("
"Ljava/lang/String;"
"[B"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callNative);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, tasks, callback);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_reportJSException = 0;
static void Java_WXBridge_reportJSException(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring func,
    jstring exception) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "reportJSException",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V",
      &g_WXBridge_reportJSException);

     env->CallVoidMethod(obj,
          method_id, instanceId, func, exception);
  base::android::CheckException(env);

}

static intptr_t g_WXBridge_callNativeModule = 0;
static base::android::ScopedLocalJavaRef<jobject>
    Java_WXBridge_callNativeModule(JNIEnv* env, jobject obj, jstring instanceId,
    jstring module,
    jstring method,
    jbyteArray arguments,
    jbyteArray options) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), NULL);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callNativeModule",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[B"
"[B"
")"
"Ljava/lang/Object;",
      &g_WXBridge_callNativeModule);

  jobject ret =
      env->CallObjectMethod(obj,
          method_id, instanceId, module, method, arguments, options);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jobject>(env, ret);
}

static intptr_t g_WXBridge_callNativeComponent = 0;
static void Java_WXBridge_callNativeComponent(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    jstring method,
    jbyteArray arguments,
    jbyteArray optionsData) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callNativeComponent",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[B"
"[B"
")"
"V",
      &g_WXBridge_callNativeComponent);

     env->CallVoidMethod(obj,
          method_id, instanceId, ref, method, arguments, optionsData);
  base::android::CheckException(env);

}

static intptr_t g_WXBridge_setTimeoutNative = 0;
static void Java_WXBridge_setTimeoutNative(JNIEnv* env, jobject obj, jstring
    callbackId,
    jstring time) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "setTimeoutNative",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V",
      &g_WXBridge_setTimeoutNative);

     env->CallVoidMethod(obj,
          method_id, callbackId, time);
  base::android::CheckException(env);

}

static intptr_t g_WXBridge_setJSFrmVersion = 0;
static void Java_WXBridge_setJSFrmVersion(JNIEnv* env, jobject obj, jstring
    version) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "setJSFrmVersion",

"("
"Ljava/lang/String;"
")"
"V",
      &g_WXBridge_setJSFrmVersion);

     env->CallVoidMethod(obj,
          method_id, version);
  base::android::CheckException(env);

}

static intptr_t g_WXBridge_callUpdateFinish = 0;
static jint Java_WXBridge_callUpdateFinish(JNIEnv* env, jobject obj, jstring
    instanceId,
    jbyteArray tasks,
    jstring callback) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callUpdateFinish",

"("
"Ljava/lang/String;"
"[B"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callUpdateFinish);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, tasks, callback);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callRefreshFinish = 0;
static jint Java_WXBridge_callRefreshFinish(JNIEnv* env, jobject obj, jstring
    instanceId,
    jbyteArray tasks,
    jstring callback) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callRefreshFinish",

"("
"Ljava/lang/String;"
"[B"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callRefreshFinish);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, tasks, callback);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_reportServerCrash = 0;
static void Java_WXBridge_reportServerCrash(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring crashFile) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "reportServerCrash",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V",
      &g_WXBridge_reportServerCrash);

     env->CallVoidMethod(obj,
          method_id, instanceId, crashFile);
  base::android::CheckException(env);

}

static intptr_t g_WXBridge_callCreateBody = 0;
static jint Java_WXBridge_callCreateBody(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring componentType,
    jstring ref,
    jobject styles,
    jobject attributes,
    jobject events,
    jfloatArray margins,
    jfloatArray paddings,
    jfloatArray borders) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callCreateBody",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/util/HashMap;"
"Ljava/util/HashMap;"
"Ljava/util/HashSet;"
"[F"
"[F"
"[F"
")"
"I",
      &g_WXBridge_callCreateBody);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, componentType, ref, styles, attributes, events,
              margins, paddings, borders);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callAddElement = 0;
static jint Java_WXBridge_callAddElement(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring componentType,
    jstring ref,
    int index,
    jstring parentRef,
    jobject styles,
    jobject attributes,
    jobject events,
    jfloatArray margins,
    jfloatArray paddings,
    jfloatArray borders,
    jboolean willLayout) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callAddElement",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
"Ljava/lang/String;"
"Ljava/util/HashMap;"
"Ljava/util/HashMap;"
"Ljava/util/HashSet;"
"[F"
"[F"
"[F"
"Z"
")"
"I",
      &g_WXBridge_callAddElement);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, componentType, ref, int(index), parentRef,
              styles, attributes, events, margins, paddings, borders,
              willLayout);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callRemoveElement = 0;
static jint Java_WXBridge_callRemoveElement(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callRemoveElement",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callRemoveElement);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callMoveElement = 0;
static jint Java_WXBridge_callMoveElement(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    jstring parentref,
    int index) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callMoveElement",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
")"
"I",
      &g_WXBridge_callMoveElement);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, parentref, int(index));
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callAddEvent = 0;
static jint Java_WXBridge_callAddEvent(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    jstring event) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callAddEvent",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callAddEvent);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, event);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callRemoveEvent = 0;
static jint Java_WXBridge_callRemoveEvent(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    jstring event) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callRemoveEvent",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callRemoveEvent);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, event);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callUpdateStyle = 0;
static jint Java_WXBridge_callUpdateStyle(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    jobject styles,
    jobject paddings,
    jobject margins,
    jobject borders) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callUpdateStyle",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/util/HashMap;"
"Ljava/util/HashMap;"
"Ljava/util/HashMap;"
"Ljava/util/HashMap;"
")"
"I",
      &g_WXBridge_callUpdateStyle);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, styles, paddings, margins, borders);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callUpdateAttrs = 0;
static jint Java_WXBridge_callUpdateAttrs(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    jobject attrs) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callUpdateAttrs",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/util/HashMap;"
")"
"I",
      &g_WXBridge_callUpdateAttrs);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, attrs);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callLayout = 0;
static jint Java_WXBridge_callLayout(JNIEnv* env, jobject obj, jstring
    instanceId,
    jstring ref,
    int top,
    int bottom,
    int left,
    int right,
    int height,
    int width,
    int index) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callLayout",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
"I"
"I"
"I"
"I"
"I"
"I"
")"
"I",
      &g_WXBridge_callLayout);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, int(top), int(bottom), int(left),
              int(right), int(height), int(width), int(index));
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callCreateFinish = 0;
static jint Java_WXBridge_callCreateFinish(JNIEnv* env, jobject obj, jstring
    instanceId) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callCreateFinish",

"("
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callCreateFinish);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callRenderSuccess = 0;
static jint Java_WXBridge_callRenderSuccess(JNIEnv* env, jobject obj, jstring
    instanceId) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callRenderSuccess",

"("
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callRenderSuccess);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callAppendTreeCreateFinish = 0;
static jint Java_WXBridge_callAppendTreeCreateFinish(JNIEnv* env, jobject obj,
    jstring instanceId,
    jstring ref) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callAppendTreeCreateFinish",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"I",
      &g_WXBridge_callAppendTreeCreateFinish);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_callHasTransitionPros = 0;
static jint Java_WXBridge_callHasTransitionPros(JNIEnv* env, jobject obj,
    jstring instanceId,
    jstring ref,
    jobject styles) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), 0);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "callHasTransitionPros",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/util/HashMap;"
")"
"I",
      &g_WXBridge_callHasTransitionPros);

  jint ret =
      env->CallIntMethod(obj,
          method_id, instanceId, ref, styles);
  base::android::CheckException(env);
  return ret;
}

static intptr_t g_WXBridge_getMeasurementFunc = 0;
static base::android::ScopedLocalJavaRef<jobject>
    Java_WXBridge_getMeasurementFunc(JNIEnv* env, jobject obj, jstring
    instanceId,
    jlong renderObjectPtr) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env), NULL);
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "getMeasurementFunc",

"("
"Ljava/lang/String;"
"J"
")"
"Lcom/taobao/weex/layout/ContentBoxMeasurement;",
      &g_WXBridge_getMeasurementFunc);

  jobject ret =
      env->CallObjectMethod(obj,
          method_id, instanceId, renderObjectPtr);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jobject>(env, ret);
}

static intptr_t g_WXBridge_reportNativeInitStatus = 0;
static void Java_WXBridge_reportNativeInitStatus(JNIEnv* env, jobject obj,
    jstring statusCode,
    jstring errorMsg) {
  /* Must call RegisterNativesImpl()  */
  //CHECK_CLAZZ(env, obj,
  //    WXBridge_clazz(env));
  jmethodID method_id =
      base::android::GetMethod(
      env, WXBridge_clazz(env),
      base::android::INSTANCE_METHOD,
      "reportNativeInitStatus",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V",
      &g_WXBridge_reportNativeInitStatus);

     env->CallVoidMethod(obj,
          method_id, statusCode, errorMsg);
  base::android::CheckException(env);

}

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsWXBridge[] = {
    { "nativeInitFrameworkEnv",
"("
"Ljava/lang/String;"
"Lcom/taobao/weex/bridge/WXParams;"
"Ljava/lang/String;"
"Z"
")"
"I", reinterpret_cast<void*>(InitFrameworkEnv) },
    { "nativeInitFramework",
"("
"Ljava/lang/String;"
"Lcom/taobao/weex/bridge/WXParams;"
")"
"I", reinterpret_cast<void*>(InitFramework) },
    { "nativeRefreshInstance",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[Lcom/taobao/weex/bridge/WXJSObject;"
")"
"V", reinterpret_cast<void*>(RefreshInstance) },
    { "nativeExecJS",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[Lcom/taobao/weex/bridge/WXJSObject;"
")"
"I", reinterpret_cast<void*>(ExecJS) },
    { "nativeExecJSService",
"("
"Ljava/lang/String;"
")"
"I", reinterpret_cast<void*>(ExecJSService) },
    { "nativeExecJSWithCallback",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[Lcom/taobao/weex/bridge/WXJSObject;"
"J"
")"
"V", reinterpret_cast<void*>(ExecJSWithCallback) },
    { "nativeCreateInstanceContext",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[Lcom/taobao/weex/bridge/WXJSObject;"
")"
"I", reinterpret_cast<void*>(CreateInstanceContext) },
    { "nativeDestoryInstance",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[Lcom/taobao/weex/bridge/WXJSObject;"
")"
"I", reinterpret_cast<void*>(DestoryInstance) },
    { "nativeExecJSOnInstance",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
")"
"Ljava/lang/String;", reinterpret_cast<void*>(ExecJSOnInstance) },
    { "nativeFireEventOnDataRenderNode",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(FireEventOnDataRenderNode) },
    { "nativeRegisterModuleOnDataRenderNode",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(RegisterModuleOnDataRenderNode) },
    { "nativeTakeHeapSnapshot",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(TakeHeapSnapshot) },
    { "nativeBindMeasurementToRenderObject",
"("
"J"
")"
"V", reinterpret_cast<void*>(BindMeasurementToRenderObject) },
    { "nativeSetRenderContainerWrapContent",
"("
"Z"
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(SetRenderContainerWrapContent) },
    { "nativeGetFirstScreenRenderTime",
"("
"Ljava/lang/String;"
")"
"[J", reinterpret_cast<void*>(GetFirstScreenRenderTime) },
    { "nativeGetRenderFinishTime",
"("
"Ljava/lang/String;"
")"
"[J", reinterpret_cast<void*>(GetRenderFinishTime) },
    { "nativeSetDefaultHeightAndWidthIntoRootDom",
"("
"Ljava/lang/String;"
"F"
"F"
"Z"
"Z"
")"
"V", reinterpret_cast<void*>(SetDefaultHeightAndWidthIntoRootDom) },
    { "nativeOnInstanceClose",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(OnInstanceClose) },
    { "nativeForceLayout",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(ForceLayout) },
    { "nativeNotifyLayout",
"("
"Ljava/lang/String;"
")"
"Z", reinterpret_cast<void*>(NotifyLayout) },
    { "nativeSetStyleWidth",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"F"
")"
"V", reinterpret_cast<void*>(SetStyleWidth) },
    { "nativeSetStyleHeight",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"F"
")"
"V", reinterpret_cast<void*>(SetStyleHeight) },
    { "nativeSetMargin",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
"F"
")"
"V", reinterpret_cast<void*>(SetMargin) },
    { "nativeSetPadding",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
"F"
")"
"V", reinterpret_cast<void*>(SetPadding) },
    { "nativeSetPosition",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"I"
"F"
")"
"V", reinterpret_cast<void*>(SetPosition) },
    { "nativeMarkDirty",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
"Z"
")"
"V", reinterpret_cast<void*>(MarkDirty) },
    { "nativeRegisterCoreEnv",
"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(RegisterCoreEnv) },
    { "nativeResetWXBridge",
"("
"Ljava/lang/Object;"
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(ResetWXBridge) },
    { "nativeUpdateGlobalConfig",
"("
"Ljava/lang/String;"
")"
"V", reinterpret_cast<void*>(UpdateGlobalConfig) },
};

static bool RegisterNativesImpl(JNIEnv* env) {

  g_WXBridge_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kWXBridgeClassPath).Get()));

  const int kMethodsWXBridgeSize =
      sizeof(kMethodsWXBridge)/sizeof(kMethodsWXBridge[0]);

  if (env->RegisterNatives(WXBridge_clazz(env),
                           kMethodsWXBridge,
                           kMethodsWXBridgeSize) < 0) {
    //jni_generator::HandleRegistrationError(
    //    env, WXBridge_clazz(env), __FILE__);
    return false;
  }

  return true;
}

static void Java_WXBridge_reset_clazz(JNIEnv* env, const char* className) {
    LOGE("Java_WXBridge_reset_clazz class Name is %s", className);
    g_WXBridge_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
                    base::android::GetClass(env, className).Get()));
                            g_WXBridge_callNative = 0;
    g_WXBridge_reportJSException = 0;
    g_WXBridge_callNativeModule = 0;
    g_WXBridge_callNativeComponent = 0;
    g_WXBridge_setTimeoutNative = 0;
    g_WXBridge_setJSFrmVersion = 0;
    g_WXBridge_callUpdateFinish = 0;
    g_WXBridge_callRefreshFinish = 0;
    g_WXBridge_reportServerCrash = 0;
    g_WXBridge_callCreateBody = 0;
    g_WXBridge_callAddElement = 0;
    g_WXBridge_callRemoveElement = 0;
    g_WXBridge_callMoveElement = 0;
    g_WXBridge_callAddEvent = 0;
    g_WXBridge_callRemoveEvent = 0;
    g_WXBridge_callUpdateStyle = 0;
    g_WXBridge_callUpdateAttrs = 0;
    g_WXBridge_callLayout = 0;
    g_WXBridge_callCreateFinish = 0;
    g_WXBridge_callRenderSuccess = 0;
    g_WXBridge_callAppendTreeCreateFinish = 0;
    g_WXBridge_callHasTransitionPros = 0;
    g_WXBridge_getMeasurementFunc = 0;
    g_WXBridge_reportNativeInitStatus = 0;
}

#endif  // com_taobao_weex_bridge_WXBridge_JNI
