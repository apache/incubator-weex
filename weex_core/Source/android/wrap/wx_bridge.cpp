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

#include "android/wrap/wx_bridge.h"
#include <fstream>
#include "core/render/manager/render_manager.h"
#include "base/time_calculator.h"
#include "base/log_defines.h"

#include "android/wrap/log_utils.h"
#include "android/base/string/string_utils.h"
#include "android/bridge/platform/android_bridge.h"
#include "android/bridge/platform/android_bridge_in_multi_process.h"
#include "android/bridge/platform/android_bridge_in_multi_so.h"
#include "android/bridge/script_bridge_in_multi_process.h"
#include "android/bridge/script_bridge_in_multi_so.h"
#include "android/utils/cache_utils.h"
#include "android/utils/params_utils.h"
#include "android/utils/so_utils.h"
#include "android/wrap/hash_set.h"
#include "android/wrap/wx_js_object.h"
#include "android/wrap/wx_map.h"
#include "base/android/jni_type.h"
#include "base/android/jni/jbytearray_ref.h"
#include "base/android/jniprebuild/jniheader/WXBridge_jni.h"
#include "core/config/core_environment.h"
#include "core/layout/layout.h"
#include "core/layout/measure_func_adapter_impl_android.h"
#include "core/manager/weex_core_manager.h"
#include "core/bridge/eagle_bridge.h"
#include "core/common/view_utils.h"
#include "third_party/json11/json11.hpp"
#include "core/moniter/render_performance.h"
#include "core/render/page/render_page_base.h"
#include "third_party/IPC/IPCFutexPageQueue.h"

using namespace WeexCore;
jlongArray jFirstScreenRenderTime = nullptr;
jlongArray jRenderFinishTime = nullptr;
static jint InitFrameworkEnv(JNIEnv* env, jobject jcaller, jstring framework,
                             jobject params, jstring cacheDir,
                             jboolean pieSupport) {
  const char* cache = env->GetStringUTFChars(cacheDir, nullptr);
  if (strlen(cache) > 0) {
    SoUtils::set_cache_dir(const_cast<char*>(cache));
  }
  SoUtils::set_pie_support(pieSupport);
  jint ret = InitFramework(env, jcaller, framework, params);
  env->ReleaseStringUTFChars(cacheDir, cache);
  return ret;
}

static void BindMeasurementToRenderObject(JNIEnv* env, jobject jcaller,
                                          jlong ptr) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->BindMeasurementToRenderObject(ptr);
}

static void OnInstanceClose(JNIEnv* env, jobject jcaller, jstring instanceId) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->OnInstanceClose(jString2StrFast(env, instanceId));
}

static void SetDefaultHeightAndWidthIntoRootDom(JNIEnv* env, jobject jcaller,
                                                jstring instanceId,
                                                jfloat defaultWidth,
                                                jfloat defaultHeight,
                                                jboolean isWidthWrapContent,
                                                jboolean isHeightWrapContent) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->SetDefaultHeightAndWidthIntoRootDom(
          jString2StrFast(env, instanceId), defaultWidth, defaultHeight,
          isWidthWrapContent, isHeightWrapContent);
}

static void SetRenderContainerWrapContent(JNIEnv* env, jobject jcaller,
                                          jboolean wrap, jstring instanceId) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->SetRenderContainerWrapContent(jString2StrFast(env, instanceId), wrap);
}

// Call on main thread
static jlongArray GetFirstScreenRenderTime(JNIEnv* env, jobject jcaller,
                                           jstring instanceId) {
  jlongArray jTemp = env->NewLongArray(3);

  std::vector<int64_t> temp =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->GetFirstScreenRenderTime(jString2StrFast(env, instanceId));

  if (temp.empty()) {
    if (jFirstScreenRenderTime != nullptr) {
      env->DeleteGlobalRef(jFirstScreenRenderTime);
      jFirstScreenRenderTime = nullptr;
    }
    jFirstScreenRenderTime = static_cast<jlongArray>(env->NewGlobalRef(jTemp));
    return jFirstScreenRenderTime;
  }

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

// Call on main thread
static jlongArray GetRenderFinishTime(JNIEnv* env, jobject jcaller,
                                      jstring instanceId) {
  jlongArray jTemp = env->NewLongArray(3);

  std::vector<int64_t> temp =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->GetRenderFinishTime(jString2StrFast(env, instanceId));
  if (temp.empty()) {
    if (jRenderFinishTime != nullptr) {
      env->DeleteGlobalRef(jRenderFinishTime);
      jRenderFinishTime = nullptr;
    }
    jRenderFinishTime = static_cast<jlongArray>(env->NewGlobalRef(jTemp));
    return jRenderFinishTime;
  }

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

// Notice that this method is invoked from main thread.
static jboolean NotifyLayout(JNIEnv* env, jobject jcaller, jstring instanceId) {
  if (WeexCoreManager::Instance()->getPlatformBridge()) {
    ScopedJStringUTF8 nativeString = ScopedJStringUTF8(env, instanceId);
    const char* c_str_instance_id = nativeString.getChars();
    std::string std_string_nstanceId = std::string(c_str_instance_id == nullptr ? "" : c_str_instance_id);
    return WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->core_side()
        ->NotifyLayout(std_string_nstanceId);
  }
  return false;
}

// Notice that this method is invoked from JS thread.
static void ForceLayout(JNIEnv* env, jobject jcaller, jstring instanceId) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->ForceLayout(
      jString2StrFast(env, instanceId));
}

// call on main thread
static void SetStyleWidth(JNIEnv* env, jobject jcaller, jstring instanceId,
                          jstring ref, jfloat value) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->SetStyleWidth(
      jString2StrFast(env, instanceId), jString2StrFast(env, ref), value);
}

// call on main thread
static void SetStyleHeight(JNIEnv* env, jobject jcaller, jstring instanceId,
                           jstring ref, jfloat value) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->SetStyleHeight(
      jString2StrFast(env, instanceId), jString2StrFast(env, ref), value);
}

