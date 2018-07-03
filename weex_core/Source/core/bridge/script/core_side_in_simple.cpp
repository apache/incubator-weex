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

#include "core_side_in_simple.h"

#include <cstdlib>
#include "IPC/IPCResult.h"
#include "android/base/log_utils.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"

namespace WeexCore {
namespace bridge {
namespace script {
CoreSideInSimple::CoreSideInSimple() {}

CoreSideInSimple::~CoreSideInSimple() {}

void CoreSideInSimple::CallNative(const char *page_id, const char *task,
                                  const char *callback) {
  LOGE("Script Bridge Core Side Simple::CallNative");
  if (page_id == nullptr || task == nullptr) return;
  if (strcmp(
          task,
          "[{\"module\":\"dom\",\"method\":\"createFinish\",\"args\":[]}]") ==
      0) {
    RenderManager::GetInstance()->CreateFinish(page_id) ? 0 : -1;
  } else {
    WeexCoreManager::getInstance()
        ->getPlatformBridge()
        ->platform_side()
        ->CallNative(page_id, task, callback);
  }
}

std::unique_ptr<IPCResult> CoreSideInSimple::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  LOGE("Script Bridge Core Side Simple::CallNativeModule");
  std::unique_ptr<IPCResult> ret = createInt32Result(-1);
  if (page_id != nullptr && module != nullptr && method != nullptr) {
    RenderManager::GetInstance()->CallNativeModule(page_id, module, method,
                                                   arguments, arguments_length,
                                                   options, options_length);

    return WeexCoreManager::getInstance()
        ->getPlatformBridge()
        ->platform_side()
        ->CallNativeModule(page_id, module, method, arguments, arguments_length,
                           options, options_length);
  }

