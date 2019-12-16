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

#ifndef WEEX_PROJECT_JSCONTEXT_JNI_H
#define WEEX_PROJECT_JSCONTEXT_JNI_H
#include <jni.h>
#include "base/android/jni/android_jni.h"

const char kJSContextClassPath[] = "org/apache/weex/jsEngine/JSContext";
// Leaking this jclass as we cannot use LazyInstance from some threads.
jclass g_JSContext_clazz = NULL;

/*
   public native void nativeBindFunc(String name);

    public native void nativeUnBindFunc(String name);

    public native void nativeExecJS(String script);
 */

static jlong nativeCreateContext(JNIEnv *env, jobject jcaller);

static void nativeDestroyContext(JNIEnv *env, jobject jcaller, jlong ctxContainer);

static void nativeBindFunc(JNIEnv *env, jobject jcaller, jlong ctxContainer, jstring func);

static void nativeUnBindFunc(JNIEnv *env, jobject jcaller, jlong ctxContainer, jstring func);

static void nativeExecJS(JNIEnv *env, jobject jcaller, jlong ctxContainer, jstring script);

static const JNINativeMethod kMethodsJSContext[] = {
    {"nativeCreateContext",
     "("
     ")"
     "J",
     reinterpret_cast<void *>(nativeCreateContext)},
    {"nativeDestroyContext",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void *>(nativeDestroyContext)},
    {"nativeBindFunc",
     "("
     "J"
     "Ljava/lang/String;"
     ")"
     "V",
     reinterpret_cast<void *>(nativeBindFunc)},
    {"nativeUnBindFunc",
     "("
     "J"
     "Ljava/lang/String;"
     ")"
     "V",
     reinterpret_cast<void *>(nativeUnBindFunc)},
    {"nativeExecJS",
     "("
     "J"
     "Ljava/lang/String;"
     ")"
     "V",
     reinterpret_cast<void *>(nativeExecJS)}};

static bool RegisterNativesImpl(JNIEnv *env) {
  g_JSContext_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kJSContextClassPath).Get()));
  const int kMethodsWXBridgeSize =
      sizeof(kMethodsJSContext) / sizeof(kMethodsJSContext[0]);
  if (env->RegisterNatives(g_JSContext_clazz, kMethodsJSContext,
                           kMethodsWXBridgeSize) < 0) {
    return false;
  }

  return true;
}

#endif  // WEEX_PROJECT_JSCONTEXT_JNI_H
