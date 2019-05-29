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

#include "script_side_in_simple.h"
#include "android/jsengine/weex_runtime.h"

namespace weex {
namespace bridge {
namespace js {
int ScriptSideInSimple::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  LOGD("ScriptSideInSimple::InitFramework");

  return runtime_->initFramework(String::fromUTF8(script), params);
}

int ScriptSideInSimple::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  LOGD("ScriptSideInSimple::InitAppFramework");
  return runtime_->initAppFramework(String::fromUTF8(instanceId),
                                    String::fromUTF8(appFramework), params);
}

int ScriptSideInSimple::CreateAppContext(const char *instanceId,
                                         const char *jsBundle) {
  LOGD("ScriptSideInSimple::CreateAppContext");
  return runtime_->createAppContext(String::fromUTF8(instanceId),
                                    String::fromUTF8(jsBundle));
}

std::unique_ptr<WeexJSResult> ScriptSideInSimple::ExecJSOnAppWithResult(const char *instanceId,
                                                const char *jsBundle) {
  LOGD("ScriptSideInSimple::ExecJSOnAppWithResult");
  return runtime_->exeJSOnAppWithResult(String::fromUTF8(instanceId),
                                        String::fromUTF8(jsBundle));
}

int ScriptSideInSimple::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  LOGD("ScriptSideInSimple::CallJSOnAppContext");
  return runtime_->callJSOnAppContext(String::fromUTF8(instanceId),
                                      String::fromUTF8(func), params);
}

int ScriptSideInSimple::DestroyAppContext(const char *instanceId) {
  LOGD("ScriptSideInSimple::DestroyAppContext");
  return runtime_->destroyAppContext(String::fromUTF8(instanceId));
}

int ScriptSideInSimple::ExecJsService(const char *source) {
  LOGD("ScriptSideInSimple::ExecJsService");
  return runtime_->exeJsService(String::fromUTF8(source));
}

int ScriptSideInSimple::ExecTimeCallback(const char *source) {
  LOGD("ScriptSideInSimple::ExecTimeCallback");
  return runtime_->exeCTimeCallback(String::fromUTF8(source));
}

int ScriptSideInSimple::ExecJS(const char *instanceId, const char *nameSpace,
                               const char *func,
                               std::vector<VALUE_WITH_TYPE *> &params) {
  LOGD("ScriptSideInSimple::ExecJS");
  return runtime_->exeJS(String::fromUTF8(instanceId),
                         String::fromUTF8(nameSpace), String::fromUTF8(func),
                         params);
}

std::unique_ptr<WeexJSResult>  ScriptSideInSimple::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  LOGD("ScriptSideInSimple::ExecJSWithResult");
  return runtime_->exeJSWithResult(String::fromUTF8(instanceId),
                                   String::fromUTF8(nameSpace),
                                   String::fromUTF8(func), params);
}

void ScriptSideInSimple::ExecJSWithCallback(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params, long callback_id) {
  LOGD("ScriptSideInSimple::ExecJSWithResult");
  runtime_->exeJSWithResult(String::fromUTF8(instanceId),
                                   String::fromUTF8(nameSpace),
                                   String::fromUTF8(func), params);
}

int ScriptSideInSimple::CreateInstance(const char *instanceId, const char *func,
                                       const char *script, const char *opts,
                                       const char *initData,
                                       const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params) {
  LOGD(
      "CreateInstance id = %s, func = %s, script = %s, opts = %s, initData = "
      "%s, extendsApi = %s",
      instanceId, func, script, opts, initData, extendsApi);

  return runtime_->createInstance(
      String::fromUTF8(instanceId), String::fromUTF8(func),
      String::fromUTF8(script), String::fromUTF8(opts),
      String::fromUTF8(initData), String::fromUTF8(extendsApi),params);
}

std::unique_ptr<WeexJSResult> ScriptSideInSimple::ExecJSOnInstance(const char *instanceId,
                                           const char *script,int type) {
  LOGD("ScriptSideInSimple::ExecJSOnInstance");
  return runtime_->exeJSOnInstance(String::fromUTF8(instanceId),
                                   String::fromUTF8(script));
}

int ScriptSideInSimple::DestroyInstance(const char *instanceId) {
  LOGD("ScriptSideInSimple::DestroyInstance");
  return runtime_->destroyInstance(String::fromUTF8(instanceId));
}

int ScriptSideInSimple::UpdateGlobalConfig(const char *config) {
  LOGD("ScriptSideInSimple::UpdateGlobalConfig");
  return runtime_->updateGlobalConfig(String::fromUTF8(config));
}


int ScriptSideInSimple::UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc){
  LOGD("ScriptSideInSimple::UpdateInitFrameworkParams");
  return runtime_->UpdateInitFrameworkParams(key ,value, desc);
}



}  // namespace js
}  // namespace bridge
}  // namespace weex