  return ret;
}

void CoreSideInSimple::CallNativeComponent(const char *page_id, const char *ref,
                                           const char *method,
                                           const char *arguments,
                                           int arguments_length,
                                           const char *options,
                                           int options_length) {
  LOGE("Script Bridge Core Side Simple::CallNativeComponent");
  if (page_id != nullptr && ref != nullptr && method != nullptr) {
    WeexCoreManager::getInstance()
        ->getPlatformBridge()
        ->platform_side()
        ->CallNativeComponent(page_id, ref, method, arguments, arguments_length,
                              options, options_length);
  }
}

void CoreSideInSimple::AddElement(const char *page_id, const char *parent_ref,
                                  const char *dom_str, int dom_str_length,
                                  const char *index_str) {
  LOGE("Script Bridge Core Side Simple::AddElement");
  const char *indexChar = index_str == nullptr ? "\0" : index_str;

  int index = atoi(indexChar);
  if (page_id == nullptr || parent_ref == nullptr || dom_str == nullptr ||
      index < -1)
    return;

  RenderManager::GetInstance()->AddRenderObject(page_id, parent_ref, index,
                                                dom_str);
}

void CoreSideInSimple::SetTimeout(const char *callback_id, const char *time) {
  LOGE("Script Bridge Core Side Simple::SetTimeout");
  WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->SetTimeout(callback_id, time);
}

void CoreSideInSimple::NativeLog(const char *str_array) {
  LOGE("Script Bridge Core Side Simple::NativeLog");
  WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->NativeLog(str_array);
}

void CoreSideInSimple::CreateBody(const char *page_id, const char *dom_str,
                                  int dom_str_length) {
  LOGE("Script Bridge Core Side Simple::CreateBody");
  RenderManager::GetInstance()->CreatePage(page_id, dom_str) ? 0 : -1;
}

int CoreSideInSimple::UpdateFinish(const char *page_id, const char *task,
                                   int task_length, const char *callback,
                                   int callback_length) {
  LOGE("Script Bridge Core Side Simple::UpdateFinish");
  return WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->UpdateFinish(page_id, task, callback);
}

void CoreSideInSimple::CreateFinish(const char *page_id) {
  LOGE("Script Bridge Core Side Simple::CreateFinish");
  RenderManager::GetInstance()->CreateFinish(page_id);
}

int CoreSideInSimple::RefreshFinish(const char *page_id, const char *task,
                                    const char *callback) {
  LOGE("Script Bridge Core Side Simple::RefreshFinish");
  if (page_id == nullptr) return -1;
  return WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->RefreshFinish(page_id, task, callback);
}

void CoreSideInSimple::UpdateAttrs(const char *page_id, const char *ref,
                                   const char *data, int data_length) {
  LOGE("Script Bridge Core Side Simple::UpdateAttrs");
  RenderManager::GetInstance()->UpdateAttr(page_id, ref, data);
}

void CoreSideInSimple::UpdateStyle(const char *page_id, const char *ref,
                                   const char *data, int data_length) {
  LOGE("Script Bridge Core Side Simple::UpdateStyle");
  RenderManager::GetInstance()->UpdateStyle(page_id, ref, data);
}

void CoreSideInSimple::RemoveElement(const char *page_id, const char *ref) {
  LOGE("Script Bridge Core Side Simple::RemoveElement");
  RenderManager::GetInstance()->RemoveRenderObject(page_id, ref);
}

void CoreSideInSimple::MoveElement(const char *page_id, const char *ref,
                                   const char *parent_ref, int index) {
  LOGE("Script Bridge Core Side Simple::MoveElement");
  RenderManager::GetInstance()->MoveRenderObject(page_id, ref, parent_ref,
                                                 index);
}

void CoreSideInSimple::AddEvent(const char *page_id, const char *ref,
                                const char *event) {
  LOGE("Script Bridge Core Side Simple::AddEvent");
  RenderManager::GetInstance()->AddEvent(page_id, ref, event);
}

void CoreSideInSimple::RemoveEvent(const char *page_id, const char *ref,
                                   const char *event) {
  LOGE("Script Bridge Core Side Simple::RemoveEvent");
  RenderManager::GetInstance()->RemoveEvent(page_id, ref, event);
}

const char *CoreSideInSimple::CallGCanvasLinkNative(const char *context_id,
                                                    int type, const char *arg) {
  LOGE("Script Bridge Core Side Simple::CallGCanvasLinkNative");
  // TODO
  return NULL;
}

int CoreSideInSimple::SetInterval(const char *page_id, const char *callback_id,
                                  const char *time) {
  LOGE("Script Bridge Core Side Simple::SetInterval");
  return (atoi(page_id) << 16) | (atoi(callback_id));
}

void CoreSideInSimple::ClearInterval(const char *page_id,
                                     const char *callback_id) {
  // do nothing
}

const char *CoreSideInSimple::CallT3DLinkNative(int type, const char *arg) {
  LOGE("Script Bridge Core Side Simple::CallT3DLinkNative");
  // TODO
  return NULL;
}
void CoreSideInSimple::PostMessage(const char *vm_id, const char *data) {
  LOGE("Script Bridge Core Side Simple::PostMessage");
  WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->PostMessage(vm_id, data);
}

void CoreSideInSimple::DispatchMessage(const char *client_id, const char *data,
                                       const char *callback,
                                       const char *vm_id) {
  LOGE("Script Bridge Core Side Simple::DispatchMessage");
  WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->DispatchMessage(client_id, vm_id, data, callback);
}

void CoreSideInSimple::ReportException(const char *page_id, const char *func,
                                       const char *exception_string) {
  LOGE("Script Bridge Core Side Simple::ReportException");
  WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->ReportException(page_id, func, exception_string);
}

void CoreSideInSimple::SetJSVersion(const char *js_version) {
  LOGE("Script Bridge Core Side Simple::SetJSVersion");
  LOGA("init JSFrm version %s", js_version);
  WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->platform_side()
      ->SetJSVersion(js_version);
}
}  // namespace script
}  // namespace bridge
}  // namespace WeexCore