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

#include "core/bridge/script_bridge_in_multi_so.h"
#include <dlfcn.h>
#include "android/base/log_utils.h"
#include "android/bridge/impl/multi_so_initializer.h"
#include "core/bridge/script/core_side_in_simple.h"
#include "core/bridge/script/script_side_in_multi_so.h"
#include "core/manager/weex_core_manager.h"

namespace WeexCore {
ScriptBridgeInMultiSo::ScriptBridgeInMultiSo() {
  set_core_side(new bridge::script::CoreSideInSimple);
  set_script_side(new bridge::script::ScriptSideInMultiSo);
  std::unique_ptr<MultiSoInitializer> initializer(new MultiSoInitializer);
  bool passable = initializer->Init(
      [this](void *handle) {
        LOGE("dlopen so and call function");

        typedef FunctionsExposedByJS *(*ExchangeJSBridgeFunctions)(
            FunctionsExposedByCore *);
        auto exchange_script_bridge_functions =
            (ExchangeJSBridgeFunctions)dlsym(handle,
                                             "ExchangeJSBridgeFunctions");
        if (!exchange_script_bridge_functions) {
          return false;
        }

        FunctionsExposedByCore *platform_expose_functions =
            GetExposedFunctions();
        auto script_side_functions =
            exchange_script_bridge_functions(platform_expose_functions);

        static_cast<bridge::script::ScriptSideInMultiSo *>(script_side())
            ->set_script_side_functions(script_side_functions);
        return true;
      },
      [this](const char *status_code, const char *error_msg) {
        WeexCoreManager::getInstance()
            ->getPlatformBridge()
            ->platform_side()
            ->ReportNativeInitStatus(status_code, error_msg);
      });
  set_is_passable(passable);
}

ScriptBridgeInMultiSo::~ScriptBridgeInMultiSo() {}

static void CallNative(const char *page_id, const char *task,
                       const char *callback) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->CallNative(
      page_id, task, callback);
}

static std::unique_ptr<IPCResult> CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  return WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->CallNativeModule(page_id, module, method, arguments, arguments_length,
                         options, options_length);
}

static void CallNativeComponent(const char *page_id, const char *ref,
                                const char *method, const char *arguments,
                                int arguments_length, const char *options,
                                int options_length) {
  WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->CallNativeComponent(page_id, ref, method, arguments, arguments_length,
                            options, options_length);
}

static void AddElement(const char *page_id, const char *parent_ref,
                       const char *dom_str, int domLen, const char *index_str) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->AddElement(
      page_id, parent_ref, dom_str, domLen, index_str);
}

static void SetTimeout(const char *callback_id, const char *time) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->SetTimeout(
      callback_id, time);
}

static void NativeLog(const char *str_array) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->NativeLog(
      str_array);
}

static void CreateBody(const char *page_id, const char *dom_str, int domLen) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->CreateBody(
      page_id, dom_str, domLen);
}

static int UpdateFinish(const char *page_id, const char *task, int taskLen,
                        const char *callback, int callbackLen) {
  return WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->UpdateFinish(page_id, task, taskLen, callback, callbackLen);
}

static void CreateFinish(const char *page_id) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->CreateFinish(
      page_id);
}

static int RefreshFinish(const char *page_id, const char *task,
                         const char *callback) {
  return WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->RefreshFinish(page_id, task, callback);
}

static void UpdateAttrs(const char *page_id, const char *ref,
                        const char *data, int dataLen) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->UpdateAttrs(
      page_id, ref, data, dataLen);
}

static void UpdateStyle(const char *page_id, const char *ref,
                        const char *data, int dataLen) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->UpdateStyle(
      page_id, ref, data, dataLen);
}

static void RemoveElement(const char *page_id, const char *ref) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->RemoveElement(
      page_id, ref);
}

static void MoveElement(const char *page_id, const char *ref,
                        const char *parent_ref, int index) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->MoveElement(
      page_id, ref, parent_ref, index);
}

static void AddEvent(const char *page_id, const char *ref, const char *event) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->AddEvent(
      page_id, ref, event);
}

static void RemoveEvent(const char *page_id, const char *ref,
                        const char *event) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->RemoveEvent(
      page_id, ref, event);
}
static const char *CallGCanvasLinkNative(const char *context_id, int type,
                                         const char *arg) {
  return WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->CallGCanvasLinkNative(context_id, type, arg);
}

static int SetInterval(const char *page_id, const char *callback_id,
                       const char *time) {
  return WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->SetInterval(page_id, callback_id, time);
}

static void ClearInterval(const char *page_id, const char *callback_id) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->ClearInterval(
      page_id, callback_id);
}

static const char *CallT3DLinkNative(int type, const char *arg) {
  return WeexCoreManager::getInstance()
      ->script_bridge()
      ->core_side()
      ->CallT3DLinkNative(type, arg);
}

static void PostMessage(const char *vim_id, const char *data) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->PostMessage(
      vim_id, data);
}

static void DispatchMessage(const char *client_id, const char *data,
                            const char *callback, const char *vm_id) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->DispatchMessage(
      client_id, data, callback, vm_id);
}
static void ReportException(const char *page_id, const char *func,
                            const char *exception_string) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->ReportException(
      page_id, func, exception_string);
}

static void SetJSVersion(const char *js_version) {
  WeexCoreManager::getInstance()->script_bridge()->core_side()->SetJSVersion(
      js_version);
}

FunctionsExposedByCore *ScriptBridgeInMultiSo::GetExposedFunctions() {
  FunctionsExposedByCore temp = {SetJSVersion,
                                 ReportException,
                                 CallNative,
                                 CallNativeModule,
                                 CallNativeComponent,
                                 AddElement,
                                 SetTimeout,
                                 NativeLog,
                                 CreateBody,
                                 UpdateFinish,
                                 CreateFinish,
                                 RefreshFinish,
                                 UpdateAttrs,
                                 UpdateStyle,
                                 RemoveElement,
                                 MoveElement,
                                 AddEvent,
                                 RemoveEvent,
                                 SetInterval,
                                 ClearInterval,
                                 CallGCanvasLinkNative,
                                 CallT3DLinkNative,
                                 PostMessage,
                                 DispatchMessage};
  auto functions =
      (FunctionsExposedByCore *)malloc(sizeof(FunctionsExposedByCore));
  memset(functions, 0, sizeof(FunctionsExposedByCore));
  memcpy(functions, &temp, sizeof(FunctionsExposedByCore));
  return functions;
}
}  // namespace WeexCore