static void SetMargin(JNIEnv* env, jobject jcaller, jstring instanceId,
                      jstring ref, jint edge, jfloat value) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->SetMargin(
      jString2StrFast(env, instanceId), jString2StrFast(env, ref), edge, value);
}

static void SetPadding(JNIEnv* env, jobject jcaller, jstring instanceId,
                       jstring ref, jint edge, jfloat value) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->SetPadding(
      jString2StrFast(env, instanceId), jString2StrFast(env, ref), edge, value);
}

static void SetPosition(JNIEnv* env, jobject jcaller, jstring instanceId,
                        jstring ref, jint edge, jfloat value) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->SetPosition(
      jString2StrFast(env, instanceId), jString2StrFast(env, ref), edge, value);
}

static void MarkDirty(JNIEnv* env, jobject jcaller, jstring instanceId,
                      jstring ref, jboolean dirty) {
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->MarkDirty(
      jString2StrFast(env, instanceId), jString2StrFast(env, ref));
}

static void RegisterCoreEnv(JNIEnv* env, jobject jcaller, jstring key,
                            jstring value) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->RegisterCoreEnv(jString2StrFast(env, key), jString2StrFast(env, value));
}

static void ResetWXBridge(JNIEnv* env, jobject jcaller,
                         jobject bridge,
                         jstring className) {
  ScopedJStringUTF8 classNameRef = ScopedJStringUTF8(env, className);
  WXBridge::Instance()->Reset(env, bridge);
  WXBridge::Instance()->reset_clazz(env, classNameRef.getChars());
}

static void SetViewPortWidth(JNIEnv* env, jobject jcaller, jstring instanceId,
                             jfloat value) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->SetViewPortWidth(jString2StrFast(env, instanceId), value);
}

static void SetInstanceRenderType(JNIEnv* env, jobject jcaller,
                                  jstring instanceId,
                                  jstring renderType){
  WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->SetPageRenderType(jString2StrFast(env, instanceId), jString2StrFast(env, renderType));
}

static void RemoveInstanceRenderType(JNIEnv* env, jobject jcaller,
                                     jstring instanceId){
  WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->RemovePageRenderType(jString2StrFast(env, instanceId));
}

static void SetLogType(JNIEnv* env, jobject jcaller, jfloat logLevel,
                       jfloat isPerf){
  int32_t l = (int32_t)logLevel;
  weex::base::LogImplement::getLog()->setPrintLevel((WeexCore::LogLevel)l);
  bool flag = isPerf == 1;
  weex::base::LogImplement::getLog()->setPerfMode(flag);
  LOGE("WeexCore setLog Level %d in Performance mode %s debug %d", l, flag ? "true" : "false", (int)WeexCore::LogLevel::Debug);
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->SetLogType(l, flag);
}

static jstring nativeDumpIpcPageQueueInfo(JNIEnv* env, jobject jcaller){
    std::string client_quene_msg;
    if (WeexCoreManager::Instance()->client_queue_ != nullptr){
        WeexCoreManager::Instance()->client_queue_->dumpPageInfo(client_quene_msg);
    }
    std::string server_quene_msg;
    if (WeexCoreManager::Instance()->server_queue_ != nullptr){
        WeexCoreManager::Instance()->server_queue_->dumpPageInfo(server_quene_msg);
    }
    std::string result ;
    result = "{client:"+client_quene_msg+"}\n"+"{server:"+server_quene_msg+"}";
    return env->NewStringUTF(result.c_str());
}
static void ReloadPageLayout(JNIEnv *env, jobject jcaller,
                              jstring instanceId){
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->RelayoutUsingRawCssStyles(jString2StrFast(env,instanceId));
}

static void SetPageArgument(JNIEnv* env, jobject jcaller,
                            jstring instanceId,
                            jstring key,
                            jstring value){
    WeexCoreManager::Instance()
            ->getPlatformBridge()
            ->core_side()->SetPageArgument(jString2StrFast(env, instanceId),
                                           jString2StrFast(env, key), jString2StrFast(env, value));
}
static void SetDeviceDisplayOfPage(JNIEnv *env, jobject jcaller,
                                   jstring instanceId,jfloat width,jfloat height){
  WeexCoreManager::Instance()->getPlatformBridge()->core_side()->SetDeviceDisplayOfPage(jString2StrFast(env,instanceId),width,height);
}

static void SetDeviceDisplay(JNIEnv* env, jobject jcaller, jstring instanceId,
                           jfloat value, float height, float scale) {
  WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->SetDeviceDisplay(jString2StrFast(env, instanceId), value, height, scale);
}

