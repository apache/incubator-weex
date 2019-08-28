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
//
// Created by yxp on 2018/6/15.
//

#include "core_side_in_multi_so.h"

#include "android/jsengine/object/args.h"
#include "base/android/log_utils.h"
#include "include/WeexApiHeader.h"

namespace weex {
namespace bridge {
namespace js {

void CoreSideInMultiSo::CallNative(const char *page_id, const char *task,
                                   const char *callback) {
  functions_exposed_by_core_->funcCallNative(page_id, task, callback);
}

std::unique_ptr<ValueWithType> CoreSideInMultiSo::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  return functions_exposed_by_core_->funcCallNativeModule(
      page_id, module, method, arguments, arguments_length, options,
      options_length);
}

void CoreSideInMultiSo::CallNativeComponent(const char *page_id,
                                            const char *ref, const char *method,
                                            const char *arguments,
                                            int arguments_length,
                                            const char *options,
                                            int options_length) {
  functions_exposed_by_core_->funcCallNativeComponent(
      page_id, ref, method, arguments, arguments_length, options,
      options_length);
}

void CoreSideInMultiSo::AddElement(const char *page_id, const char *parent_ref,
                                   const char *dom_str, int dom_str_length,
                                   const char *index_str) {
  functions_exposed_by_core_->funcCallAddElement(page_id, parent_ref, dom_str, dom_str_length,
                                                 index_str);
}

void CoreSideInMultiSo::SetTimeout(const char *callback_id, const char *time) {
  functions_exposed_by_core_->funcSetTimeout(callback_id, time);
}

void CoreSideInMultiSo::NativeLog(const char *str_array) {
  functions_exposed_by_core_->funcCallNativeLog(str_array);
}

void CoreSideInMultiSo::CreateBody(const char *page_id, const char *dom_str,
                                   int dom_str_length) {
  functions_exposed_by_core_->funcCallCreateBody(page_id, dom_str, dom_str_length);
}

int CoreSideInMultiSo::UpdateFinish(const char *page_id, const char *task,
                                    int task_length, const char *callback,
                                    int callback_length) {
  return functions_exposed_by_core_->funcCallUpdateFinish(page_id, task, task_length,
                                                          callback, callback_length);
}

void CoreSideInMultiSo::CreateFinish(const char *page_id) {
  functions_exposed_by_core_->funcCallCreateFinish(page_id);
}

int CoreSideInMultiSo::RefreshFinish(const char *page_id, const char *task,
                                     const char *callback) {
  return functions_exposed_by_core_->funcCallRefreshFinish(page_id, task,
                                                           callback);
}

void CoreSideInMultiSo::UpdateAttrs(const char *page_id, const char *ref,
                                    const char *data, int data_length) {
  functions_exposed_by_core_->funcCallUpdateAttrs(page_id, ref, data, data_length);
}

void CoreSideInMultiSo::UpdateStyle(const char *page_id, const char *ref,
                                    const char *data, int data_length) {
  functions_exposed_by_core_->funcCallUpdateStyle(page_id, ref, data, data_length);
}

void CoreSideInMultiSo::RemoveElement(const char *page_id, const char *ref) {
  functions_exposed_by_core_->funcCallRemoveElement(page_id, ref);
}

void CoreSideInMultiSo::MoveElement(const char *page_id, const char *ref,
                                    const char *parent_ref, int index) {
  functions_exposed_by_core_->funcCallMoveElement(page_id, ref, parent_ref,
                                                  index);
}

void CoreSideInMultiSo::AddEvent(const char *page_id, const char *ref,
                                 const char *event) {
  functions_exposed_by_core_->funcCallAddEvent(page_id, ref, event);
}

void CoreSideInMultiSo::RemoveEvent(const char *page_id, const char *ref,
                                    const char *event) {
  functions_exposed_by_core_->funcCallRemoveEvent(page_id, ref, event);
}

const char *CoreSideInMultiSo::CallGCanvasLinkNative(const char *context_id,
                                                     int type,
                                                     const char *arg) {
  return functions_exposed_by_core_->funcCallGCanvasLinkNative(context_id, type,
                                                               arg);
}

int CoreSideInMultiSo::SetInterval(const char *page_id, const char *callback_id,
                                   const char *time) {
  return functions_exposed_by_core_->funcSetInterval(page_id, callback_id,
                                                     time);
}

void CoreSideInMultiSo::ClearInterval(const char *page_id,
                                      const char *callback_id) {
//  LOGE("CoreSideInMultiSo::ClearInterval");
  functions_exposed_by_core_->funcClearInterval(page_id, callback_id);
}

const char *CoreSideInMultiSo::CallT3DLinkNative(int type, const char *arg) {
  return functions_exposed_by_core_->funcT3dLinkNative(type, arg);
}

void CoreSideInMultiSo::PostMessage(const char *vim_id, const char *data, int dataLength) {
  functions_exposed_by_core_->funcCallHandlePostMessage(vim_id, data, dataLength);
}

void CoreSideInMultiSo::DispatchMessage(const char *client_id, const char *data, int dataLength,
                                        const char *callback,
                                        const char *vm_id) {
  functions_exposed_by_core_->funcCallDIspatchMessage(client_id, data, dataLength, callback,
                                                      vm_id);
}

std::unique_ptr<WeexJSResult> CoreSideInMultiSo::DispatchMessageSync(
    const char *client_id, const char *data, int dataLength,
    const char *vm_id) {
  return functions_exposed_by_core_->funcCallDispatchMessageSync(client_id, data,
                                                                 dataLength, vm_id);
}

void CoreSideInMultiSo::ReportException(const char *page_id, const char *func,
                                        const char *exception_string) {
  functions_exposed_by_core_->funcReportException(page_id, func,
                                                  exception_string);
}

void CoreSideInMultiSo::SetJSVersion(const char *js_version) {
  functions_exposed_by_core_->funcSetJSVersion(js_version);
}

void CoreSideInMultiSo::OnReceivedResult(long callback_id, std::unique_ptr<WeexJSResult> &result) {
  functions_exposed_by_core_->funcOnReceivedResult(callback_id, result);
}

void CoreSideInMultiSo::UpdateComponentData(const char *page_id,
                                            const char *cid,
                                            const char *json_data) {
  functions_exposed_by_core_->funcUpdateComponentData(page_id, cid, json_data);
}

bool CoreSideInMultiSo::Log(int level, const char *tag,
                            const char *file,
                            unsigned long line,
                            const char *log) {
  return functions_exposed_by_core_->funcLog(level, tag, file, line, log);
}

}  // namespace js
}  // namespace bridge
}  // namespace weex
