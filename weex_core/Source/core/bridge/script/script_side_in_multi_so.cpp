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

#include <android/base/log_utils.h>
#include "core/bridge/script/script_side_in_multi_so.h"

namespace WeexCore {
namespace bridge {
namespace script {

ScriptSideInMultiSo::ScriptSideInMultiSo() {}

ScriptSideInMultiSo::~ScriptSideInMultiSo() {}

int ScriptSideInMultiSo::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    LOGE("ScriptSideInMultiSo::InitFramework");
  return script_side_functions_->funcInitFramework(script, params);
}

int ScriptSideInMultiSo::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    LOGE("ScriptSideInMultiSo::InitAppFramework");
  return script_side_functions_->funcInitAppFramework(instanceId, appFramework,
                                                      params);
}

int ScriptSideInMultiSo::CreateAppContext(const char *instanceId,
                                          const char *jsBundle) {
  LOGE("ScriptSideInMultiSo::CreateAppContext");
  return script_side_functions_->funcCreateAppContext(instanceId, jsBundle);
}

char *ScriptSideInMultiSo::ExecJSOnAppWithResult(const char *instanceId,
                                                 const char *jsBundle) {
  LOGE("ScriptSideInMultiSo::ExecJSOnAppWithResult");
  return script_side_functions_->funcExeJSOnAppWithResult(instanceId, jsBundle);
}

int ScriptSideInMultiSo::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
    LOGE("ScriptSideInMultiSo::CallJSOnAppContext");
  return script_side_functions_->funcCallJSOnAppContext(instanceId, func,
                                                        params);
}

int ScriptSideInMultiSo::DestroyAppContext(const char *instanceId) {
  LOGE("ScriptSideInMultiSo::DestroyAppContext");
  return script_side_functions_->funcDestroyAppContext(instanceId);
}

int ScriptSideInMultiSo::ExecJsService(const char *source) {
  LOGE("ScriptSideInMultiSo::ExecJsService");
  return script_side_functions_->funcExeJsService(source);
}

int ScriptSideInMultiSo::ExecTimeCallback(const char *source) {
  LOGE("ScriptSideInMultiSo::ExecTimeCallback");
  return script_side_functions_->funcExeCTimeCallback(source);
}

int ScriptSideInMultiSo::ExecJS(const char *instanceId, const char *nameSpace,
                                const char *func,
                                std::vector<VALUE_WITH_TYPE *> &params) {
    LOGE("ScriptSideInMultiSo::ExecJS");
  return script_side_functions_->funcExeJS(instanceId, nameSpace, func, params);
}

WeexJSResult ScriptSideInMultiSo::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
    LOGE("ScriptSideInMultiSo::ExecJSWithResult");
  return script_side_functions_->funcExeJSWithResult(instanceId, nameSpace,
                                                     func, params);
}

int ScriptSideInMultiSo::CreateInstance(const char *instanceId,
                                        const char *func, const char *script,
                                        const char *opts, const char *initData,
                                        const char *extendsApi) {
  LOGE("ScriptSideInMultiSo::CreateInstance");
  return script_side_functions_->funcCreateInstance(instanceId, func, script,
                                                    opts, initData, extendsApi);
}

char *ScriptSideInMultiSo::ExecJSOnInstance(const char *instanceId,
                                            const char *script) {
  LOGE("ScriptSideInMultiSo::ExecJSOnInstance");
  return script_side_functions_->funcExeJSOnInstance(instanceId, script);
}

int ScriptSideInMultiSo::DestroyInstance(const char *instanceId) {
  LOGE("ScriptSideInMultiSo::DestroyInstance");
  return script_side_functions_->funcDestroyInstance(instanceId);
}

int ScriptSideInMultiSo::UpdateGlobalConfig(const char *config) {
  LOGE("ScriptSideInMultiSo::UpdateGlobalConfig");
  return script_side_functions_->funcUpdateGlobalConfig(config);
}
}  // namespace script
}  // namespace bridge
}  // namespace WeexCore