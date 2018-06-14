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
#include "weexcore_impl_android.h"
#include <android/jsengine/multiprocess/WeexProxy.h>
#include <android/base/jni/android_jni.h>
#include <android/jniprebuild/jniheader/WXBridge_jni.h>
#include <android/base/string/string_utils.h>
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <core/render/node/render_object.h>
#include <core/config/core_environment.h>
#include <map>
#include <core/manager/weex_core_manager.h>

using namespace WeexCore;

jclass jBridgeClazz;
jclass jWXJSObject;
jclass jWXLogUtils;
jclass jMapClazz;
jclass jSetClazz;
jmethodID jMapConstructorMethodId = nullptr;
jmethodID jMapPutMethodId = nullptr;
jmethodID jSetConstructorMethodId = nullptr;
jmethodID jSetAddMethodId = nullptr;

jclass jWMBridgeClazz = nullptr;
jmethodID jDoubleValueMethodId;
jobject jThis;
jobject jWMThis;
jlongArray jFirstScreenRenderTime = nullptr;
jlongArray jRenderFinishTime = nullptr;
std::map<std::string, jobject> componentTypeCache;

JStringCache refCache(2048);

static JavaVM *sVm = NULL;

JNIEnv *getJNIEnv() {
  JNIEnv *env = NULL;
  if ((sVm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
    return JNI_FALSE;
  }
  return env;
}

jstring getComponentTypeFromCache(const std::string type) {
  std::map<std::string, jobject>::const_iterator iter = componentTypeCache.find(type);
  if (iter != componentTypeCache.end()) {
    return (jstring)(iter->second);
  } else {
    return nullptr;
  }
}

jstring putComponentTypeToCache(const std::string type) {
  JNIEnv *env = getJNIEnv();
  jstring jType = env->NewStringUTF(type.c_str());
  jobject jGlobalType = env->NewGlobalRef(jType);
  componentTypeCache.insert(std::pair<std::string, jobject>(type, jGlobalType));
  env->DeleteLocalRef(jType);
  return (jstring) jGlobalType;
}

jstring getKeyFromCache(JNIEnv *env, const char *key) {
  return refCache.GetString(env, key);
}

jfloatArray c2jFloatArray(JNIEnv *env, const float c_array[]) {
  if (nullptr == c_array) {
    return nullptr;
  }
  if (0 == c_array[0]
      && 0 == c_array[1]
      && 0 == c_array[2]
      && 0 == c_array[3]) {
    // Default value;
    return nullptr;
  }
  jfloatArray jArray = env->NewFloatArray(4);
  env->SetFloatArrayRegion(jArray, 0, 4, c_array);
  return jArray;
}

static jint InitFrameworkEnv(JNIEnv *env, jobject jcaller,
                             jstring framework,
                             jobject params,
                             jstring cacheDir,
                             jboolean pieSupport) {
  jThis = env->NewGlobalRef(jcaller);
  jclass tempClass = env->FindClass(
          "com/taobao/weex/bridge/WXBridge");
  jBridgeClazz = (jclass) env->NewGlobalRef(tempClass);
  env->DeleteLocalRef(tempClass);
  return WeexProxy::doInitFramework(env, jThis, framework, params, cacheDir, pieSupport);
}

static void BindMeasurementToRenderObject(JNIEnv* env, jobject jcaller,
                                          jlong ptr){
  RenderObject *render =  convert_long_to_render_object(ptr);
  if(render){
    render->BindMeasureFunc();
  }
}

static void OnInstanceClose(JNIEnv *env, jobject jcaller, jstring instanceId) {
    WeexCoreManager::getInstance()->getPlatformBridge()->onInstanceClose(
            env->GetStringUTFChars(instanceId, JNI_FALSE)
    );
}

static void SetDefaultHeightAndWidthIntoRootDom(JNIEnv *env, jobject jcaller,
                                                jstring instanceId, jfloat defaultWidth, jfloat defaultHeight,
                                                jboolean isWidthWrapContent, jboolean isHeightWrapContent) {
  WeexCoreManager::getInstance()->getPlatformBridge()->setDefaultHeightAndWidthIntoRootDom(
          env->GetStringUTFChars(instanceId, JNI_FALSE),
          defaultWidth,
          defaultHeight,
          isWidthWrapContent,
          isHeightWrapContent
  );
}

static void SetRenderContainerWrapContent(JNIEnv* env, jobject jcaller, jboolean wrap, jstring instanceId) {
    const char *cInstanceId = env->GetStringUTFChars(instanceId, JNI_FALSE);
    WeexCoreManager::getInstance()->getPlatformBridge()->setRenderContainerWrapContent(
            cInstanceId,wrap
    );
}

static jlongArray GetFirstScreenRenderTime(JNIEnv *env, jobject jcaller, jstring instanceId) {
  jlongArray jTemp = env->NewLongArray(3);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr) {
    if (jFirstScreenRenderTime != nullptr) {
      env->DeleteGlobalRef(jFirstScreenRenderTime);
      jFirstScreenRenderTime = nullptr;
    }
    jFirstScreenRenderTime = static_cast<jlongArray>(env->NewGlobalRef(jTemp));
    return jFirstScreenRenderTime;
  }

  std::vector<int64_t> temp = page->PrintFirstScreenLog();

  jlong ret[3];

  ret[0] = temp[0];
  ret[1] = temp[1];
  ret[2] = temp[2];
  env->SetLongArrayRegion(jTemp, 0, 3, ret);

  if (jFirstScreenRenderTime != nullptr) {
    env->DeleteGlobalRef(jFirstScreenRenderTime);
    jFirstScreenRenderTime = nullptr;
  }
  jFirstScreenRenderTime = static_cast<jlongArray>(env->NewGlobalRef(jTemp));

  env->DeleteLocalRef(jTemp);
  return jFirstScreenRenderTime;
}

