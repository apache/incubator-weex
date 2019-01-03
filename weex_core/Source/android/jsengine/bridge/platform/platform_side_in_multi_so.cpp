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
// Created by yxp on 2018/6/14.
//

#include "platform_side_in_multi_so.h"
#include "android/base/log_utils.h"
#include "core/layout/layout.h"

namespace weex {
WXCoreSize PlatformSideInMultiSo::InvokeMeasureFunction(
    const char *page_id, long render_ptr, float width, int width_measure_mode,
    float height, int height_measure_mode) {
  return platform_expose_functions_->invoke_measure_function(
      page_id, render_ptr, width, width_measure_mode, height,
      height_measure_mode);
}
void PlatformSideInMultiSo::InvokeLayoutBefore(const char *page_id,
                                               long render_ptr) {
  platform_expose_functions_->invoke_layout_before(page_id, render_ptr);
}
void PlatformSideInMultiSo::InvokeLayoutAfter(const char *page_id,
                                              long render_ptr, float width,
                                              float height) {
  platform_expose_functions_->invoke_layout_after(page_id, render_ptr, width,
                                                  height);
}

void PlatformSideInMultiSo::SetJSVersion(const char *version) {
  platform_expose_functions_->set_js_version(version);
}

void PlatformSideInMultiSo::ReportException(const char *page_id,
                                            const char *func,
                                            const char *exception_string) {
  platform_expose_functions_->report_exception(page_id, func, exception_string);
}

void PlatformSideInMultiSo::ReportServerCrash(const char *instance_id) {
  platform_expose_functions_->report_server_crash(instance_id);
}

void PlatformSideInMultiSo::ReportNativeInitStatus(const char *status_code,
                                                   const char *error_msg) {
  platform_expose_functions_->report_native_Init_status(status_code, error_msg);
}

int PlatformSideInMultiSo::CallNative(const char *page_id, const char *task,
                                      const char *callback) {
  return platform_expose_functions_->call_native(page_id, task, callback);
}

std::unique_ptr<IPCResult> PlatformSideInMultiSo::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  return platform_expose_functions_->call_native_module(
      page_id, module, method, arguments, arguments_length, options,
      options_length);
}

void PlatformSideInMultiSo::CallNativeComponent(
    const char *page_id, const char *ref, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  platform_expose_functions_->call_native_component(page_id, ref, method,
                                                    arguments, arguments_length,
                                                    options, options_length);
}

void PlatformSideInMultiSo::SetTimeout(const char *callback_id,
                                       const char *time) {
  platform_expose_functions_->set_timeout(callback_id, time);
}

void PlatformSideInMultiSo::NativeLog(const char *str_array) {
  platform_expose_functions_->native_log(str_array);
}

int PlatformSideInMultiSo::UpdateFinish(const char *page_id, const char *task, int taskLen,
                                        const char* callback, int callbackLen) {
  return platform_expose_functions_->update_finish(page_id, task,taskLen, callback,callbackLen);
}

int PlatformSideInMultiSo::RefreshFinish(const char *page_id, const char *task,
                                         const char *callback) {
  return platform_expose_functions_->refresh_finish(page_id, task, callback);
}

int PlatformSideInMultiSo::AddEvent(const char *page_id, const char *ref,
                                    const char *event) {
  return platform_expose_functions_->add_event(page_id, ref, event);
}

int PlatformSideInMultiSo::RemoveEvent(const char *page_id, const char *ref,
                                       const char *event) {
  return platform_expose_functions_->remove_event(page_id, ref, event);
}

int PlatformSideInMultiSo::CreateBody(
    const char *page_id, const char *component_type, const char *ref,
    std::map<std::string, std::string> *styles,
    std::map<std::string, std::string> *attributes,
    std::set<std::string> *events, const WXCoreMargin &margins,
    const WXCorePadding &paddings, const WXCoreBorderWidth &borders) {
  return platform_expose_functions_->create_body(page_id, component_type, ref,
                                                 styles, attributes, events,
                                                 margins, paddings, borders);
}

int PlatformSideInMultiSo::AddElement(
    const char *page_id, const char *component_type, const char *ref,
    int &index, const char *parentRef,
    std::map<std::string, std::string> *styles,
    std::map<std::string, std::string> *attributes,
    std::set<std::string> *events, const WXCoreMargin &margins,
    const WXCorePadding &paddings, const WXCoreBorderWidth &borders,
    bool willLayout) {
  return platform_expose_functions_->add_element(
      page_id, component_type, ref, index, parentRef, styles, attributes,
      events, margins, paddings, borders, willLayout);
}

int PlatformSideInMultiSo::Layout(const char* page_id, const char* ref, float top,
                  float bottom, float left, float right, float height,
                  float width, bool isRTL, int index) {
  return platform_expose_functions_->layout(page_id, ref, top, bottom, left,
                                            right, height, width, isRTL, index);
}

int PlatformSideInMultiSo::UpdateStyle(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style,
    std::vector<std::pair<std::string, std::string>> *margin,
    std::vector<std::pair<std::string, std::string>> *padding,
    std::vector<std::pair<std::string, std::string>> *border) {
  return platform_expose_functions_->update_style(page_id, ref, style, margin,
                                                  padding, border);
}

int PlatformSideInMultiSo::UpdateAttr(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  return platform_expose_functions_->update_attrs(page_id, ref, attrs);
}

int PlatformSideInMultiSo::CreateFinish(const char *page_id) {
  return platform_expose_functions_->create_finish(page_id);
}

int PlatformSideInMultiSo::RenderSuccess(const char *page_id) {
  return platform_expose_functions_->render_success(page_id);
}

int PlatformSideInMultiSo::RemoveElement(const char *page_id, const char *ref) {
  return platform_expose_functions_->remove_element(page_id, ref);
}

int PlatformSideInMultiSo::MoveElement(const char *page_id, const char *ref,
                                       const char *parent_ref, int index) {
  return platform_expose_functions_->move_element(page_id, ref, parent_ref,
                                                  index);
}

int PlatformSideInMultiSo::AppendTreeCreateFinish(const char *page_id,
                                                  const char *ref) {
  return platform_expose_functions_->append_tree_create_finish(page_id, ref);
}

int PlatformSideInMultiSo::HasTransitionPros(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style) {
  return platform_expose_functions_->has_transition_pros(page_id, ref, style);
}

void PlatformSideInMultiSo::PostMessage(const char *vm_id, const char *data) {
  platform_expose_functions_->post_message(vm_id, data);
}

void PlatformSideInMultiSo::DispatchMessage(const char *client_id,
                                            const char *vm_id, const char *data,
                                            const char *callback) {
  platform_expose_functions_->dispatch_message(client_id, vm_id, data,
                                               callback);
}
}  // namespace weex