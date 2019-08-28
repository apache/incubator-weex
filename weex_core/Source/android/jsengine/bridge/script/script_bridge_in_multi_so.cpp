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

#include "android/jsengine/bridge/script/script_bridge_in_multi_so.h"

#include "android/jsengine/bridge/platform/platform_bridge_in_multi_so.h"
#include "android/jsengine/bridge/script/core_side_in_multi_process.h"
#include "android/jsengine/bridge/script/core_side_in_multi_so.h"
#include "android/jsengine/bridge/script/core_side_in_simple.h"
#include "android/jsengine/bridge/script/script_side_in_simple.h"
#include "android/jsengine/bridge/script/script_side_in_queue.h"
#include "android/jsengine/object/weex_env.h"
#include "android/jsengine/weex_runtime.h"
#include "core/manager/weex_core_manager.h"

static FunctionsExposedByCore *g_functions_exposed_by_core = nullptr;

extern "C" FunctionsExposedByJS *ExchangeJSBridgeFunctions(
    FunctionsExposedByCore *functions) {
  g_functions_exposed_by_core = functions;
  return weex::bridge::js::ScriptBridgeInMultiSo::GetExposedFunctions();
}

namespace weex {
namespace bridge {
namespace js {
ScriptBridgeInMultiSo *ScriptBridgeInMultiSo::g_instance = NULL;

ScriptBridgeInMultiSo::ScriptBridgeInMultiSo() {
  set_script_side(new ScriptSideInQueue());
//  set_core_side(new CoreSideInSimple());
    set_core_side(new CoreSideInMultiSo(g_functions_exposed_by_core));
  //  set_core_side(new MultiProcessCoreSide());
}

ScriptBridgeInMultiSo::~ScriptBridgeInMultiSo() {}

FunctionsExposedByJS *ScriptBridgeInMultiSo::GetExposedFunctions() {
  FunctionsExposedByJS temp = {
      InitFramework,         InitAppFramework,   CreateAppContext,
      ExecJSOnAppWithResult, CallJSOnAppContext, DestroyAppContext,
      ExecJSService,         ExecTimeCallback,   ExecJS,
      ExecJSWithResult,      ExecJSWithCallback, CreateInstance,     ExecJSOnInstance,
      DestroyInstance,       UpdateGlobalConfig,  UpdateInitFrameworkParams, SetLogType};
  auto functions = (FunctionsExposedByJS *)malloc(sizeof(FunctionsExposedByJS));
  memset(functions, 0, sizeof(FunctionsExposedByJS));
  memcpy(functions, &temp, sizeof(FunctionsExposedByJS));
  return functions;
}

int ScriptBridgeInMultiSo::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  static_cast<ScriptSideInQueue *>(Instance()->script_side())
          ->setTaskQueue(new WeexTaskQueue(false));

  WeexEnv::getEnv()->setScriptBridge(Instance());

  return Instance()->script_side()->InitFramework(script, params);
}

int ScriptBridgeInMultiSo::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return Instance()->script_side()->InitAppFramework(instanceId, appFramework,
                                                 params);
}

int ScriptBridgeInMultiSo::CreateAppContext(const char *instanceId,
                                            const char *jsBundle) {
  return Instance()->script_side()->CreateAppContext(instanceId, jsBundle);
}

std::unique_ptr<WeexJSResult> ScriptBridgeInMultiSo::ExecJSOnAppWithResult(const char *instanceId,
                                                   const char *jsBundle) {
  return Instance()->script_side()->ExecJSOnAppWithResult(instanceId, jsBundle);
}

int ScriptBridgeInMultiSo::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return Instance()->script_side()->CallJSOnAppContext(instanceId, func, params);
}

int ScriptBridgeInMultiSo::DestroyAppContext(const char *instanceId) {
  return Instance()->script_side()->DestroyAppContext(instanceId);
}

int ScriptBridgeInMultiSo::ExecJSService(const char *source) {
  return Instance()->script_side()->ExecJsService(source);
}

int ScriptBridgeInMultiSo::ExecTimeCallback(const char *source) {
  return Instance()->script_side()->ExecTimeCallback(source);
}

int ScriptBridgeInMultiSo::ExecJS(const char *instanceId, const char *nameSpace,
                                  const char *func,
                                  std::vector<VALUE_WITH_TYPE *> &params) {
  return Instance()->script_side()->ExecJS(instanceId, nameSpace, func, params);
}

std::unique_ptr<WeexJSResult>  ScriptBridgeInMultiSo::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return Instance()->script_side()->ExecJSWithResult(instanceId, nameSpace, func,
                                                 params);
}

void ScriptBridgeInMultiSo::ExecJSWithCallback(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params, long callback_id) {
  Instance()->script_side()->ExecJSWithCallback(instanceId, nameSpace, func, params, callback_id);
}

int ScriptBridgeInMultiSo::CreateInstance(const char *instanceId,
                                          const char *func, const char *script,
                                          const char *opts,
                                          const char *initData,
                                          const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params) {
  return Instance()->script_side()->CreateInstance(instanceId, func, script, opts,
                                               initData, extendsApi, params);
}

std::unique_ptr<WeexJSResult> ScriptBridgeInMultiSo::ExecJSOnInstance(const char *instanceId,
                                              const char *script,int type) {
  return Instance()->script_side()->ExecJSOnInstance(instanceId, script,type);
}

int ScriptBridgeInMultiSo::DestroyInstance(const char *instanceId) {
  return Instance()->script_side()->DestroyInstance(instanceId);
}

int ScriptBridgeInMultiSo::UpdateGlobalConfig(const char *config) {
  return Instance()->script_side()->UpdateGlobalConfig(config);
}

int ScriptBridgeInMultiSo::UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc){
    return Instance()->script_side()->UpdateInitFrameworkParams(key, value, desc);
}

void ScriptBridgeInMultiSo::SetLogType(const int logLevel, const bool isPerf) {
  return Instance()->script_side()->SetLogType(logLevel, isPerf);
}


}  // namespace js
}  // namespace bridge
}  // namespace weex