static jlongArray GetRenderFinishTime(JNIEnv *env, jobject jcaller, jstring instanceId) {
  jlongArray jTemp = env->NewLongArray(3);

  RenderPage *page = RenderManager::GetInstance()->GetPage(jString2StrFast(env, instanceId));
  if (page == nullptr) {
    if (jRenderFinishTime != nullptr) {
      env->DeleteGlobalRef(jRenderFinishTime);
      jRenderFinishTime = nullptr;
    }
    jRenderFinishTime = static_cast<jlongArray>(env->NewGlobalRef(jTemp));
    return jRenderFinishTime;
  }

  std::vector<int64_t> temp = page->PrintRenderSuccessLog();

  jlong ret[3];

  ret[0] = temp[0];
  ret[1] = temp[1];
  ret[2] = temp[2];
  env->SetLongArrayRegion(jTemp, 0, 3, ret);

  if (jRenderFinishTime != nullptr) {
    env->DeleteGlobalRef(jRenderFinishTime);
    jRenderFinishTime = nullptr;
  }
  jRenderFinishTime = static_cast<jlongArray>(env->NewGlobalRef(jTemp));

  env->DeleteLocalRef(jTemp);
  return jRenderFinishTime;
}

//Notice that this method is invoked from main thread.
static jboolean NotifyLayout(JNIEnv* env, jobject jcaller, jstring instanceId) {
    bool ret = WeexCoreManager::getInstance()->getPlatformBridge()->notifyLayout(
            env->GetStringUTFChars(instanceId, JNI_FALSE)
    );
    return ret ? JNI_TRUE : JNI_FALSE;
}

//Notice that this method is invoked from JS thread.
static void ForceLayout(JNIEnv *env, jobject jcaller, jstring instanceId) {
    WeexCoreManager::getInstance()->getPlatformBridge()->forceLayout(
            env->GetStringUTFChars(instanceId, JNI_FALSE)
    );
}

