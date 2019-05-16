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
// Created by yxp on 2018/6/12.
//

#include "platform_bridge_in_multi_so.h"
#include "android/base/log_utils.h"
#include "android/jsengine/bridge/script/script_bridge_in_simple.h"
#include "core/config/core_environment.h"
#include "core_side_in_simple.h"
#include "core/manager/weex_core_manager.h"
#include "include/WeexApiHeader.h"
#include "platform_side_in_multi_so.h"

static PlatformExposeFunctions *g_platform_expose_functions = nullptr;

// For so to change function to communicate
extern "C" CoreSideFunctionsOfPlatformBridge *ExchangePlatformBridgeFunctions(
    PlatformExposeFunctions *platform_expose_functions) {
  g_platform_expose_functions = platform_expose_functions;
  return weex::PlatformBridgeInMultiSo::GetExposedFunctions();
}

namespace weex {
PlatformBridgeInMultiSo *PlatformBridgeInMultiSo::g_instance = NULL;

PlatformBridgeInMultiSo::PlatformBridgeInMultiSo() {
  set_core_side(new CoreSideInSimple);
  set_platform_side(new PlatformSideInMultiSo(g_platform_expose_functions));
}

PlatformBridgeInMultiSo::~PlatformBridgeInMultiSo() {}

CoreSideFunctionsOfPlatformBridge *
PlatformBridgeInMultiSo::GetExposedFunctions() {
  CoreSideFunctionsOfPlatformBridge temp = {SetDefaultHeightAndWidthIntoRootDom,
                                            OnInstanceClose,
                                            SetStyleWidth,
                                            SetStyleHeight,
                                            SetMargin,
                                            SetPadding,
                                            SetPosition,
                                            MarkDirty,
                                            SetViewPortWidth,
                                            ForceLayout,
                                            NotifyLayout,
                                            GetFirstScreenRenderTime,
                                            GetRenderFinishTime,
                                            SetRenderContainerWrapContent,
                                            BindMeasurementToRenderObject,
                                            RegisterCoreEnv,
                                            GetRenderObject,
                                            UpdateRenderObjectStyle,
                                            UpdateRenderObjectAttr,
                                            CopyRenderObject,
                                            SetMeasureFunctionAdapter,
                                            SetPlatform,
                                            SetDeviceWidthAndHeight,
                                            AddOption,
                                            InitFramework,
                                            InitAppFramework,
                                            CreateAppContext,
                                            ExecJSOnAppWithResult,
                                            CallJSOnAppContext,
                                            DestroyAppContext,
                                            ExecJsService,
                                            ExecTimeCallback,
                                            ExecJS,
                                            ExecJSWithResult,
                                            CreateInstance,
                                            ExecJSOnInstance,
                                            DestroyInstance,
                                            UpdateGlobalConfig,
                                            UpdateInitFrameworkParams};
  auto functions = (CoreSideFunctionsOfPlatformBridge *)malloc(
      sizeof(CoreSideFunctionsOfPlatformBridge));
  memset(functions, 0, sizeof(CoreSideFunctionsOfPlatformBridge));
  memcpy(functions, &temp, sizeof(CoreSideFunctionsOfPlatformBridge));
  return functions;
}

///////////////////////////

void PlatformBridgeInMultiSo::SetDefaultHeightAndWidthIntoRootDom(
    const std::string &instance_id, float default_width, float default_height,
    bool is_width_wrap_content, bool is_height_wrap_content) {
  g_instance->core_side()->SetDefaultHeightAndWidthIntoRootDom(
      instance_id, default_width, default_height, is_width_wrap_content,
      is_height_wrap_content);
}

void PlatformBridgeInMultiSo::OnInstanceClose(const std::string &instance_id) {
  LOGE("OnInstanceClose OnInstanceClose");
  g_instance->core_side()->OnInstanceClose(instance_id);
}

void PlatformBridgeInMultiSo::SetStyleWidth(const std::string &instance_id,
                                            const std::string &render_ref,
                                            float width) {
  g_instance->core_side()->SetStyleWidth(instance_id, render_ref, width);
}

void PlatformBridgeInMultiSo::SetStyleHeight(const std::string &instance_id,
                                             const std::string &render_ref,
                                             float height) {
  g_instance->core_side()->SetStyleHeight(instance_id, render_ref, height);
}

void PlatformBridgeInMultiSo::SetMargin(const std::string &instance_id,
                                        const std::string &render_ref, int edge,
                                        float value) {
  g_instance->core_side()->SetMargin(instance_id, render_ref, edge, value);
}
void PlatformBridgeInMultiSo::SetPadding(const std::string &instance_id,
                                         const std::string &render_ref,
                                         int edge, float value) {
  g_instance->core_side()->SetPadding(instance_id, render_ref, edge, value);
}

void PlatformBridgeInMultiSo::SetPosition(const std::string &instance_id,
                                          const std::string &render_ref,
                                          int edge, float value) {
  g_instance->core_side()->SetPosition(instance_id, render_ref, edge, value);
}

void PlatformBridgeInMultiSo::MarkDirty(const std::string &instance_id,
                                        const std::string &render_ref) {
  g_instance->core_side()->MarkDirty(instance_id, render_ref);
}

void PlatformBridgeInMultiSo::SetViewPortWidth(const std::string &instance_id,
                                               float width) {
  g_instance->core_side()->SetViewPortWidth(instance_id, width);
}

void PlatformBridgeInMultiSo::ForceLayout(const std::string &instance_id) {
  g_instance->core_side()->ForceLayout(instance_id);
}

bool PlatformBridgeInMultiSo::NotifyLayout(const std::string &instance_id) {
  return g_instance->core_side()->NotifyLayout(instance_id);
}

std::vector<int64_t> PlatformBridgeInMultiSo::GetFirstScreenRenderTime(
    const std::string &instance_id) {
  return g_instance->core_side()->GetFirstScreenRenderTime(instance_id);
}

std::vector<int64_t> PlatformBridgeInMultiSo::GetRenderFinishTime(
    const std::string &instance_id) {
  return g_instance->core_side()->GetRenderFinishTime(instance_id);
}

void PlatformBridgeInMultiSo::SetRenderContainerWrapContent(
    const std::string &instance_id, bool wrap) {
  g_instance->core_side()->SetRenderContainerWrapContent(instance_id, wrap);
}

void PlatformBridgeInMultiSo::BindMeasurementToRenderObject(long ptr) {
  g_instance->core_side()->BindMeasurementToRenderObject(ptr);
}

void PlatformBridgeInMultiSo::RegisterCoreEnv(const std::string &key,
                                              const std::string &value) {
  g_instance->core_side()->RegisterCoreEnv(key, value);
}

long PlatformBridgeInMultiSo::GetRenderObject(const std::string &instance_id,
                                              const std::string &render_ref) {
  return g_instance->core_side()->GetRenderObject(instance_id, render_ref);
}
void PlatformBridgeInMultiSo::UpdateRenderObjectStyle(
    long render_ptr, const std::string &key, const std::string &value) {
  g_instance->core_side()->UpdateRenderObjectStyle(render_ptr, key, value);
}

void PlatformBridgeInMultiSo::UpdateRenderObjectAttr(long render_ptr,
                                                     const std::string &key,
                                                     const std::string &value) {
  g_instance->core_side()->UpdateRenderObjectAttr(render_ptr, key, value);
}

long PlatformBridgeInMultiSo::CopyRenderObject(long render_ptr) {
  return g_instance->core_side()->CopyRenderObject(render_ptr);
}

void PlatformBridgeInMultiSo::SetMeasureFunctionAdapter() {
  WeexCore::WeexCoreManager::Instance()->set_platform_bridge(
      PlatformBridgeInMultiSo::Instance());
  g_instance->core_side()->SetMeasureFunctionAdapter();
}

void PlatformBridgeInMultiSo::SetPlatform(const std::string &platformName) {
  WXCoreEnvironment::getInstance()->SetPlatform(platformName);
}

void PlatformBridgeInMultiSo::SetDeviceWidthAndHeight(float width,
                                                      float height) {
  WXCoreEnvironment::getInstance()->set_device_width(width);
  WXCoreEnvironment::getInstance()->set_device_height(height);
}

void PlatformBridgeInMultiSo::AddOption(const std::string &key,
                                        const std::string &value) {
  WXCoreEnvironment::getInstance()->AddOption(key, value);
}

int PlatformBridgeInMultiSo::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return Instance()->core_side()->InitFramework(script, params);
}