static jint InitFramework(JNIEnv* env, jobject object, jstring script,
                          jobject params) {
  if (!WXBridge::Instance()->jni_object()) {
    WXBridge::Instance()->Reset(env, object);
  }
  // Init platform thread --- ScriptThread
  WeexCoreManager::Instance()->InitScriptThread();
  // Exception handler for so
  SoUtils::RegisterExceptionHanler(
      [](const char* status_code, const char* error_msg) {
        WeexCoreManager::Instance()
            ->getPlatformBridge()
            ->platform_side()
            ->ReportNativeInitStatus(status_code, error_msg);
      });
  PlatformBridge* bridge = WeexCoreManager::Instance()->getPlatformBridge();
  // Init params
  std::vector<INIT_FRAMEWORK_PARAMS*> params_vector = initFromParam(
      env, params, [](const char* status_code, const char* error_msg) {
        WeexCoreManager::Instance()
            ->getPlatformBridge()
            ->platform_side()
            ->ReportNativeInitStatus(status_code, error_msg);
      });
  // If parse init params error, return false
  if (params_vector.empty()) return false;
  // Set project mode
  WeexCoreManager::Instance()->set_project_mode(
          WeexCoreManager::ProjectMode::MULTI_PROCESS);

  WeexCoreManager::Instance()->set_script_bridge(
          new ScriptBridgeInMultiProcess);

  // It means initialization failed when any bridge is not passable
  if (!WeexCoreManager::Instance()->getPlatformBridge()->is_passable() ||
      !WeexCoreManager::Instance()->script_bridge()->is_passable()) {

    if(isSingleProcess()) {
      WeexCoreManager::Instance()->set_project_mode(
              WeexCoreManager::ProjectMode::MULTI_SO);
      WeexCoreManager::Instance()->set_script_bridge(new ScriptBridgeInMultiSo);

      if (!WeexCoreManager::Instance()->getPlatformBridge()->is_passable() ||
          !WeexCoreManager::Instance()->script_bridge()->is_passable()) {
        return false;
      }
    }
  }


//  if (isSingleProcess()) {
//    WeexCoreManager::Instance()->set_project_mode(
//        WeexCoreManager::ProjectMode::MULTI_SO);
//  } else {
//    WeexCoreManager::Instance()->set_project_mode(
//        WeexCoreManager::ProjectMode::MULTI_PROCESS);
//  }
//  // Init script bridge
//  if (WeexCoreManager::Instance()->project_mode() ==
//      WeexCoreManager::ProjectMode::MULTI_PROCESS) {
//    WeexCoreManager::Instance()->set_script_bridge(
//        new ScriptBridgeInMultiProcess);
//  } else {
//    WeexCoreManager::Instance()->set_script_bridge(new ScriptBridgeInMultiSo);
//  }
//  // It means initialization failed when any bridge is not passable
//  if (!WeexCoreManager::Instance()->getPlatformBridge()->is_passable() ||
//      !WeexCoreManager::Instance()->script_bridge()->is_passable()) {
//    return false;
//  }

  // for environment
  bridge->core_side()->SetPlatform(
      WXCoreEnvironment::getInstance()->platform());
  bridge->core_side()->SetDeviceWidthAndHeight(
      WXCoreEnvironment::getInstance()->DeviceWidth(),
      WXCoreEnvironment::getInstance()->DeviceHeight());
  auto options = WXCoreEnvironment::getInstance()->options();
  auto it = options.begin();
  for (; it != options.end(); it++) {
    bridge->core_side()->AddOption(it->first, it->second);
  }
  // Set measure function
  WeexCoreManager::Instance()->set_measure_function_adapter(
      new MeasureFunctionAdapterImplAndroid());
  bridge->core_side()->SetMeasureFunctionAdapter();
  ScopedJStringUTF8 c_script(env, script);
  // Call InitFramework
  auto result =
      bridge->core_side()->InitFramework(c_script.getChars(), params_vector);
  freeParams(params_vector);
  return result;
}

static void RefreshInstance(JNIEnv* env, jobject jcaller, jstring instanceId,
                            jstring _namespace, jstring _function,
                            jobjectArray args) {
  ScopedJStringUTF8 instance_id(env, instanceId);
  ScopedJStringUTF8 name_space(env, _namespace);
  ScopedJStringUTF8 function(env, _function);
  int length = 0;
  if (args != nullptr) {
    length = env->GetArrayLength(args);
  }
  std::vector<VALUE_WITH_TYPE*> params;

  for (int i = 0; i < length; i++) {
    VALUE_WITH_TYPE* param = nullptr;

    param = WeexCore::getValueWithTypePtr();
    auto jni_object = base::android::ScopedLocalJavaRef<jobject>(
        env, env->GetObjectArrayElement(args, i));
    auto wx_js_object =
        std::unique_ptr<WXJSObject>(new WXJSObject(env, jni_object.Get()));
    addParamsFromJArgs(params, param, env, wx_js_object);
  }
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->RefreshInstance(instance_id.getChars(), name_space.getChars(),
                        function.getChars(), params);

  freeParams(params);
}

static jint ExecJSService(JNIEnv* env, jobject object, jstring script) {
  if (script == nullptr) return false;
  ScopedJStringUTF8 source(env, script);
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->ExecJsService(source.getChars());
}

static void TakeHeapSnapshot(JNIEnv* env, jobject object, jstring name) {}

/**
 * Called to execute JavaScript such as . createInstance(),destroyInstance ext.
 *
 */
static jint ExecJS(JNIEnv* env, jobject jthis, jstring jinstanceid,
                   jstring jnamespace, jstring jfunction, jobjectArray jargs) {
  if (jfunction == NULL || jinstanceid == NULL) {
    LOGE("native_execJS function is NULL");
    return false;
  }

  ScopedJStringUTF8 instance_id(env, jinstanceid);
  ScopedJStringUTF8 name_space(env, jnamespace);
  ScopedJStringUTF8 function(env, jfunction);
  int length = 0;
  if (jargs != nullptr) {
    length = env->GetArrayLength(jargs);
  }
  std::vector<VALUE_WITH_TYPE*> params;

  for (int i = 0; i < length; i++) {
    VALUE_WITH_TYPE* param = nullptr;

    param = WeexCore::getValueWithTypePtr();
    auto jni_object = base::android::ScopedLocalJavaRef<jobject>(
        env, env->GetObjectArrayElement(jargs, i));
    auto wx_js_object =
        std::unique_ptr<WXJSObject>(new WXJSObject(env, jni_object.Get()));
    addParamsFromJArgs(params, param, env, wx_js_object);
  }
  auto result =
      WeexCoreManager::Instance()->getPlatformBridge()->core_side()->ExecJS(
          instance_id.getChars(), name_space.getChars(), function.getChars(),
          params);

  freeParams(params);
  return result;
}