static void SetStyleWidth(JNIEnv *env, jobject jcaller,
                          jstring instanceId, jstring ref, jfloat value) {


    WeexCoreManager::getInstance()->getPlatformBridge()->setStyleWidth(
            env->GetStringUTFChars(instanceId, JNI_FALSE),
            env->GetStringUTFChars(ref, JNI_FALSE),
            value
    );
}

static void SetStyleHeight(JNIEnv *env, jobject jcaller,
                           jstring instanceId, jstring ref, jfloat value) {
    WeexCoreManager::getInstance()->getPlatformBridge()->setStyleHeight(
            env->GetStringUTFChars(instanceId, JNI_FALSE),
            env->GetStringUTFChars(ref, JNI_FALSE),
            value
    );
}

static void SetMargin(JNIEnv *env, jobject jcaller,
                      jstring instanceId, jstring ref, jint edge, jfloat value) {
    WeexCoreManager::getInstance()->getPlatformBridge()->setMargin(
            env->GetStringUTFChars(instanceId, JNI_FALSE),
            env->GetStringUTFChars(ref, JNI_FALSE),
            edge,
            value
    );
}

static void SetPadding(JNIEnv *env, jobject jcaller,
                       jstring instanceId, jstring ref, jint edge, jfloat value) {
    WeexCoreManager::getInstance()->getPlatformBridge()->setPadding(
            env->GetStringUTFChars(instanceId, JNI_FALSE),
            env->GetStringUTFChars(ref, JNI_FALSE),
            edge,
            value
    );
}


static void SetPosition(JNIEnv *env, jobject jcaller,
                        jstring instanceId, jstring ref, jint edge, jfloat value) {
  WeexCoreManager::getInstance()->getPlatformBridge()->setPosition(
          env->GetStringUTFChars(instanceId, JNI_FALSE),
          env->GetStringUTFChars(ref, JNI_FALSE),
          edge,
          value
  );
}

static void MarkDirty(JNIEnv *env, jobject jcaller,
                      jstring instanceId, jstring ref, jboolean dirty) {
    WeexCoreManager::getInstance()->getPlatformBridge()->markDirty(
            env->GetStringUTFChars(instanceId, JNI_FALSE),
            env->GetStringUTFChars(ref, JNI_FALSE),
            dirty
    );
}

static void RegisterCoreEnv(JNIEnv *env, jobject jcaller, jstring key, jstring value) {
  LOGE("RegisterCoreEnv，key: %s, value: %s", jString2StrFast(env, key).c_str(),
       jString2StrFast(env, value).c_str());
  WXCoreEnvironment::getInstance()->AddOption(jString2StrFast(env, key), jString2StrFast(env, value));
}

static jint InitFramework(JNIEnv *env, jobject object, jstring script, jobject params) {
  jThis = env->NewGlobalRef(object);
  jclass tempClass = env->FindClass(
          "com/taobao/weex/bridge/WXBridge");
  jBridgeClazz = (jclass) env->NewGlobalRef(tempClass);
  env->DeleteLocalRef(tempClass);
  return WeexProxy::doInitFramework(env, jThis, script, params);
}

static jint ExecJSService(JNIEnv *env, jobject object, jstring script) {
  if (script == nullptr)
    return false;
  return WeexProxy::execJSService(env, object, script);
}

static void TakeHeapSnapshot(JNIEnv *env, jobject object, jstring name) {
}

/**
 * Called to execute JavaScript such as . createInstance(),destroyInstance ext.
 *
 */
static jint ExecJS(JNIEnv *env, jobject jthis, jstring jinstanceid, jstring jnamespace, jstring jfunction, jobjectArray jargs) {
  if (jfunction == NULL || jinstanceid == NULL) {
    LOGE("native_execJS function is NULL");
    return false;
  }

  return WeexProxy::execJS(env, jThis, jinstanceid, jnamespace, jfunction, jargs);
}

