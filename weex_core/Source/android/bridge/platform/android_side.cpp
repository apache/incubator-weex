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

#include "android/bridge/platform/android_side.h"

#include "android/base/string/string_utils.h"
#include "android/utils/ipc_string_result.h"
#include "android/utils/cache_utils.h"
#include "android/utils/params_utils.h"
#include "android/wrap/hash_set.h"
#include "android/wrap/log_utils.h"
#include "android/wrap/wml_bridge.h"
#include "android/wrap/wx_js_object.h"
#include "android/wrap/wx_map.h"
#include "base/android/jni/android_jni.h"
#include "base/android/jni/jbytearray_ref.h"
#include "base/android/jni_type.h"
#include "base/log_defines.h"
#include "core/layout/layout.h"
#include "core/layout/measure_func_adapter.h"
#include "core/manager/weex_core_manager.h"
#include "third_party/IPC/IPCResult.h"

namespace WeexCore {

AndroidSide::AndroidSide()
    : wml_bridge_(WMLBridge::Instance()), wx_bridge_(WXBridge::Instance()) {}

AndroidSide::~AndroidSide() {}

WXCoreSize AndroidSide::InvokeMeasureFunction(const char *page_id,
                                              long render_ptr, float width,
                                              int width_measure_mode,
                                              float height,
                                              int height_measure_mode) {
  return WeexCoreManager::Instance()->measure_function_adapter()->Measure(
      page_id, render_ptr, width, static_cast<MeasureMode>(width_measure_mode),
      height, static_cast<MeasureMode>(height_measure_mode));
}
void AndroidSide::InvokeLayoutBefore(const char *page_id, long render_ptr) {
  WeexCoreManager::Instance()->measure_function_adapter()->LayoutBefore(
      page_id, render_ptr);
}

void AndroidSide::InvokeLayoutPlatform(const char* page_id, long render_ptr) {
  // do nothing
}

void AndroidSide::TriggerVSync(const char* page_id) {
  // do nothing
}

void AndroidSide::InvokeLayoutAfter(const char *page_id, long render_ptr,
                                    float width, float height) {
  WeexCoreManager::Instance()->measure_function_adapter()->LayoutAfter(
      page_id, render_ptr, width, height);
}

void AndroidSide::SetJSVersion(const char *version) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wx_bridge_->SetJSFrmVersion(env, version);
}

void AndroidSide::ReportException(const char *page_id, const char *func,
                                  const char *exception_string) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wx_bridge_->ReportException(env, page_id, func, exception_string);
}

void AndroidSide::ReportServerCrash(const char *instance_id) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  std::string crash_file;
  crash_file.assign("/crash_dump.log");
  wx_bridge_->ReportServerCrash(env, instance_id, crash_file.c_str());
}

void AndroidSide::ReportNativeInitStatus(const char *status_code,
                                         const char *error_msg) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wx_bridge_->ReportNativeInitStatus(env, status_code, error_msg);
}

int AndroidSide::CallNative(const char *page_id, const char *task,
                            const char *callback) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->CallNative(env, page_id, task, callback);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callNative");
  }
  return flag;
}