static void ExecJSWithCallback(JNIEnv* env, jobject jcaller,
                             jstring instanceId,
                             jstring _namespace,
                             jstring _function,
                             jobjectArray args,
                             jlong callbackId) {
  if (_function == NULL || instanceId == NULL) {
    LOGE("native_execJS function is NULL");
    return;
  }
  ScopedJStringUTF8 instance_id(env, instanceId);
  ScopedJStringUTF8 name_space(env, _namespace);
  ScopedJStringUTF8 function(env, _function);
  int length = 0;
  if (args != NULL) {
    length = env->GetArrayLength(args);
  }
  std::vector<VALUE_WITH_TYPE*> params;

  for (int i = 0; i < length; i++) {
    VALUE_WITH_TYPE* param = nullptr;

    param = getValueWithTypePtr();

    auto wx_js_object = std::unique_ptr<WXJSObject>(
            new WXJSObject(env, base::android::ScopedLocalJavaRef<jobject>(
                    env, env->GetObjectArrayElement(args, i))
                    .Get()));
    addParamsFromJArgs(params, param, env, wx_js_object);
  }

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->ExecJSWithCallback(instance_id.getChars(), name_space.getChars(),
                           function.getChars(), params, callbackId);

  freeParams(params);
}


static void UpdateInitFrameworkParams(JNIEnv* env, jobject jcaller,
                                      jstring key_,
                                      jstring value_,
                                      jstring desc_){

  if(key_ == nullptr || value_ == nullptr || desc_ == nullptr){
    return;
  }

  WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->core_side()
        ->UpdateInitFrameworkParams(jString2StrFast(env, key_),
                                    jString2StrFast(env, value_),
                                    jString2StrFast(env, desc_));
  if(jString2StrFast(env, key_) == "androidStatusBarHeight"){
    WXCoreEnvironment::getInstance()->PutOption(WeexCore::STATUS_BAR_HEIGHT, jString2StrFast(env, value_));
  }
}

static void UpdateGlobalConfig(JNIEnv* env, jobject jcaller, jstring config) {
  if (config == NULL) {
    LOGE("native_execJS function is NULL");
    return;
  }
  ScopedJStringUTF8 scoped_config(env, config);
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->UpdateGlobalConfig(scoped_config.getChars());
}




static jint CreateInstanceContext(JNIEnv* env, jobject jcaller,
                                  jstring instanceId, jstring name,
                                  jstring function, jobjectArray args) {
  if (function == NULL || instanceId == NULL) {
    LOGE("native_createInstanceContext function is NULL");
    return false;
  }
  int length = 0;
  if (args != NULL) {
    length = env->GetArrayLength(args);
  }
  if (length < 4) {
    LOGE("native_createInstanceContext jargs format error");
    return false;
  }
  auto arg1 = std::unique_ptr<WXJSObject>(
      new WXJSObject(env, base::android::ScopedLocalJavaRef<jobject>(
          env, env->GetObjectArrayElement(args, 1))
          .Get()));
  auto jscript = arg1->GetData(env);
  auto opts = base::android::ScopedLocalJavaRef<jstring>(
      env, getJsonData(env, args, 2));
  // init jsonData
  auto initData = base::android::ScopedLocalJavaRef<jstring>(
      env, getJsonData(env, args, 3));
  auto arg4 = std::unique_ptr<WXJSObject>(
      new WXJSObject(env, base::android::ScopedLocalJavaRef<jobject>(
          env, env->GetObjectArrayElement(args, 4))
          .Get()));
  // get render strategy
  auto render_strategy = std::unique_ptr<WXJSObject>(
      new WXJSObject(env, base::android::ScopedLocalJavaRef<jobject>(
          env, env->GetObjectArrayElement(args, 5))
          .Get()));
  auto japi = arg4->GetData(env);
  auto extraOptionString = base::android::ScopedLocalJavaRef<jstring>(
          env, getJsonData(env, args, 6));


  ScopedJStringUTF8 scoped_id(env, instanceId);
  ScopedJStringUTF8 scoped_func(env, function);
  ScopedJStringUTF8 scoped_opts(env, opts.Get());
  ScopedJStringUTF8 scoped_init_data(env, initData.Get());
  ScopedJStringUTF8 scoped_api(env, static_cast<jstring>(japi.Get()));
  ScopedJStringUTF8 scoped_render_strategy(
      env, static_cast<jstring>(render_strategy->GetData(env).Release()));

  ScopedJStringUTF8 scoped_extra_option(env, extraOptionString.Get());
  const std::string input = scoped_extra_option.getChars();
  std::vector<INIT_FRAMEWORK_PARAMS*> params;
  if(input.length() > 0) {
    std::string err;

    const json11::Json &json = json11::Json::parse(input, err);
    const std::map<std::string, json11::Json> &data = json.object_items();
    auto it = data.begin();

    while (it != data.end()) {
      INIT_FRAMEWORK_PARAMS *param = nullptr;
      const std::string &string = it->second.string_value();
      param = WeexCore::genInitFrameworkParams(it->first.c_str(),it->second.string_value().c_str());
      params.push_back(param);
      it++;
    }
  }

  // If strategy is DATA_RENDER_BINARY, jscript is a jbyteArray, otherwise jstring
  // TODO use better way
  if (!WXBridge::Instance()->jni_object()) {
    WXBridge::Instance()->Reset(env, jcaller);
  }
  if (scoped_render_strategy.getChars() != nullptr
      && strcmp(scoped_render_strategy.getChars(), "DATA_RENDER_BINARY") == 0) {
    JByteArrayRef byte_array(env, static_cast<jbyteArray>(jscript.Get()));
    return WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->core_side()
        ->CreateInstance(scoped_id.getChars(), scoped_func.getChars(),
                         byte_array.getBytes(), byte_array.length(), scoped_opts.getChars(),
                         scoped_init_data.getChars(), scoped_api.getChars(), params,
                         scoped_render_strategy.getChars());
  } else {
    ScopedJStringUTF8 scoped_script(env, static_cast<jstring>(jscript.Get()));
    return WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->core_side()
        ->CreateInstance(scoped_id.getChars(), scoped_func.getChars(),
                         scoped_script.getChars(), strlen(scoped_script.getChars()),
                         scoped_opts.getChars(),
                         scoped_init_data.getChars(), scoped_api.getChars(), params,
                         scoped_render_strategy.getChars());
  }
}

