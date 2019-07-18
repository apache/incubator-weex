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
// Created by Darin on 2019/3/25.
//
#ifndef WEEXV8_WEEX_RUNTIME_VIR
#define WEEXV8_WEEX_RUNTIME_VIR
#include "core/bridge/script_bridge.h"
#include "js_runtime/weex/object/weex_object_holder_v2.h"

class WeexRuntime {
 public:

  explicit WeexRuntime(WeexCore::ScriptBridge* scriptBridge) { this->script_bridge_ = scriptBridge;}

  WeexCore::ScriptBridge* script_bridge_;

  virtual bool hasInstanceId(std::string &id) = 0;


  virtual int initFramework(const std::string &script,
                            std::vector<INIT_FRAMEWORK_PARAMS *> &params) = 0;


  virtual int
  initAppFramework(const std::string &instanceId, const std::string &appFramework,
                   std::vector<INIT_FRAMEWORK_PARAMS *> &params) = 0;

  virtual int createAppContext(const std::string &instanceId, const std::string &jsBundle) = 0;

  virtual std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const std::string &instanceId,
                                                             const std::string &jsBundle) = 0;

  virtual int
  callJSOnAppContext(const std::string &instanceId,
                     const std::string &func,
                     std::vector<VALUE_WITH_TYPE *> &params) = 0;

  virtual int destroyAppContext(const std::string &instanceId) = 0;

  virtual int exeJsService(const std::string &source) = 0;

  virtual int exeCTimeCallback(const std::string &source) = 0;


  virtual int
  exeJS(const std::string &instanceId, const std::string &nameSpace, const std::string &func,
        std::vector<VALUE_WITH_TYPE *> &params) = 0;


  virtual std::unique_ptr<WeexJSResult> exeJSWithResult(const std::string &instanceId,
                                                        const std::string &nameSpace,
                                                        const std::string &func,
                                                        std::vector<VALUE_WITH_TYPE *> &params) = 0;

  virtual void exeJSWithCallback(const std::string &instanceId,
                                 const std::string &nameSpace,
                                 const std::string &func,
                                 std::vector<VALUE_WITH_TYPE *> &params,
                                 long callback_id) = 0;

  virtual int createInstance(const std::string &instanceId,
                             const std::string &func,
                             const std::string &script,
                             const std::string &opts,
                             const std::string &initData,
                             const std::string &extendsApi,
                             std::vector<INIT_FRAMEWORK_PARAMS *> &params) = 0;

  virtual std::unique_ptr<WeexJSResult> exeJSOnInstance(const std::string &instanceId,
                                                        const std::string &script) = 0;

  virtual int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc)=0;

  virtual int destroyInstance(const std::string &instanceId) = 0;


  virtual int updateGlobalConfig(const std::string &config) = 0;

  virtual int exeTimerFunctionForRunTimeApi(const std::string &instanceId,
                                            uint32_t timerFunction,
                                            bool is_from_instance) = 0;

  virtual void removeTimerFunctionForRunTimeApi(const std::string &instanceId,
                                                const uint32_t timerFunction,
                                                bool is_from_instance) = 0;
};

#endif //WEEXV8_WEEX_RUNTIME_VIR