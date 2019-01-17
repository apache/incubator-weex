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

#include "android/base/string/string_utils.h"
#include "android/utils/params_utils.h"
#include "base/android/jni/android_jni.h"
#include "base/android/jni_type.h"
#include "base/android/log_utils.h"
#include "core/manager/weex_core_manager.h"

static intptr_t jDoubleValueMethodId = 0;
static jint InitAppFramework(JNIEnv* env, jobject jcaller, jstring jinstanceid,
                             jstring jframwork, jobjectArray jargs) {
  WeexCore::WMLBridge::Instance()->Reset(env, jcaller);
  ScopedJStringUTF8 id(env, jinstanceid);
  ScopedJStringUTF8 framework(env, jframwork);
  int length = jargs == NULL ? 0 : env->GetArrayLength(jargs);
  std::vector<INIT_FRAMEWORK_PARAMS*> params;

  for (int i = 0; i < length; i++) {
    auto jArg = std::unique_ptr<WXJSObject>(
        new WXJSObject(env, env->GetObjectArrayElement(jargs, i)));
    jint jTypeInt = jArg->GetType(env);
    auto jDataObj = jArg->GetData(env);
    auto jKeyStr = jArg->GetKey(env);

    if (jTypeInt == 1) {
      jclass jDoubleClazz = env->FindClass("java/lang/Double");
      jmethodID method_id = nullptr;
      if (jDoubleValueMethodId != 0) {
        method_id = reinterpret_cast<jmethodID>(jDoubleValueMethodId);
      } else {
        method_id = base::android::GetMethod(
                env, jDoubleClazz,
                base::android::INSTANCE_METHOD,
                "doubleValue",
                "()D",
                &jDoubleValueMethodId);
      }

      jdouble jDoubleObj = env->CallDoubleMethod(jDataObj.Get(), method_id);
      const auto size = sizeof(jDoubleObj) + 1;
      char * charData = (char *) malloc(size);
      if(charData == nullptr){
        continue;
      }

      memset(charData, 0, size);
      snprintf(charData, size, "%f", jDoubleObj);

      ScopedJStringUTF8 jni_key(env, jKeyStr.Get());
      params.push_back(genInitFrameworkParams(jni_key.getChars(), charData));
      free(charData);
    } else if (jTypeInt == 2) {
      jstring jDataStr = (jstring)jDataObj.Get();
      ScopedJStringUTF8 jni_key(env, jKeyStr.Get());
      ScopedJStringUTF8 jni_data(env, jDataStr);
      params.push_back(
          genInitFrameworkParams(jni_key.getChars(), jni_data.getChars()));
    } else if (jTypeInt == 3) {
      jstring jDataStr = (jstring)jDataObj.Get();
      ScopedJStringUTF8 jni_key(env, jKeyStr.Get());
      ScopedJStringUTF8 jni_data(env, jDataStr);
      params.push_back(
          genInitFrameworkParams(jni_key.getChars(), jni_data.getChars()));
    }
  }

  auto result =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->InitAppFramework(id.getChars(), framework.getChars(), params);
  freeParams(params);
  return result;
}

static jint DestoryAppContext(JNIEnv* env, jobject jcaller,
                              jstring jinstanceid) {
  ScopedJStringUTF8 id(env, jinstanceid);
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->DestroyAppContext(id.getChars());
}

static jint CreateAppContext(JNIEnv* env, jobject jcaller, jstring jinstanceid,
                             jstring jbundle, jobject jargs) {
  ScopedJStringUTF8 id(env, jinstanceid);
  ScopedJStringUTF8 bundle(env, jbundle);
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->CreateAppContext(id.getChars(), bundle.getChars());
}

static jbyteArray ExecJsOnAppWithResult(JNIEnv* env, jobject jcaller,
                                        jstring jinstanceid, jstring jbundle,
                                        jobject jargs) {
  ScopedJStringUTF8 id(env, jinstanceid);
  ScopedJStringUTF8 bundle(env, jbundle);
  auto ret = WeexCoreManager::Instance()
                 ->getPlatformBridge()
                 ->core_side()
                 ->ExecJSOnAppWithResult(id.getChars(), bundle.getChars());
  if (ret.get() != nullptr && ret->data.get() != nullptr) {
    jbyteArray array = env->NewByteArray(ret->length);
    env->SetByteArrayRegion(array, 0, ret->length,
                            reinterpret_cast<const jbyte*>(ret->data.get()));
    return array;
  }

  return nullptr;
}

static jint ExecJsOnApp(JNIEnv* env, jobject jcaller, jstring jinstanceid,
                        jstring jfunction, jobjectArray jargs) {
  //  return WeexProxy::execJsOnApp(env, jcaller, jinstanceid, jfunction,
  //  jargs);

  ScopedJStringUTF8 id(env, jinstanceid);
  ScopedJStringUTF8 function(env, jfunction);

  int length = 0;
  if (jargs != NULL) {
    length = env->GetArrayLength(jargs);
  }
  std::vector<VALUE_WITH_TYPE*> params;

  for (int i = 0; i < length; i++) {
    auto jArg = std::unique_ptr<WXJSObject>(
        new WXJSObject(env, env->GetObjectArrayElement(jargs, i)));
    jint jTypeInt = jArg->GetType(env);
    auto jDataObj = jArg->GetData(env);

    VALUE_WITH_TYPE* param = nullptr;

    param = getValueWithTypePtr();

    if (jTypeInt == 1) {
      jdouble jDoubleObj =
          base::android::JNIType::DoubleValue(env, jDataObj.Get());
      param->type = ParamsType::DOUBLE;
      param->value.doubleValue = jDoubleObj;

    } else if (jTypeInt == 2) {
      jstring jDataStr = (jstring)jDataObj.Get();
      param->type = ParamsType::STRING;
      param->value.string = jstring2WeexString(env, jDataStr);
    } else if (jTypeInt == 3) {
      jstring jDataStr = (jstring)jDataObj.Get();
      param->type = ParamsType::JSONSTRING;
      param->value.string = jstring2WeexString(env, jDataStr);
    } else {
      param->type = ParamsType::JSUNDEFINED;
    }
    params.push_back(param);
  }

  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->CallJSOnAppContext(id.getChars(), function.getChars(), params);
}