static jint DestoryInstance(JNIEnv* env, jobject jcaller, jstring instanceId,
                            jstring name, jstring function, jobjectArray args) {
  int ret = ExecJS(env, nullptr, instanceId, name, function, args);
  if (function == NULL || instanceId == NULL) {
    LOGE("native_destoryInstance function is NULL");
    return false;
  }

  ScopedJStringUTF8 idChar(env, instanceId);
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()
      ->DestroyInstance(idChar.getChars());
}

static jstring ExecJSOnInstance(JNIEnv* env, jobject jcaller,
                                jstring instanceId, jstring script, jint type) {
  if (instanceId == NULL || script == NULL) {
    return env->NewStringUTF("");
  }

  ScopedJStringUTF8 idChar(env, instanceId);
  ScopedJStringUTF8 scriptChar(env, script);
  auto result =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->core_side()
          ->ExecJSOnInstance(idChar.getChars(), scriptChar.getChars(),type);

  if (result.get() == nullptr || result->data.get() == nullptr)
    return nullptr;

  return env->NewStringUTF(result->data.get());
}

static void onInteractionTimeUpdate(JNIEnv* env, jobject jcaller, jstring instanceId) {
  ScopedJStringUTF8 idChar(env, instanceId);

  RenderPageBase* page = RenderManager::GetInstance()->GetPage(idChar.getChars());
  if (nullptr == page){
    return;
  }
  auto performance = page->getPerformance();
  if (nullptr == performance){
    return;
  }
  bool change = performance->onInteractionTimeUpdate();

  if (!change){
      return;
  }

  std::map<std::string,std::string> c_performance_data;
  performance->getPerformanceStringData(c_performance_data);


  auto performance_map = std::unique_ptr<WXMap>(new WXMap);
  if (performance_map == nullptr) {
      return;
  }
  performance_map->Put(env,c_performance_data);

  jobject jni_map_performance =
          performance_map.get() != nullptr ? performance_map->jni_object() : nullptr;
  Java_WXBridge_onNativePerformanceDataUpdate(env,jcaller,instanceId,jni_map_performance);
}

static void FireEventOnDataRenderNode(JNIEnv* env, jobject jcaller,
                                      jstring instanceId, jstring ref,
                                      jstring type, jstring data,
                                      jstring domChanges) {
  if (instanceId == NULL || ref == NULL || type == NULL || data == NULL) {
    return;
  }

  ScopedJStringUTF8 idChar(env, instanceId);
  ScopedJStringUTF8 refChar(env, ref);
  ScopedJStringUTF8 typeChar(env, type);
  ScopedJStringUTF8 dataChar(env, data);
  ScopedJStringUTF8 domChangesChar(env, domChanges);

  WeexCore::EagleBridge::GetInstance()->data_render_handler()->FireEvent(
      idChar.getChars(), refChar.getChars(), typeChar.getChars(),
      dataChar.getChars(), domChangesChar.getChars()
  );
}

static void InvokeCallbackOnDataRender(JNIEnv* env, jobject jcaller,
                                       jstring instanceId, jstring callbackId,
                                       jstring data, jboolean keepAlive) {
  if (instanceId == NULL || callbackId == NULL || data == NULL) {
    return;
  }

  ScopedJStringUTF8 idChar(env, instanceId);
  ScopedJStringUTF8 callbackChar(env, callbackId);
  ScopedJStringUTF8 dataChar(env, data);

  WeexCore::EagleBridge::GetInstance()->data_render_handler()->InvokeCallback(
      idChar.getChars(), callbackChar.getChars(), dataChar.getChars(),keepAlive);
}

static void RegisterModuleOnDataRenderNode(JNIEnv* env, jobject jcaller,
                                      jstring data) {
  if (data == NULL) {
    return;
  }

  ScopedJStringUTF8 dataChar(env, data);

  auto data_render_handler = WeexCore::EagleBridge::GetInstance()->data_render_handler();
  if(data_render_handler){
    data_render_handler->RegisterModules(
        dataChar.getChars());
  }
}

static void RegisterComponentOnDataRenderNode(JNIEnv* env, jobject jcaller,
                                              jstring data) {
  if (data == NULL) {
    return;
  }

  ScopedJStringUTF8 dataChar(env, data);
  auto data_render_handler = WeexCore::EagleBridge::GetInstance()->data_render_handler();
  if(data_render_handler) {
    data_render_handler->RegisterComponent(dataChar.getChars());
  }
}

namespace WeexCore {

WXBridge* WXBridge::g_instance = nullptr;

bool WXBridge::RegisterJNIUtils(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

base::android::ScopedLocalJavaRef<jobject> WXBridge::GetMeasureFunc(
    JNIEnv* env, const char* page_id, jlong render_object_ptr) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  return Java_WXBridge_getMeasurementFunc(env, jni_object(), jni_page_id.Get(),
                                          render_object_ptr);
}

int WXBridge::HasTransitionPros(
    JNIEnv* env, const char* page_id, const char* ref,
    const std::vector<std::pair<std::string, std::string>>& styles) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  auto styles_map = std::unique_ptr<WXMap>(new WXMap);
  if (!styles.empty()) {
    styles_map->Put(env, styles);
  }

  return Java_WXBridge_callHasTransitionPros(env, jni_object(), jni_page_id.Get(),
                                             jni_ref.Get(), styles_map->jni_object());
}

int WXBridge::AppendTreeCreateFinish(JNIEnv* env, const char* page_id,
                                     const char* ref) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));
  return Java_WXBridge_callAppendTreeCreateFinish(env, jni_object(),
                                                  jni_page_id.Get(), jni_ref.Get());
}

int WXBridge::MoveElement(JNIEnv* env, const char* page_id, const char* ref,
                          const char* parent_ref, int index) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));
  auto jni_parent_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(parent_ref));

  return Java_WXBridge_callMoveElement(env, jni_object(), jni_page_id.Get(), jni_ref.Get(),
                                       jni_parent_ref.Get(), index);
}

int WXBridge::RemoveElement(JNIEnv* env, const char* page_id, const char* ref) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  return Java_WXBridge_callRemoveElement(env, jni_object(), jni_page_id.Get(),
                                         jni_ref.Get());
}