static jbyteArray ExecJSWithResult(JNIEnv* env, jobject jcaller, jstring instanceId, jstring _namespace, jstring _function, jobjectArray args) {
  if (_function == NULL || instanceId == NULL) {
    LOGE("native_execJS function is NULL");
    return NULL;
  }

  return WeexProxy::execJSWithResult(env, jcaller, instanceId, _namespace, _function, args);
}

static void UpdateGlobalConfig(JNIEnv* env, jobject jcaller, jstring config) {
  WeexProxy::updateGlobalConfig(env, jcaller, config);
}


static jint CreateInstanceContext(JNIEnv* env, jobject jcaller, jstring instanceId, jstring name, jstring function, jobjectArray args) {
  return WeexProxy::createInstanceContext(env, jcaller, instanceId, name, function, args);
}

static jint DestoryInstance(JNIEnv* env, jobject jcaller, jstring instanceId, jstring name, jstring function, jobjectArray args) {
  return WeexProxy::destoryInstance(env, jcaller, instanceId, name, function, args);
}

static jstring ExecJSOnInstance(JNIEnv* env, jobject jcaller, jstring instanceId, jstring script, jint type) {
  return WeexProxy::execJSOnInstance(env, jcaller, instanceId, script, type);
}

static jint native_initAppFramework(JNIEnv* env,
                                    jobject jcaller,
                                    jstring jinstanceid,
                                    jstring jframwork,
                                    jobjectArray jargs) {
  jWMThis = env->NewGlobalRef(jcaller);
  Bridge_Impl_Android::getInstance()->setGlobalWMRef(jWMThis);
  return WeexProxy::initAppFramework(env, jcaller, jinstanceid, jframwork, jargs);
}

static jint native_destoryAppContext(JNIEnv* env,
                                     jobject jcaller,
                                     jstring jinstanceid) {
  return WeexProxy::destoryAppContext(env, jcaller, jinstanceid);
}

static jint native_createAppContext(JNIEnv* env,
                                    jobject jcaller,
                                    jstring jinstanceid,
                                    jstring jbundle,
                                    jobject jargs) {
  return WeexProxy::createAppContext(env, jcaller, jinstanceid, jbundle, jargs);
}

static jbyteArray native_execJsOnAppWithResult(JNIEnv* env,
                                               jobject jcaller,
                                               jstring jinstanceid,
                                               jstring jbundle,
                                               jobject jargs) {
  return WeexProxy::execJsOnAppWithResult(env, jcaller, jinstanceid, jbundle, jargs);
}

static jint native_execJsOnApp(JNIEnv* env,
                               jobject jcaller,
                               jstring jinstanceid,
                               jstring jfunction,
                               jobjectArray jargs) {
  return WeexProxy::execJsOnApp(env, jcaller, jinstanceid, jfunction, jargs);
}


namespace WeexCore {
  bool RegisterJNIUtils(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
  static JNINativeMethod gWMMethods[] = {
        {"initAppFramework",
                "(Ljava/lang/String;Ljava/lang/String;[Lcom/taobao/weex/bridge/WXJSObject;)I",
                (void *) native_initAppFramework},
        {"createAppContext",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)I",
                (void *) native_createAppContext},
        { "execJsOnApp",
                "(Ljava/lang/String;Ljava/lang/String;"
                        "[Lcom/taobao/weex/bridge/WXJSObject;)I",
                (void*)native_execJsOnApp },
        { "execJsOnAppWithResult",
                "(Ljava/lang/String;Ljava/lang/String;"
                        "Ljava/util/Map;)[B",
                (void*)native_execJsOnAppWithResult },
        { "destoryAppContext",
                "(Ljava/lang/String;)I",
                (void*)native_destoryAppContext },
  };

