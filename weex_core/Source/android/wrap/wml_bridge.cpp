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

#include "android/wrap/wml_bridge.h"
#include "android/base/jni/android_jni.h"
#include "android/base/log_utils.h"
#include "android/base/string/string_utils.h"
#include "android/jsengine/multiprocess/WeexProxy.h"

static jint InitAppFramework(JNIEnv* env, jobject jcaller, jstring jinstanceid,
                             jstring jframwork, jobjectArray jargs) {
  WeexCore::WMLBridge::Instance()->Reset(env, jcaller);
  return WeexProxy::initAppFramework(env, jcaller, jinstanceid, jframwork,
                                     jargs);
}

static jint DestoryAppContext(JNIEnv* env, jobject jcaller,
                              jstring jinstanceid) {
  return WeexProxy::destoryAppContext(env, jcaller, jinstanceid);
}

static jint CreateAppContext(JNIEnv* env, jobject jcaller, jstring jinstanceid,
                             jstring jbundle, jobject jargs) {
  return WeexProxy::createAppContext(env, jcaller, jinstanceid, jbundle, jargs);
}

static jbyteArray ExecJsOnAppWithResult(JNIEnv* env, jobject jcaller,
                                        jstring jinstanceid, jstring jbundle,
                                        jobject jargs) {
  return WeexProxy::execJsOnAppWithResult(env, jcaller, jinstanceid, jbundle,
                                          jargs);
}

static jint ExecJsOnApp(JNIEnv* env, jobject jcaller, jstring jinstanceid,
                        jstring jfunction, jobjectArray jargs) {
  return WeexProxy::execJsOnApp(env, jcaller, jinstanceid, jfunction, jargs);
}

namespace WeexCore {

WMLBridge* WMLBridge::g_instance = nullptr;

const char kWMLBridgeClassPath[] = "com/taobao/windmill/bridge/WMLBridge";
jclass g_WMLBridge_clazz = nullptr;

static JNINativeMethod gWMMethods[] = {
    {"initAppFramework",
     "(Ljava/lang/String;Ljava/lang/String;[Lcom/taobao/weex/bridge/"
     "WXJSObject;)I",
     (void*)InitAppFramework},
    {"createAppContext",
     "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)I",
     (void*)CreateAppContext},
    {"execJsOnApp",
     "(Ljava/lang/String;Ljava/lang/String;"
     "[Lcom/taobao/weex/bridge/WXJSObject;)I",
     (void*)ExecJsOnApp},
    {"execJsOnAppWithResult",
     "(Ljava/lang/String;Ljava/lang/String;"
     "Ljava/util/Map;)[B",
     (void*)ExecJsOnAppWithResult},
    {"destoryAppContext", "(Ljava/lang/String;)I", (void*)DestoryAppContext},
};

static int registerWMLBridgeNativeMethods(JNIEnv* env, JNINativeMethod* methods,
                                          int numMethods) {
  if (g_WMLBridge_clazz == NULL) {
    LOGE("registerWMLBridgeNativeMethods failed to find bridge class.");
    return JNI_FALSE;
  }
  if ((env)->RegisterNatives(g_WMLBridge_clazz, methods, numMethods) < 0) {
    LOGE(
        "registerWMLBridgeNativeMethods failed to register native methods for "
        "bridge class.");
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static intptr_t g_WMLBridge_dispatchMessage = 0;
static void Java_WMLBridge_dispatchMessage(JNIEnv* env, jobject obj,
                                           jstring clientID, jstring vmID,
                                           jbyteArray data, jstring callback) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WMLBridge_clazz, base::android::INSTANCE_METHOD, "dispatchMessage",
      "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;)V",
      &g_WMLBridge_dispatchMessage);

  env->CallVoidMethod(obj, method_id, clientID, vmID, data, callback);
  base::android::CheckException(env);
}

static intptr_t g_WMLBridge_postMessage = 0;

static void Java_WMLBridge_postMessage(JNIEnv* env, jobject obj, jstring vmID,
                                       jbyteArray data) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WMLBridge_clazz, base::android::INSTANCE_METHOD, "postMessage",
      "(Ljava/lang/String;[B)V", &g_WMLBridge_postMessage);

  env->CallVoidMethod(obj, method_id, vmID, data);
  base::android::CheckException(env);
}

bool WMLBridge::RegisterJNIUtils(JNIEnv* env) {
  g_WMLBridge_clazz = env->FindClass(kWMLBridgeClassPath);

  // use to check WMLBridge has already on env
  if (env->ExceptionOccurred()) {
    LOGE("failed find class WMBridge");
    env->ExceptionDescribe();
    env->ExceptionClear();
    return false;
  } else if (g_WMLBridge_clazz != nullptr) {
    g_WMLBridge_clazz = (jclass)env->NewGlobalRef(g_WMLBridge_clazz);
    env->DeleteLocalRef(g_WMLBridge_clazz);
    LOGE("success find class WMBridge");
    registerWMLBridgeNativeMethods(env, gWMMethods,
                                   sizeof(gWMMethods) / sizeof(gWMMethods[0]));
    return true;
  }
}

void WMLBridge::PostMessage(JNIEnv* env, const char* vm_id, const char* data) {
  if (jni_object() != NULL) {
    auto jni_str = base::android::ScopedLocalJavaRef<jstring>(
        env, env->NewStringUTF(vm_id));
    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(
        env, newJByteArray(env, data));

    Java_WMLBridge_postMessage(env, jni_object(), jni_str.Get(),
                               jni_array.Get());
  }
}

void WMLBridge::DispatchMessage(JNIEnv* env, const char* client_id,
                                const char* vm_id, const char* data,
                                const char* callback) {
  if (jni_object() != NULL) {
    auto jni_client_id = base::android::ScopedLocalJavaRef<jstring>(
        env, newJString(env, client_id));
    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(
        env, newJByteArray(env, data));
    auto jni_callback = base::android::ScopedLocalJavaRef<jstring>(
        env, newJString(env, callback));
    auto jni_vm_id =
        base::android::ScopedLocalJavaRef<jstring>(env, newJString(env, vm_id));
    Java_WMLBridge_dispatchMessage(env, jni_object(), jni_client_id.Get(),
                                   jni_vm_id.Get(), jni_array.Get(),
                                   jni_callback.Get());
  }
}
}  // namespace WeexCore