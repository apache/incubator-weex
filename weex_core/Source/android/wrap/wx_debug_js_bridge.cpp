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

#include "wx_debug_js_bridge.h"

#include "android/base/string/string_utils.h"
#include "android/wrap/wx_bridge.h"
#include "base/android/jniprebuild/jniheader/WXDebugJsBridge_jni.h"
#include "base/android/jni/jbytearray_ref.h"
#include "base/make_copyable.h"
#include "base/thread/waitable_event.h"
#include "base/string_util.h"
#include "core/render/manager/render_manager.h"
#include "core/bridge/platform_bridge.h"
#include "core/common/view_utils.h"
#include "core/manager/weex_core_manager.h"
#include "third_party/IPC/IPCResult.h"
#include "wson/wson_parser.h"

using namespace WeexCore;

namespace WeexCore {
  bool RegisterWXDebugJsBridge(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}

void resetWXBridge(JNIEnv *env, jobject object, jobject bridge, jstring className) {
  ScopedJStringUTF8 classNameRef = ScopedJStringUTF8(env, className);
  WXBridge::Instance()->Reset(env, bridge);
  WXBridge::Instance()->reset_clazz(env, classNameRef.getChars());
}

void jsHandleSetJSVersion(JNIEnv *env, jobject object, jstring jsVersion) {
  ScopedJStringUTF8 js_version_ref = ScopedJStringUTF8(env, jsVersion);
  WeexCoreManager::Instance()->script_bridge()->core_side()->SetJSVersion(
      js_version_ref.getChars());
}

void jsHandleReportException(JNIEnv *env, jobject object, jstring instanceId, jstring func,
                             jstring exceptionjstring) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  ScopedJStringUTF8 func_ref = ScopedJStringUTF8(env, func);
  ScopedJStringUTF8 exception_string = ScopedJStringUTF8(env, exceptionjstring);

  WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->ReportException(page_id_ref.getChars(), func_ref.getChars(), exception_string.getChars());
}

void jsHandleCallNative(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                        jstring callback) {


  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  JByteArrayRef tasks_ref = JByteArrayRef(env, tasks);
  ScopedJStringUTF8 callbask_ref = ScopedJStringUTF8(env, callback);

  WeexCoreManager::Instance()->script_bridge()->core_side()->CallNative(page_id_ref.getChars(),
                                                                        tasks_ref.getBytes(),
                                                                        callbask_ref.getChars());
}

void jsHandleCallNativeModule(JNIEnv *env, jobject object, jstring instanceId, jstring module,
                              jstring method, jbyteArray arguments, jbyteArray options,
                              jboolean from) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  ScopedJStringUTF8 module_ref = ScopedJStringUTF8(env, module);
  ScopedJStringUTF8 method_ref = ScopedJStringUTF8(env, method);
  JByteArrayRef arg = JByteArrayRef(env, arguments);
  JByteArrayRef opt = JByteArrayRef(env, options);

  std::string ret_str = "";

  std::unique_ptr<ValueWithType> ret = WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->CallNativeModule(page_id_ref.getChars(), module_ref.getChars(), method_ref.getChars(),
                         arg.getBytes(), arg.length(),
                         opt.getBytes(), opt.length());


  switch (ret.get()->type) {
    case ParamsType::INT32:
      ret_str = to_string(ret.get()->value.int32Value);
      break;
    case ParamsType::INT64:
      ret_str = to_string(ret.get()->value.int64Value);
      break;
    case ParamsType::FLOAT:
    case ParamsType::DOUBLE:
      ret_str = to_string(ret.get()->value.doubleValue);
      break;
    case ParamsType::VOID:
      break;
    case ParamsType::BYTEARRAY:
      //ret.get()->value.byteArray->content;
      //ret.get()->value.byteArray->length;
      break;
    case ParamsType::JSONSTRING:
      weex::base::to_utf8(ret.get()->value.string->content, ret.get()->value.string->length);
      break;
    case ParamsType::STRING:
      weex::base::to_utf8(ret.get()->value.string->content, ret.get()->value.string->length);
      break;
    default:
      break;
  }
}

void jsHandleCallNativeComponent(JNIEnv *env, jobject object, jstring instanceId, jstring componentRef,
                            jstring method,
                            jbyteArray arguments, jbyteArray options, jboolean from) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  ScopedJStringUTF8 component_ref_ref = ScopedJStringUTF8(env, componentRef);
  ScopedJStringUTF8 method_ref = ScopedJStringUTF8(env, method);
  JByteArrayRef arg = JByteArrayRef(env, arguments);
  JByteArrayRef opt = JByteArrayRef(env, options);

  WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->CallNativeComponent(page_id_ref.getChars(), component_ref_ref.getChars(),
                            method_ref.getChars(),
                            arg.getBytes(), arg.length(), opt.getBytes(), opt.length());
}

void jsHandleCallAddElement(JNIEnv *env, jobject object, jstring instanceId, jstring ref, jbyteArray dom,
                       jstring index) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);
  JByteArrayRef dom_ref = JByteArrayRef(env, dom);
  ScopedJStringUTF8 index_ref = ScopedJStringUTF8(env, index);

  WeexCoreManager::Instance()->script_bridge()->core_side()->AddElement(page_id_ref.getChars(),
                                                                        ref_ref.getChars(),
                                                                        dom_ref.getBytes(),
                                                                        dom_ref.length(),
                                                                        index_ref.getChars());
}

