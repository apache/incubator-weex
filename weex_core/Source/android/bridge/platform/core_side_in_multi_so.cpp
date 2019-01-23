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

#include "core_side_in_multi_so.h"

namespace WeexCore {
void CoreSideInMultiSo::SetDefaultHeightAndWidthIntoRootDom(
    const std::string &instance_id, float default_width, float default_height,
    bool is_width_wrap_content, bool is_height_wrap_content) {
  core_side_functions_->set_default_height_and_width_into_root_dom(
      instance_id, default_width, default_height, is_width_wrap_content,
      is_height_wrap_content);
}

void CoreSideInMultiSo::OnInstanceClose(const std::string &instance_id) {
  core_side_functions_->on_instance_close(instance_id);
}

void CoreSideInMultiSo::SetStyleWidth(const std::string &instance_id,
                                      const std::string &render_ref,
                                      float width) {
  core_side_functions_->set_style_width(instance_id, render_ref, width);
}

void CoreSideInMultiSo::SetStyleHeight(const std::string &instance_id,
                                       const std::string &render_ref,
                                       float height) {
  core_side_functions_->set_style_height(instance_id, render_ref, height);
}

void CoreSideInMultiSo::SetMargin(const std::string &instance_id,
                                  const std::string &render_ref, int edge,
                                  float value) {
  core_side_functions_->set_margin(instance_id, render_ref, edge, value);
}

void CoreSideInMultiSo::SetPadding(const std::string &instance_id,
                                   const std::string &render_ref, int edge,
                                   float value) {
  core_side_functions_->set_padding(instance_id, render_ref, edge, value);
}

void CoreSideInMultiSo::SetPosition(const std::string &instance_id,
                                    const std::string &render_ref, int edge,
                                    float value) {
  core_side_functions_->set_position(instance_id, render_ref, edge, value);
}

void CoreSideInMultiSo::MarkDirty(const std::string &instance_id,
                                  const std::string &render_ref) {
  core_side_functions_->mark_dirty(instance_id, render_ref);
}

void CoreSideInMultiSo::SetViewPortWidth(const std::string &instance_id,
                                         float width) {
  core_side_functions_->set_view_port_width(instance_id, width);
}

void CoreSideInMultiSo::ForceLayout(const std::string &instance_id) {
  core_side_functions_->force_layout(instance_id);
}

bool CoreSideInMultiSo::NotifyLayout(const std::string &instance_id) {
  return core_side_functions_->notify_layout(instance_id);
}

std::vector<int64_t> CoreSideInMultiSo::GetFirstScreenRenderTime(
    const std::string &instance_id) {
  return core_side_functions_->get_first_screen_render_time(instance_id);
}

std::vector<int64_t> CoreSideInMultiSo::GetRenderFinishTime(
    const std::string &instance_id) {
  return core_side_functions_->get_render_finish_time(instance_id);
}

void CoreSideInMultiSo::SetRenderContainerWrapContent(
    const std::string &instance_id, bool wrap) {
  core_side_functions_->set_render_container_wrap_content(instance_id, wrap);
}

void CoreSideInMultiSo::BindMeasurementToRenderObject(long ptr) {
  core_side_functions_->bind_measurement_to_render_object(ptr);
}

void CoreSideInMultiSo::RegisterCoreEnv(const std::string &key,
                                        const std::string &value) {
  core_side_functions_->register_core_env(key, value);
}

long CoreSideInMultiSo::GetRenderObject(const std::string &instance_id,
                                        const std::string &render_ref) {
  return core_side_functions_->get_render_object(instance_id, render_ref);
}

void CoreSideInMultiSo::UpdateRenderObjectStyle(long render_ptr,
                                                const std::string &key,
                                                const std::string &value) {
  core_side_functions_->update_render_object_style(render_ptr, key, value);
}

void CoreSideInMultiSo::UpdateRenderObjectAttr(long render_ptr,
                                               const std::string &key,
                                               const std::string &value) {
  core_side_functions_->update_render_object_attr(render_ptr, key, value);
}

long CoreSideInMultiSo::CopyRenderObject(long render_ptr) {
  return core_side_functions_->copy_render_object(render_ptr);
}

void CoreSideInMultiSo::SetMeasureFunctionAdapter() {
  core_side_functions_->set_measure_function_adapter();
}

void CoreSideInMultiSo::SetPlatform(const std::string &platform) {
  core_side_functions_->set_platform(platform);
}

void CoreSideInMultiSo::SetDeviceWidthAndHeight(float width, float height) {
  core_side_functions_->set_device_width_and_height(width, height);
}

void CoreSideInMultiSo::AddOption(const std::string &key,
                                  const std::string &value) {
  core_side_functions_->add_option(key, value);
}

////

int CoreSideInMultiSo::RefreshInstance(const char* instanceId, const char* nameSpace,
                    const char* func,
                    std::vector<VALUE_WITH_TYPE *> &params) {
  return 1;
}

int CoreSideInMultiSo::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return core_side_functions_->init_framework(script, params);
}

int CoreSideInMultiSo::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return core_side_functions_->init_app_framework(instanceId, appFramework,
                                                  params);
}

int CoreSideInMultiSo::CreateAppContext(const char *instanceId,
                                        const char *jsBundle) {
  return core_side_functions_->create_app_context(instanceId, jsBundle);
}

std::unique_ptr<WeexJSResult> CoreSideInMultiSo::ExecJSOnAppWithResult(const char *instanceId,
                                                     const char *jsBundle) {
  return core_side_functions_->exec_js__on_app_with_result(instanceId,
                                                           jsBundle);
}

int CoreSideInMultiSo::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return core_side_functions_->call_js_on_app_context(instanceId, func, params);
}

int CoreSideInMultiSo::DestroyAppContext(const char *instanceId) {
  return core_side_functions_->destroy_app_context(instanceId);
}

int CoreSideInMultiSo::ExecJsService(const char *source) {
  return core_side_functions_->exec_js_service(source);
}

int CoreSideInMultiSo::ExecTimeCallback(const char *source) { return 0; }

int CoreSideInMultiSo::ExecJS(const char *instanceId, const char *nameSpace,
                              const char *func,
                              std::vector<VALUE_WITH_TYPE *> &params) {
  return core_side_functions_->exec_js(instanceId, nameSpace, func, params);
}

std::unique_ptr<WeexJSResult> CoreSideInMultiSo::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return core_side_functions_->exec_js_with_result(instanceId, nameSpace, func,
                                                   params);
}

int CoreSideInMultiSo::CreateInstance(const char *instanceId, const char *func,
                                      const char *script, const char *opts,
                                      const char *initData,
                                      const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params, const char* render_strategy) {
  return core_side_functions_->create_instance(instanceId, func, script, opts,
                                               initData, extendsApi, params);
}

std::unique_ptr<WeexJSResult> CoreSideInMultiSo::ExecJSOnInstance(const char *instanceId,
                                                const char *script) {
  return core_side_functions_->exec_js_on_instance(instanceId, script);
}

int CoreSideInMultiSo::DestroyInstance(const char *instanceId) {
  return core_side_functions_->destroy_instance(instanceId);
}

int CoreSideInMultiSo::UpdateGlobalConfig(const char *config) {
  return core_side_functions_->update_global_config(config);
}
}  // namespace WeexCore
