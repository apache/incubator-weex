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

#include "script_side_in_multi_so.h"
#include "base/android/log_utils.h"

namespace WeexCore {
namespace bridge {
namespace script {

ScriptSideInMultiSo::ScriptSideInMultiSo():script_side_functions_(nullptr) {}

ScriptSideInMultiSo::~ScriptSideInMultiSo() {}

int ScriptSideInMultiSo::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::InitFramework script_side_functions_ is null");
    return false;
  }

  return script_side_functions_->funcInitFramework(script, params);
}

int ScriptSideInMultiSo::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::InitAppFramework script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcInitAppFramework(instanceId, appFramework,
                                                      params);
}

int ScriptSideInMultiSo::CreateAppContext(const char *instanceId,
                                          const char *jsBundle) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::CreateAppContext script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcCreateAppContext(instanceId, jsBundle);
}

std::unique_ptr<WeexJSResult> ScriptSideInMultiSo::ExecJSOnAppWithResult(const char *instanceId,
                                                 const char *jsBundle) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::ExecJSOnAppWithResult script_side_functions_ is null");
    std::unique_ptr<WeexJSResult> result;
    return result;
  }
  return script_side_functions_->funcExeJSOnAppWithResult(instanceId, jsBundle);
}

int ScriptSideInMultiSo::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::CallJSOnAppContext script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcCallJSOnAppContext(instanceId, func,
                                                        params);
}

int ScriptSideInMultiSo::DestroyAppContext(const char *instanceId) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::DestroyAppContext script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcDestroyAppContext(instanceId);
}

int ScriptSideInMultiSo::ExecJsService(const char *source) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::ExecJsService script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcExeJsService(source);
}

int ScriptSideInMultiSo::ExecTimeCallback(const char *source) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::ExecTimeCallback script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcExeCTimeCallback(source);
}

int ScriptSideInMultiSo::ExecJS(const char *instanceId, const char *nameSpace,
                                const char *func,
                                std::vector<VALUE_WITH_TYPE *> &params) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::ExecJS script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcExeJS(instanceId, nameSpace, func, params);
}

std::unique_ptr<WeexJSResult> ScriptSideInMultiSo::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::ExecJSWithResult script_side_functions_ is null");
    std::unique_ptr<WeexJSResult> result;
    return  result;
  }
  return script_side_functions_->funcExeJSWithResult(instanceId, nameSpace,
                                                     func, params);
}

void ScriptSideInMultiSo::ExecJSWithCallback(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params, long callback_id) {
  if (script_side_functions_ == nullptr) {
    LOGE(
        "ScriptSideInMultiSo::ExecJSWithCallback script_side_functions_ is null");
    return;
  }
  script_side_functions_->funcExeJSWithResultId(instanceId, nameSpace, func,
                                                params, callback_id);
}

int ScriptSideInMultiSo::CreateInstance(const char *instanceId,
                                        const char *func, const char *script,
                                        const char *opts, const char *initData,
                                        const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::CreateInstance script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcCreateInstance(instanceId, func, script,
                                                    opts, initData, extendsApi, params);
}

std::unique_ptr<WeexJSResult> ScriptSideInMultiSo::ExecJSOnInstance(const char *instanceId,
                                            const char *script,int type) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::ExecJSOnInstance script_side_functions_ is null");
    std::unique_ptr<WeexJSResult> result;
    return  result;
  }
  return script_side_functions_->funcExeJSOnInstance(instanceId, script,type);
}

int ScriptSideInMultiSo::DestroyInstance(const char *instanceId) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::DestroyInstance script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcDestroyInstance(instanceId);
}

int ScriptSideInMultiSo::UpdateGlobalConfig(const char *config) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::UpdateGlobalConfig script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcUpdateGlobalConfig(config);
}

int ScriptSideInMultiSo::UpdateInitFrameworkParams(const std::string &key, const std::string &value,
                                                   const std::string &desc) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::UpdateInitFrameworkParams script_side_functions_ is null");
    return false;
  }
  return script_side_functions_->funcUpdateInitFrameworkParams(key, value, desc);
}

void ScriptSideInMultiSo::SetLogType(const int logLevel, const bool isPerf) {
  if(script_side_functions_ == nullptr) {
    LOGE("ScriptSideInMultiSo::UpdateInitFrameworkParams script_side_functions_ is null");
    return;
  }
  return script_side_functions_->funcSetLogType(logLevel, isPerf);

}

}  // namespace script
}  // namespace bridge
}  // namespace WeexCore