int PlatformBridgeInMultiSo::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return Instance()->core_side()->InitAppFramework(instanceId, appFramework,
                                                   params);
}

int PlatformBridgeInMultiSo::CreateAppContext(const char *instanceId,
                                              const char *jsBundle) {
  return Instance()->core_side()->CreateAppContext(instanceId, jsBundle);
}

std::unique_ptr<WeexJSResult> PlatformBridgeInMultiSo::ExecJSOnAppWithResult(const char *instanceId,
                                                     const char *jsBundle) {
  return const_cast<char *>(
      Instance()->core_side()->ExecJSOnAppWithResult(instanceId, jsBundle));
}

int PlatformBridgeInMultiSo::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return Instance()->core_side()->CallJSOnAppContext(instanceId, func, params);
}

int PlatformBridgeInMultiSo::DestroyAppContext(const char *instanceId) {
  return Instance()->core_side()->DestroyAppContext(instanceId);
}

int PlatformBridgeInMultiSo::ExecJsService(const char *source) {
  return Instance()->core_side()->ExecJsService(source);
}

int PlatformBridgeInMultiSo::ExecTimeCallback(const char *source) {
  return Instance()->core_side()->ExecTimeCallback(source);
}

int PlatformBridgeInMultiSo::ExecJS(const char *instanceId,
                                    const char *nameSpace, const char *func,
                                    std::vector<VALUE_WITH_TYPE *> &params) {
  return Instance()->core_side()->ExecJS(instanceId, nameSpace, func, params);
}

std::unique_ptr<WeexJSResult>  PlatformBridgeInMultiSo::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return Instance()->core_side()->ExecJSWithResult(instanceId, nameSpace, func,
                                                   params);
}

int PlatformBridgeInMultiSo::CreateInstance(
    const char *instanceId, const char *func, const char *script,
    const char *opts, const char *initData, const char *extendsApi,
    std::vector<INIT_FRAMEWORK_PARAMS*>& params) {
  return Instance()->core_side()->CreateInstance(instanceId, func, script, opts,
                                                 initData, extendsApi,params);
}

std::unique_ptr<WeexJSResult> PlatformBridgeInMultiSo::ExecJSOnInstance(const char *instanceId,
                                                const char *script) {
  return const_cast<char *>(
      Instance()->core_side()->ExecJSOnInstance(instanceId, script));
}

int PlatformBridgeInMultiSo::DestroyInstance(const char *instanceId) {
  return Instance()->core_side()->DestroyInstance(instanceId);
}

int PlatformBridgeInMultiSo::UpdateGlobalConfig(const char *config) {
  return Instance()->core_side()->UpdateGlobalConfig(config);
}

int PlatformBridgeInMultiSo::UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc){
    return Instance()->core_side()->UpdateInitFrameworkParams(key, value, desc);
}


}  // namespace weex