namespace WeexCore {

WMLBridge* WMLBridge::g_instance = nullptr;

const char kWMLBridgeClassPath[] =  "com/taobao/windmill/bundle/bridge/WeexBridge";
jclass g_WMLBridge_clazz = nullptr;

static JNINativeMethod gWMMethods[] = {
    {"nativeInitAppFramework",
     "(Ljava/lang/String;Ljava/lang/String;[Lcom/taobao/weex/bridge/"
     "WXJSObject;)I",
     (void*)InitAppFramework},
    {"nativeCreateAppContext",
     "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)I",
     (void*)CreateAppContext},
    {"nativeExecJsOnApp",
     "(Ljava/lang/String;Ljava/lang/String;"
     "[Lcom/taobao/weex/bridge/WXJSObject;)I",
     (void*)ExecJsOnApp},
    {"nativeExecJsOnAppWithResult",
     "(Ljava/lang/String;Ljava/lang/String;"
     "Ljava/util/Map;)[B",
     (void*)ExecJsOnAppWithResult},
    {"nativeDestoryAppContext", "(Ljava/lang/String;)I", (void*)DestoryAppContext},
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

static intptr_t g_WMLBridge_dispatchMessageSync = 0;
static base::android::ScopedLocalJavaRef<jbyteArray>
Java_WMLBridge_dispatchMessageSync(JNIEnv* env, jobject obj, jstring clientID,
                                   jstring vmID, jbyteArray data) {
  jmethodID method_id = base::android::GetMethod(
      env, g_WMLBridge_clazz, base::android::INSTANCE_METHOD,
      "dispatchMessageSync", "(Ljava/lang/String;Ljava/lang/String;[B)[B",
      &g_WMLBridge_dispatchMessageSync);

  auto result = env->CallObjectMethod(obj, method_id, clientID, vmID, data);
  base::android::CheckException(env);
  return base::android::ScopedLocalJavaRef<jbyteArray>(
      env, static_cast<jbyteArray>(result));
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
  jclass temp_class = env->FindClass(kWMLBridgeClassPath);

  // use to check WMLBridge has already on env
  if (env->ExceptionOccurred()) {
    LOGE("failed find class WMBridge");
    env->ExceptionDescribe();
    env->ExceptionClear();
    return false;
  } else if (temp_class != nullptr) {
    g_WMLBridge_clazz = (jclass)env->NewGlobalRef(temp_class);
    env->DeleteLocalRef(temp_class);
    LOGE("success find class WMBridge");
    registerWMLBridgeNativeMethods(env, gWMMethods,
                                   sizeof(gWMMethods) / sizeof(gWMMethods[0]));
    return true;
  }
  return false;
}

void WMLBridge::PostMessage(JNIEnv* env, const char* vm_id, const char* data, int dataLength) {
  if (jni_object() != NULL) {
    auto jni_str = base::android::ScopedLocalJavaRef<jstring>(
        env, env->NewStringUTF(vm_id));
    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(
        env, newJByteArray(env, data, dataLength));

    Java_WMLBridge_postMessage(env, jni_object(), jni_str.Get(),
                               jni_array.Get());
  }
}

void WMLBridge::DispatchMessage(JNIEnv *env, const char *client_id,
                                    const char *data, int dataLength, const char *callback, const char* vm_id) {
  if (jni_object() != NULL) {
    auto jni_client_id = base::android::ScopedLocalJavaRef<jstring>(
        env, newJString(env, client_id));
    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(
        env, newJByteArray(env, data, dataLength));
    auto jni_callback = base::android::ScopedLocalJavaRef<jstring>(
        env, newJString(env, callback));
    auto jni_vm_id = base::android::ScopedLocalJavaRef<jstring>(env, newJString(env, vm_id));
    Java_WMLBridge_dispatchMessage(env, jni_object(), jni_client_id.Get(),
                                   jni_vm_id.Get(), jni_array.Get(),
                                   jni_callback.Get());
  }
}

base::android::ScopedLocalJavaRef<jbyteArray> WMLBridge::DispatchMessageSync(
    JNIEnv* env, const char* client_id, const char* data, int dataLength,
    const char* vm_id) {
  if (jni_object() != NULL) {
    auto jni_client_id = base::android::ScopedLocalJavaRef<jstring>(
        env, newJString(env, client_id));
    auto jni_array = base::android::ScopedLocalJavaRef<jbyteArray>(
        env, newJByteArray(env, data, dataLength));
    auto jni_vm_id =
        base::android::ScopedLocalJavaRef<jstring>(env, newJString(env, vm_id));
    return Java_WMLBridge_dispatchMessageSync(env, jni_object(),
                                              jni_client_id.Get(),
                                              jni_vm_id.Get(), jni_array.Get());
  }
  return base::android::ScopedLocalJavaRef<jbyteArray>();
}

}  // namespace WeexCore