int WXBridge::CreateFinish(JNIEnv* env, const char* page_id) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  return Java_WXBridge_callCreateFinish(env, jni_object(), jni_page_id.Get());
}

int WXBridge::UpdateAttr(
    JNIEnv* env, const char* page_id, const char* ref,
    std::vector<std::pair<std::string, std::string>>* attrs) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  auto attributes_map = std::unique_ptr<WXMap>();
  if (attrs != nullptr) {
    attributes_map.reset(new WXMap);
    attributes_map->Put(env, *attrs);
  }

  jobject jni_attributes =
      attributes_map.get() != nullptr ? attributes_map->jni_object() : nullptr;
  return Java_WXBridge_callUpdateAttrs(env, jni_object(), jni_page_id.Get(), jni_ref.Get(),
                                       jni_attributes);
}

int WXBridge::UpdateStyle(
    JNIEnv* env, const char* page_id, const char* ref,
    std::vector<std::pair<std::string, std::string>>* style,
    std::vector<std::pair<std::string, std::string>>* margin,
    std::vector<std::pair<std::string, std::string>>* padding,
    std::vector<std::pair<std::string, std::string>>* border) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  auto styles_map = std::unique_ptr<WXMap>();
  if (style != nullptr) {
    styles_map.reset(new WXMap);
    styles_map->Put(env, *style);
  }
  auto margins_map = std::unique_ptr<WXMap>();
  if (margin != nullptr) {
    margins_map.reset(new WXMap);
    margins_map->Put(env, *margin);
  }
  auto paddings_map = std::unique_ptr<WXMap>();
  if (padding != nullptr) {
    paddings_map.reset(new WXMap);
    paddings_map->Put(env, *padding);
  }
  auto borders_map = std::unique_ptr<WXMap>();
  if (border != nullptr) {
    borders_map.reset(new WXMap);
    borders_map->Put(env, *border);
  }

  jobject jni_styles =
      styles_map.get() != nullptr ? styles_map->jni_object() : nullptr;
  jobject jni_margins =
      margins_map.get() != nullptr ? margins_map->jni_object() : nullptr;
  jobject jni_paddings =
      paddings_map.get() != nullptr ? paddings_map->jni_object() : nullptr;
  jobject jni_borders =
      borders_map.get() != nullptr ? borders_map->jni_object() : nullptr;

  return Java_WXBridge_callUpdateStyle(env, jni_object(), jni_page_id.Get(), jni_ref.Get(),
                                       jni_styles, jni_margins, jni_paddings,
                                       jni_borders);
}

int WXBridge::Layout(JNIEnv* env, const char* page_id, const char* ref, int top,
                     int bottom, int left, int right, int height, int width, bool isRTL,
                     int index) {
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jRef = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  return Java_WXBridge_callLayout(env, jni_object(), jPageId.Get(), jRef.Get(), top, bottom,
                                  left, right, height, width, isRTL, index);
}

int WXBridge::AddElement(JNIEnv* env, const char* page_id,
                         const char* component_type, const char* ref,
                         int& index, const char* parentRef,
                         std::map<std::string, std::string>* styles,
                         std::map<std::string, std::string>* attributes,
                         std::set<std::string>* events,
                         const WXCoreMargin& margins,
                         const WXCorePadding& paddings,
                         const WXCoreBorderWidth& borders, bool willLayout) {
  auto jni_page_id = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));
  auto jni_parent_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(parentRef));

  auto styles_map = std::unique_ptr<WXMap>(new WXMap);
  styles_map->Put(env, *styles);
  auto attributes_map = std::unique_ptr<WXMap>(new WXMap);
  attributes_map->Put(env, *attributes);
  auto events_set = std::unique_ptr<HashSet>(new HashSet);
  events_set->Add(env, *events);

  float c_margins[4];
  float c_paddings[4];
  float c_borders[4];

  c_margins[0] = margins.getMargin(kMarginTop);
  c_margins[1] = margins.getMargin(kMarginBottom);
  c_margins[2] = margins.getMargin(kMarginLeft);
  c_margins[3] = margins.getMargin(kMarginRight);

  c_paddings[0] = paddings.getPadding(kPaddingTop);
  c_paddings[1] = paddings.getPadding(kPaddingBottom);
  c_paddings[2] = paddings.getPadding(kPaddingLeft);
  c_paddings[3] = paddings.getPadding(kPaddingRight);

  c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
  c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
  c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
  c_borders[3] = borders.getBorderWidth(kBorderWidthRight);

  auto jni_margins =
      0 == c_margins[0] && 0 == c_margins[1] && 0 == c_margins[2] &&
      0 == c_margins[3]
      ? base::android::ScopedLocalJavaRef<jfloatArray>()
      : base::android::JNIType::NewFloatArray(env, 4, c_margins);
  auto jni_paddings =
      0 == c_paddings[0] && 0 == c_paddings[1] && 0 == c_paddings[2] &&
      0 == c_paddings[3]
      ? base::android::ScopedLocalJavaRef<jfloatArray>()
      : base::android::JNIType::NewFloatArray(env, 4, c_paddings);
  auto jni_borders =
      0 == c_borders[0] && 0 == c_borders[1] && 0 == c_borders[2] &&
      0 == c_borders[3]
      ? base::android::ScopedLocalJavaRef<jfloatArray>()
      : base::android::JNIType::NewFloatArray(env, 4, c_borders);

  jstring jni_component_type = getComponentTypeFromCache(component_type);
  if (jni_component_type == nullptr) {
    jni_component_type = putComponentTypeToCache(component_type);
  }

  return Java_WXBridge_callAddElement(
      env, jni_object(), jni_page_id.Get(), jni_component_type, jni_ref.Get(), index,
      jni_parent_ref.Get(), styles_map->jni_object(), attributes_map->jni_object(),
      events_set->jni_object(), jni_margins.Get(), jni_paddings.Get(),
      jni_borders.Get(), willLayout);
}

