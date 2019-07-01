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

#ifndef PROJECT_WEEX_GLOBAL_OBJECT_V2_H
#define PROJECT_WEEX_GLOBAL_OBJECT_V2_H


#include "core/bridge/script_bridge.h"
#include "include/WeexApiHeader.h"
//#include "task/timer_queue.h"
#include "js_runtime/weex/task/timer_queue.h"
#include "js_runtime/runtime/runtime_context.h"
#include "js_runtime/runtime/runtime_object.h"


enum WeexGlobalObjectType {
    WeexInstance,
    WeexGlobal,
    AppWorker
};


class WeexGlobalObjectV2 {
public:
    std::vector<INIT_FRAMEWORK_PARAMS *> m_initFrameworkParams;
    std::string id = "";
    TimerQueue *timeQueue = nullptr;
    std::unique_ptr<unicorn::RuntimeContext> context;
    std::unique_ptr<unicorn::RuntimeObject> global_object_binding;

private:
    WeexCore::ScriptBridge *script_bridge_;
    WeexGlobalObjectType object_type_;
    uint32_t function_id_;
    std::map<uint32_t, unicorn::RuntimeValues *> function_maps_;


public:

    void makeWeexInstanceObject(unicorn::RuntimeVM *vm, const std::string &id, const std::string &name);

    void makeWeexGlobalObject(unicorn::RuntimeVM *vm);

    void makeAppWorkerObject(unicorn::RuntimeVM *vm);


    void addExtraOptions(std::vector<INIT_FRAMEWORK_PARAMS *> &params);

    void initWxEnvironment(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool isSave);

    inline WeexCore::ScriptBridge *js_bridge() { return script_bridge_; }

    void setScriptBridge(WeexCore::ScriptBridge *script_bridge);


    unicorn::RuntimeValues *removeTimer(uint32_t function_id);


    int setNativeTimeout(unicorn::RuntimeValues *func, int timeOut, bool interval);
    void clearNativeTimeout(int timer_task_id);

    unicorn::RuntimeValues *getTimerFunction(uint32_t function_id);

    void updateInitFrameworkParams(const std::string& key, const std::string& value);

    inline WeexGlobalObjectType getObjectType() {
        return this->object_type_;
    }

private:
    // uint32_t genFunctionID();

    //void addTimer(uint32_t function_id, unicorn::RuntimeValues *func);
};


#endif //PROJECT_WEEX_GLOBAL_OBJECT_V2_H