  static int registerWMLBridgeNativeMethods(JNIEnv* env, JNINativeMethod* methods, int numMethods) {
    if (jWMBridgeClazz == NULL) {
      LOGE("registerWMLBridgeNativeMethods failed to find bridge class.");
      return JNI_FALSE;
    }
    if ((env)->RegisterNatives(jWMBridgeClazz, methods, numMethods) < 0) {
      LOGE("registerWMLBridgeNativeMethods failed to register native methods for bridge class.");
      return JNI_FALSE;
    }

    return JNI_TRUE;
}



jint OnLoad(JavaVM *vm, void *reserved) {
    LOGD("begin JNI_OnLoad");
    JNIEnv *env;
    /* Get environment */
    if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
      return JNI_FALSE;
    }
    try {
      throw 1;
    } catch (int) {
    }
    sVm = vm;
    jclass tempClass = env->FindClass(
        "com/taobao/weex/bridge/WXBridge");
    jBridgeClazz = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("com/taobao/weex/bridge/WXJSObject");
    jWXJSObject = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("com/taobao/weex/utils/WXLogUtils");
    jWXLogUtils = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("com/taobao/weex/utils/WXMap");
    jMapClazz = (jclass) env->NewGlobalRef(tempClass);

    tempClass = env->FindClass("java/util/HashSet");
    jSetClazz = (jclass) env->NewGlobalRef(tempClass);

    jMapConstructorMethodId = env->GetMethodID(jMapClazz, "<init>", "()V");
    jMapPutMethodId = env->GetMethodID(jMapClazz, "put", "(Ljava/lang/String;[B)Ljava/lang/String;");
    jSetConstructorMethodId = env->GetMethodID(jSetClazz, "<init>", "()V");
    jSetAddMethodId = env->GetMethodID(jSetClazz, "add", "(Ljava/lang/Object;)Z");

    // can use this code to manal register jni
    tempClass = nullptr;
    tempClass = env->FindClass("com/taobao/windmill/bridge/WMLBridge");
    // use to check WMLBridge has already on env
    if (env->ExceptionOccurred()) {
      LOGE("failed find class WMBridge");
      env->ExceptionDescribe();
      env->ExceptionClear();
      jWMBridgeClazz = nullptr;
    } else if (tempClass != nullptr) {
      jWMBridgeClazz = (jclass)env->NewGlobalRef(tempClass);
      LOGE("success find class WMBridge");
      registerWMLBridgeNativeMethods(env, gWMMethods, sizeof(gWMMethods) / sizeof(gWMMethods[0]));
    }
    env->DeleteLocalRef(tempClass);

    LOGD("end JNI_OnLoad");
    WeexProxy::setCacheDir(env);

    return JNI_VERSION_1_4;
  }

  void Unload(JavaVM *vm, void *reserved) {
    LOGD("beigin JNI_OnUnload");
    JNIEnv *env;

    /* Get environment */
    if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
      return;
    }
    env->DeleteGlobalRef(jBridgeClazz);
    env->DeleteGlobalRef(jWXJSObject);
    env->DeleteGlobalRef(jWXLogUtils);
    env->DeleteGlobalRef(jMapClazz);

    jMapConstructorMethodId = nullptr;
    jMapPutMethodId = nullptr;
    jSetConstructorMethodId = nullptr;
    jSetAddMethodId = nullptr;

    if (jFirstScreenRenderTime != nullptr) {
      env->DeleteLocalRef(jFirstScreenRenderTime);
      jFirstScreenRenderTime = nullptr;
    }

    if (jRenderFinishTime != nullptr) {
      env->DeleteLocalRef(jRenderFinishTime);
      jRenderFinishTime = nullptr;
    }

    for (auto iter = componentTypeCache.begin(); iter != componentTypeCache.end(); iter++) {
      if (iter->second != nullptr) {
        env->DeleteGlobalRef(iter->second);
        iter->second = nullptr;
      }
    }
    componentTypeCache.clear();

    refCache.clearRefCache(env);

    if (jThis)
      env->DeleteGlobalRef(jThis);
    if (jWMThis)
      env->DeleteLocalRef(jWMThis);
    WeexProxy::reset();
    LOGD(" end JNI_OnUnload");
  }
}