int WXBridge::CreateBody(JNIEnv* env, const char* page_id,
                         const char* component_type, const char* ref,
                         std::map<std::string, std::string>* styles,
                         std::map<std::string, std::string>* attributes,
                         std::set<std::string>* events,
                         const WXCoreMargin& margins,
                         const WXCorePadding& paddings,
                         const WXCoreBorderWidth& borders) {
  auto jni_pageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jni_ref = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  auto styles_map = std::unique_ptr<WXMap>(new WXMap);
  styles_map->Put(env, *styles);
  auto attributes_map = std::unique_ptr<WXMap>(new WXMap);
  attributes_map->Put(env, *attributes);
  auto events_set = std::unique_ptr<HashSet>(new HashSet);
  events_set->Add(env, *events);

  float c_margins[4];
  float c_paddings[4];
  float c_borders[4];

  c_margins[0] = margins.getMargin(kMarginTop);
  c_margins[1] = margins.getMargin(kMarginBottom);
  c_margins[2] = margins.getMargin(kMarginLeft);
  c_margins[3] = margins.getMargin(kMarginRight);

  c_paddings[0] = paddings.getPadding(kPaddingTop);
  c_paddings[1] = paddings.getPadding(kPaddingBottom);
  c_paddings[2] = paddings.getPadding(kPaddingLeft);
  c_paddings[3] = paddings.getPadding(kPaddingRight);

  c_borders[0] = borders.getBorderWidth(kBorderWidthTop);
  c_borders[1] = borders.getBorderWidth(kBorderWidthBottom);
  c_borders[2] = borders.getBorderWidth(kBorderWidthLeft);
  c_borders[3] = borders.getBorderWidth(kBorderWidthRight);

  auto jni_margins =
      0 == c_margins[0] && 0 == c_margins[1] && 0 == c_margins[2] &&
      0 == c_margins[3]
      ? base::android::ScopedLocalJavaRef<jfloatArray>()
      : base::android::JNIType::NewFloatArray(env, 4, c_margins);
  auto jni_paddings =
      0 == c_paddings[0] && 0 == c_paddings[1] && 0 == c_paddings[2] &&
      0 == c_paddings[3]
      ? base::android::ScopedLocalJavaRef<jfloatArray>()
      : base::android::JNIType::NewFloatArray(env, 4, c_paddings);
  auto jni_borders =
      0 == c_borders[0] && 0 == c_borders[1] && 0 == c_borders[2] &&
      0 == c_borders[3]
      ? base::android::ScopedLocalJavaRef<jfloatArray>()
      : base::android::JNIType::NewFloatArray(env, 4, c_borders);

  jstring jni_component_type = getComponentTypeFromCache(component_type);
  if (jni_component_type == nullptr) {
    jni_component_type = putComponentTypeToCache(component_type);
  }

  int flag = Java_WXBridge_callCreateBody(
      env, jni_object(), jni_pageId.Get(), jni_component_type, jni_ref.Get(),
      styles_map->jni_object(), attributes_map->jni_object(),
      events_set->jni_object(), jni_margins.Get(), jni_paddings.Get(),
      jni_borders.Get());
  return flag;
}

int WXBridge::RemoveEvent(JNIEnv* env, const char* page_id, const char* ref,
                          const char* event) {
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jRef = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  auto jEventId =
      base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(event));

  return Java_WXBridge_callRemoveEvent(env, jni_object(), jPageId.Get(), jRef.Get(),
                                       jEventId.Get());
}

int WXBridge::AddEvent(JNIEnv* env, const char* page_id, const char* ref,
                       const char* event) {
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(ref));

  auto jEventId =
      base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(event));
  return Java_WXBridge_callAddEvent(env, jni_object(), jPageId.Get(), jRef.Get(),
                                    jEventId.Get());
}

int WXBridge::AddChildToRichtext(JNIEnv* env, const char *pageId, const char *nodeType,
        const char *ref,const char *parentRef,const char *richtextRef,
        std::map<std::string, std::string> *styles,std::map<std::string, std::string> *attributes) {
    auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(pageId));
    auto jParentPef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(parentRef));
    auto jRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(ref));
    auto jRichtextRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(richtextRef));
    auto jNodeType =
                base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(nodeType));
    auto styles_map = std::unique_ptr<WXMap>(new WXMap);
    styles_map->Put(env, *styles);
    auto attributes_map = std::unique_ptr<WXMap>(new WXMap);
    attributes_map->Put(env, *attributes);
    return Java_WXBridge_callAddChildToRichtext(env, jni_object(), jPageId.Get(), jNodeType.Get(), jRef.Get(),
            jParentPef.Get(),jRichtextRef.Get(),styles_map->jni_object(), attributes_map->jni_object());
}

int WXBridge::RemoveChildFromRichtext(JNIEnv* env, const char *pageId, const char *ref, const char *parent_ref,
                                          const char *richtext_ref) {
    auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(pageId));
    auto jParentPef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(parent_ref));
    auto jRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(ref));
    auto jRichtextRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(richtext_ref));
    return Java_WXBridge_callRemoveChildFromRichtext(env, jni_object(), jPageId.Get(),jRef.Get(),
                                                    jParentPef.Get(),jRichtextRef.Get());
}
int WXBridge::UpdateRichtextStyle(JNIEnv* env, const char *pageId, const char *ref,
                                      std::vector<std::pair<std::string, std::string>> *styles, const char *parent_ref,
                                      const char *richtext_ref) {
    auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(pageId));
    auto jParentPef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(parent_ref));
    auto jRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(ref));
    auto jRichtextRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(richtext_ref));
    auto styles_map = std::unique_ptr<WXMap>(new WXMap);
    styles_map->Put(env, *styles);
    return Java_WXBridge_callUpdateRichtextStyle(env, jni_object(), jPageId.Get(), jRef.Get(),
                                                    styles_map->jni_object(), jParentPef.Get(), jRichtextRef.Get());
}
int WXBridge::UpdateRichtextChildAttr(JNIEnv* env, const char *pageId, const char *ref,
                                      std::vector<std::pair<std::string, std::string>> *attributes,
                                      const char *parent_ref, const char *richtext_ref) {
    auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(pageId));
    auto jParentPef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(parent_ref));
    auto jRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(ref));
    auto jRichtextRef = base::android::ScopedLocalJavaRef<jstring >(env, env->NewStringUTF(richtext_ref));
    auto attributes_map = std::unique_ptr<WXMap>(new WXMap);
    attributes_map->Put(env, *attributes);
    return Java_WXBridge_callUpdateRichtextChildAttr(env, jni_object(), jPageId.Get(), jRef.Get(),
                                                 attributes_map->jni_object(), jParentPef.Get(), jRichtextRef.Get());
}