void jsHandleSetTimeout(JNIEnv *env, jobject object, jstring callbackId, jstring time) {

  ScopedJStringUTF8 callback_id_ref = ScopedJStringUTF8(env, callbackId);
  ScopedJStringUTF8 time_ref = ScopedJStringUTF8(env, time);
  WeexCoreManager::Instance()->script_bridge()->core_side()->SetTimeout(callback_id_ref.getChars(),
                                                                        time_ref.getChars());
}

void jsHandleCallNativeLog(JNIEnv *env, jobject object, jbyteArray str_array) {
  JByteArrayRef str_array_ref = JByteArrayRef(env, str_array);
  WeexCoreManager::Instance()->script_bridge()->core_side()->NativeLog(str_array_ref.getBytes());
}

void jsHandleCallCreateBody(JNIEnv *env, jobject object, jstring pageId, jbyteArray domStr,
                              jboolean from) {
  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  JByteArrayRef dom_str_ref = JByteArrayRef(env, domStr);
  WeexCoreManager::Instance()->script_bridge()->core_side()->CreateBody(page_id_ref.getChars(),
                                                                        dom_str_ref.getBytes(),
                                                                        dom_str_ref.length());
}

void jsHandleCallUpdateFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                                jstring callback) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  JByteArrayRef tasks_ref = JByteArrayRef(env, tasks);
  ScopedJStringUTF8 callback_ref = ScopedJStringUTF8(env, callback);

  int result = -1;
  result = WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->UpdateFinish(page_id_ref.getChars(), tasks_ref.getBytes(), tasks_ref.length(),
                     callback_ref.getChars(), strlen(callback_ref.getChars()));
}

void jsHandleCallCreateFinish(JNIEnv *env, jobject object, jstring pageId) {
  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(page_id_ref.getChars());
}

void jsHandleCallRefreshFinish(JNIEnv *env, jobject object, jstring instanceId, jbyteArray tasks,
                                 jstring callback) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, instanceId);
  JByteArrayRef tasks_ref = JByteArrayRef(env, tasks);
  ScopedJStringUTF8 callback_ref = ScopedJStringUTF8(env, callback);

  WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->RefreshFinish(page_id_ref.getChars(), tasks_ref.getBytes(), callback_ref.getChars());
}

void jsHandleCallUpdateAttrs(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data,
                          jboolean from) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);
  JByteArrayRef data_ref = JByteArrayRef(env, data);

  WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateAttrs(page_id_ref.getChars(),
                                                                         ref_ref.getChars(),
                                                                         data_ref.getBytes(),
                                                                         data_ref.length());
}

void jsHandleCallUpdateStyle(JNIEnv *env, jobject object, jstring pageId, jstring ref, jbyteArray data,
                          jboolean from) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);
  JByteArrayRef data_ref = JByteArrayRef(env, data);

  WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateStyle(page_id_ref.getChars(),
                                                                         ref_ref.getChars(),
                                                                         data_ref.getBytes(),
                                                                         data_ref.length());
}

void jsHandleCallRemoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);

  WeexCoreManager::Instance()
      ->script_bridge()
      ->core_side()
      ->RemoveElement(page_id_ref.getChars(),
                      ref_ref.getChars());
}

void jsHandleCallMoveElement(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                               jstring parentRef, jstring index_str) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);
  ScopedJStringUTF8 parent_ref_ref = ScopedJStringUTF8(env, parentRef);
  ScopedJStringUTF8 index_str_ref = ScopedJStringUTF8(env, index_str);
  int index_number = atoi(index_str_ref.getChars());

  WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement(
      page_id_ref.getChars(),
      ref_ref.getChars(),
      parent_ref_ref.getChars(),
      index_number);
}

void jsHandleCallAddEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref, jstring event) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);
  ScopedJStringUTF8 event_ref = ScopedJStringUTF8(env, event);
  WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent(page_id_ref.getChars(),
                                                                      ref_ref.getChars(),
                                                                      event_ref.getChars());
}

void jsHandleCallRemoveEvent(JNIEnv *env, jobject object, jstring pageId, jstring ref,
                               jstring event) {

  ScopedJStringUTF8 page_id_ref = ScopedJStringUTF8(env, pageId);
  ScopedJStringUTF8 ref_ref = ScopedJStringUTF8(env, ref);
  ScopedJStringUTF8 event_ref = ScopedJStringUTF8(env, event);
  WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent(page_id_ref.getChars(),
                                                                         ref_ref.getChars(),
                                                                         event_ref.getChars());
}

void jsHandleSetInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId,
                         jstring time) {}

void jsHandleClearInterval(JNIEnv *env, jobject object, jstring instanceId, jstring callbackId) {}

void jsHandleCallGCanvasLinkNative(JNIEnv *env, jobject object, jstring contextId, int type,
                                   jstring val) {}
