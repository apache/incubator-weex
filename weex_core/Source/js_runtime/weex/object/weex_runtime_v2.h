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
// Created by chenpeihan on 2019/2/22.
//

#ifndef PROJECT_WEEX_RUNTIME_V2_H
#define PROJECT_WEEX_RUNTIME_V2_H

#include <core/bridge/script_bridge.h>
#include "weex_object_holder_v2.h"
#include "weex_runtime.h"


class WeexRuntimeV2 : public WeexRuntime {

public:

    explicit WeexRuntimeV2(TimerQueue *timeQueue, WeexCore::ScriptBridge *script_bridge, bool isMultiProgress);

    bool hasInstanceId(std::string &id) override;


    int initFramework(const std::string &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    int
    initAppFramework(const std::string &instanceId, const std::string &appFramework,
                     std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    int createAppContext(const std::string &instanceId, const std::string &jsBundle) override;

    std::unique_ptr<WeexJSResult> exeJSOnAppWithResult(const std::string &instanceId, const std::string &jsBundle) override;


    int
    callJSOnAppContext(const std::string &instanceId, const std::string &func, std::vector<VALUE_WITH_TYPE *> &params) override;

    int destroyAppContext(const std::string &instanceId) override;

    int exeJsService(const std::string &source) override;

    int exeCTimeCallback(const std::string &source) override;


    int
    exeJS(const std::string &instanceId, const std::string &nameSpace, const std::string &func,
          std::vector<VALUE_WITH_TYPE *> &params) override;


    std::unique_ptr<WeexJSResult> exeJSWithResult(const std::string &instanceId, const std::string &nameSpace, const std::string &func,
                                                  std::vector<VALUE_WITH_TYPE *> &params) override;

    void exeJSWithCallback(const std::string &instanceId, const std::string &nameSpace, const std::string &func,
                           std::vector<VALUE_WITH_TYPE *> &params, long callback_id) override;

    int createInstance(const std::string &instanceId, const std::string &func, const std::string &script, const std::string &opts,
                       const std::string &initData, const std::string &extendsApi,
                       std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

    std::unique_ptr<WeexJSResult> exeJSOnInstance(const std::string &instanceId, const std::string &script) override;

    int destroyInstance(const std::string &instanceId) override;

    int updateGlobalConfig(const std::string &config) override;

    int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc);

    WeexObjectHolderV2 *getLightAppObjectHolderV2(const std::string &instanceId);

    int exeTimerFunctionForRunTimeApi(const std::string &instanceId, uint32_t timerFunction, bool is_from_instance) override;

    void removeTimerFunctionForRunTimeApi(const std::string &instanceId,const uint32_t timerFunction, bool is_from_instance) override;


protected:
    int _initFrameworkWithScript(const std::string &source);

    int _initAppFrameworkWithScript(const std::string &instanceId, const std::string &appFramework);

    void _geJSRuntimeArgsFromWeexParams(unicorn::EngineContext *context, std::vector<unicorn::ScopeValues> *obj,
                                        std::vector<VALUE_WITH_TYPE *> &params);

    WeexGlobalObjectV2* findWeexObj(const std::string &instanceId, bool is_instance);


protected:
    std::unique_ptr<WeexObjectHolderV2> weex_object_holder_v2_;
    std::map<std::string, WeexObjectHolderV2 *> app_worker_context_holder_map_v2_;
    unicorn::RuntimeVM *vm_;
    bool multi_process_flag_;
};


#endif //PROJECT_WEEX_RUNTIME_V2_H