int WXBridge::RefreshFinish(JNIEnv* env, const char* page_id, const char* task,
                            const char* callback) {
  auto jTask = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, task));
  auto jCallback = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(callback));
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  return Java_WXBridge_callRefreshFinish(env, jni_object(), jPageId.Get(),
                                         jTask.Get(), jCallback.Get());
}

int WXBridge::RenderSuccess(JNIEnv* env, const char* page_id) {
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  return Java_WXBridge_callRenderSuccess(env, jni_object(), jPageId.Get());
}

int WXBridge::UpdateFinish(JNIEnv* env, const char* page_id, const char* task,
                           const char* callback) {
  auto jTask = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, task));
  auto jCallback = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(callback));
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));

  return Java_WXBridge_callUpdateFinish(env, jni_object(), jPageId.Get(), jTask.Get(),
                                        jCallback.Get());
}

void WXBridge::SetTimeout(JNIEnv* env, const char* callback_id,
                          const char* time) {
  auto jCallbackID = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(callback_id));
  auto jTime =
      base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(time));

  Java_WXBridge_setTimeoutNative(env, jni_object(), jCallbackID.Get(),
                                 jTime.Get());
}

void WXBridge::CallNativeComponent(JNIEnv* env, const char* page_id,
                                   const char* ref, const char* method,
                                   const char* arguments, int arguments_length,
                                   const char* options, int options_length) {
  auto jMethod = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(method));
  auto jArgString = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, arguments, arguments_length));
  auto jOptString = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, options, options_length));
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));
  auto jRef = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(ref));

  if (!jMethod.IsNull()) {
    Java_WXBridge_callNativeComponent(env, jni_object(), jPageId.Get(), jRef.Get(),
                                      jMethod.Get(), jArgString.Get(),
                                      jOptString.Get());
  }
}

base::android::ScopedLocalJavaRef<jobject> WXBridge::CallNativeModule(
    JNIEnv* env, const char* page_id, const char* module, const char* method,
    const char* arguments, int arguments_length, const char* options,
    int options_length) {
  auto jModule = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(module));
  auto jMethod = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(method));
  auto jArgString = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, arguments, arguments_length));
  auto jOptString = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, options, options_length));
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));

  if (!jModule.IsNull() && !jMethod.IsNull()) {
    return Java_WXBridge_callNativeModule(env, jni_object(), jPageId.Get(),
                                          jModule.Get(), jMethod.Get(),
                                          jArgString.Get(), jOptString.Get());
  }
  return base::android::ScopedLocalJavaRef<jobject>();
}

int WXBridge::CallNative(JNIEnv* env, const char* page_id, const char* task,
                         const char* callback) {
  auto jTask = base::android::ScopedLocalJavaRef<jbyteArray>(
      env, newJByteArray(env, task));
  auto jCallback = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(callback));
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));

  if (!jTask.IsNull()) {
    return Java_WXBridge_callNative(env, jni_object(), jPageId.Get(), jTask.Get(),
                                    jCallback.Get());
  }
  return -1;
}

void WXBridge::ReportException(JNIEnv* env, const char* page_id,
                               const char* func, const char* exception_string) {
  auto jFunc =
      base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(func));
  auto jExceptionString = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(exception_string));
  auto jPageId = base::android::ScopedLocalJavaRef<jstring>(env, env->NewStringUTF(page_id));

  Java_WXBridge_reportJSException(env, jni_object(), jPageId.Get(), jFunc.Get(),
                                  jExceptionString.Get());
}

void WXBridge::SetJSFrmVersion(JNIEnv* env, const char* version) {
  auto jVersion = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(version));
  Java_WXBridge_setJSFrmVersion(env, jni_object(), jVersion.Get());
}

void WXBridge::ReportServerCrash(JNIEnv* env, const char* instance_id,
                                 const char* crash_file) {
  auto jni_instance_id = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(instance_id));
  auto jni_crash_file = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(crash_file));
  Java_WXBridge_reportServerCrash(env, jni_object(), jni_instance_id.Get(),
                                  jni_crash_file.Get());
}

void WXBridge::ReportNativeInitStatus(JNIEnv* env, const char* statusCode,
                                      const char* errorMsg) {
  auto jni_status_code = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(statusCode));
  auto jni_error_msg = base::android::ScopedLocalJavaRef<jstring>(
      env, env->NewStringUTF(errorMsg));
  Java_WXBridge_reportNativeInitStatus(env, jni_object(), jni_status_code.Get(),
                                       jni_error_msg.Get());
}

void WXBridge::OnReceivedResult(JNIEnv *env, long callback_id,
                                std::unique_ptr<WeexJSResult>& result) {

  auto array = base::android::ScopedLocalJavaRef<jbyteArray>();
  if (result != nullptr && result->length != 0) {
    array.Reset(env, env->NewByteArray(result->length));
    env->SetByteArrayRegion(array.Get(), 0, result->length,
                            reinterpret_cast<const jbyte*>(result->data.get()));
  }
  Java_WXBridge_onReceivedResult(env, jni_object(), callback_id, array.Get());

}

void WXBridge::reset_clazz(JNIEnv* env, const char* className) {
  LOGE("class Name is %s", className);
  Java_WXBridge_reset_clazz(env, className);
}

}  // namespace WeexCore