std::unique_ptr<ValueWithType> AndroidSide::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  std::unique_ptr<ValueWithType> ipc_result(new ValueWithType());
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return ipc_result;

  auto result =
      wx_bridge_->CallNativeModule(env, page_id, module, method, arguments,
                                   arguments_length, options, options_length);

  ipc_result->type = ParamsType::INT32;
  ipc_result->value.int32Value = -1;
  if (result.IsNull()) {
    return ipc_result;
  }

  auto wx_js_object_result =
      std::unique_ptr<WXJSObject>(new WXJSObject(env, result.Get()));
  jint jTypeInt = wx_js_object_result->GetType(env);
  auto jDataObj = wx_js_object_result->GetData(env);

  if (jTypeInt == 1) {
    ipc_result->type = ParamsType::DOUBLE;
    ipc_result->value.doubleValue =
        base::android::JNIType::DoubleValue(env, jDataObj.Get());
  } else if (jTypeInt == 2) {
    jstring jDataStr = (jstring)jDataObj.Get();
    ipc_result->type = ParamsType::STRING;
    ipc_result->value.string = jstring2WeexString(env, jDataStr);
  } else if (jTypeInt == 3) {
    jstring jDataStr = (jstring)jDataObj.Get();
    ipc_result->type = ParamsType::JSONSTRING;
    ipc_result->value.string = jstring2WeexString(env, jDataStr);
  } else if (jTypeInt == 4) {
    jbyteArray array = (jbyteArray)jDataObj.Get();
    if (array != nullptr) {
      int length = env->GetArrayLength(array);
      void *data = env->GetByteArrayElements(array, 0);
      ipc_result->type = ParamsType::BYTEARRAY;
      ipc_result->value.byteArray =
          genWeexByteArray((const char *)data, length);
      env->ReleaseByteArrayElements(array, (jbyte *)data, 0);
    }
  }

  return ipc_result;
}

void AndroidSide::CallNativeComponent(const char *page_id, const char *ref,
                                      const char *method, const char *arguments,
                                      int arguments_length, const char *options,
                                      int options_length) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wx_bridge_->CallNativeComponent(env, page_id, ref, method, arguments,
                                  arguments_length, options, options_length);
}

void AndroidSide::SetTimeout(const char *callback_id, const char *time) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wx_bridge_->SetTimeout(env, callback_id, time);
}

void AndroidSide::NativeLog(const char *str_array) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  LogUtils::NativeLog(env, str_array);
}

int AndroidSide::UpdateFinish(const char *page_id, const char *task,
                              int taskLen, const char *callback,
                              int callbackLen) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->UpdateFinish(env, page_id, task, callback);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callUpdateFinish");
  }
  return flag;
}

int AndroidSide::RefreshFinish(const char *page_id, const char *task,
                               const char *callback) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->RefreshFinish(env, page_id, task, callback);

  if (flag == -1) {
    LOGE("instance destroy JFM must stop callNative");
  }
  return flag;
}

int AndroidSide::AddEvent(const char *page_id, const char *ref,
                          const char *event) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->AddEvent(env, page_id, ref, event);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callAddEvent");
  }
  return flag;
}

int AndroidSide::RemoveEvent(const char *page_id, const char *ref,
                             const char *event) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->RemoveEvent(env, page_id, ref, event);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callRemoveElement");
  }
  return flag;
}

int AndroidSide::CreateBody(const char *page_id, const char *component_type,
                            const char *ref,
                            std::map<std::string, std::string> *styles,
                            std::map<std::string, std::string> *attributes,
                            std::set<std::string> *events,
                            const WXCoreMargin &margins,
                            const WXCorePadding &paddings,
                            const WXCoreBorderWidth &borders) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag =
      wx_bridge_->CreateBody(env, page_id, component_type, ref, styles,
                             attributes, events, margins, paddings, borders);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callCreateBody");
  }
  return flag;
}

int AndroidSide::AddElement(const char *page_id, const char *component_type,
                            const char *ref, int &index, const char *parentRef,
                            std::map<std::string, std::string> *styles,
                            std::map<std::string, std::string> *attributes,
                            std::set<std::string> *events,
                            const WXCoreMargin &margins,
                            const WXCorePadding &paddings,
                            const WXCoreBorderWidth &borders, bool willLayout) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->AddElement(env, page_id, component_type, ref, index,
                                    parentRef, styles, attributes, events,
                                    margins, paddings, borders, willLayout);

  if (flag == -1) {
    LOGE("instance destroy JFM must stop callAddElement");
  }
  return flag;
}

int AndroidSide::Layout(const char *page_id, const char *ref, float top,
                        float bottom, float left, float right, float height,
                        float width, bool isRTL, int index) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = 0;
  wx_bridge_->Layout(env, page_id, ref, top, bottom, left, right,
                     height, width, isRTL, index);

  if (flag == -1) {
    LOGE("instance destroy JFM must stop callLayout");
  }
  return flag;
}

int AndroidSide::UpdateStyle(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style,
    std::vector<std::pair<std::string, std::string>> *margin,
    std::vector<std::pair<std::string, std::string>> *padding,
    std::vector<std::pair<std::string, std::string>> *border) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->UpdateStyle(env, page_id, ref, style, margin, padding,
                                     border);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callUpdateStyle");
  }
  return flag;
}

int AndroidSide::UpdateAttr(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = 0;
  flag = wx_bridge_->UpdateAttr(env, page_id, ref, attrs);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callUpdateStyle");
  }
  return flag;
}

int AndroidSide::CreateFinish(const char *page_id) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->CreateFinish(env, page_id);

  if (flag == -1) {
    LOGE("instance destroy JFM must stop callCreateFinish");
  }
  return flag;
}

int AndroidSide::RenderSuccess(const char *page_id) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->RenderSuccess(env, page_id);

  if (flag == -1) {
    LOGE("instance destroy JFM must stop callRenderFinish");
  }
  return flag;
}

int AndroidSide::RemoveElement(const char *page_id, const char *ref) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->RemoveElement(env, page_id, ref);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callRemoveElement");
  }
  return flag;
}

int AndroidSide::MoveElement(const char *page_id, const char *ref,
                             const char *parent_ref, int index) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  int flag = wx_bridge_->MoveElement(env, page_id, ref, parent_ref, index);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callRemoveElement");
  }
  return flag;
}

int AndroidSide::AppendTreeCreateFinish(const char *page_id, const char *ref) {
  JNIEnv *env = base::android::AttachCurrentThread();
  int flag = wx_bridge_->AppendTreeCreateFinish(env, page_id, ref);
  if (flag == -1) {
    LOGE("instance destroy JFM must stop callAppendTreeCreateFinish");
  }
  return flag;
}

int AndroidSide::HasTransitionPros(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return -1;

  if (style != nullptr) {
    return wx_bridge_->HasTransitionPros(env, page_id, ref, *style);
  } else {
    return wx_bridge_->HasTransitionPros(
        env, page_id, ref, std::vector<std::pair<std::string, std::string>>());
  }
}

void AndroidSide::PostMessage(const char *vm_id, const char *data, int dataLength) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wml_bridge_->PostMessage(env, vm_id, data, dataLength);
}

void AndroidSide::DispatchMessage(const char *client_id,
                                  const char *data, int dataLength, const char *callback, const char *vm_id) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wml_bridge_->DispatchMessage(env, client_id,  data, dataLength, callback, vm_id);
}

std::unique_ptr<WeexJSResult> AndroidSide::DispatchMessageSync(
    const char *client_id, const char *data, int dataLength,
    const char *vm_id) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return std::unique_ptr<WeexJSResult>();

  auto jni_result =
      wml_bridge_->DispatchMessageSync(env, client_id, data, dataLength, vm_id);
  JByteArrayRef byte_array(env, jni_result.Get());
  char *copy = nullptr;
  if (byte_array.length() > 0) {
    copy = new char[byte_array.length()];
    strcpy(copy, byte_array.getBytes());
  }
  return std::unique_ptr<WeexJSResult>(
      new WeexJSResult(std::unique_ptr<char[]>(copy), byte_array.length()));
}

void AndroidSide::OnReceivedResult(long callback_id, std::unique_ptr<WeexJSResult>& result) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return;

  wx_bridge_->OnReceivedResult(env, callback_id,  result);
}

jobject AndroidSide::getMeasureFunc(const char *pageId, jlong renderObjectPtr) {
  JNIEnv *env = base::android::AttachCurrentThread();
  if (env == nullptr)
    return nullptr;

  return wx_bridge_->GetMeasureFunc(env, pageId, renderObjectPtr).Release();
}
}  // namespace WeexCore
