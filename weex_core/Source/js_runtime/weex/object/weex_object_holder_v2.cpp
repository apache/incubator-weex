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

#include "weex_object_holder_v2.h"
#include "js_runtime/utils/log_utils.h"

WeexObjectHolderV2::WeexObjectHolderV2(unicorn::RuntimeVM *vm, TimerQueue *timeQueue,
                                       bool isMultiProgress) {
    this->vm = vm;
    this->timeQueue = timeQueue;
    this->isMultiProgress = isMultiProgress;
}

void WeexObjectHolderV2::initFromParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool forAppContext) {
    if (forAppContext) {
        LOG_RUNTIME("Create MiniApp worker Context");
        globalObject = std::unique_ptr<WeexGlobalObjectV2>(this->createAppWorkerObject());
    } else {
        LOG_RUNTIME("Create global Context");
        globalObject = std::unique_ptr<WeexGlobalObjectV2>(this->createWeexObject());
    }
    globalObject->timeQueue = this->timeQueue;
    globalObject->initWxEnvironment(params, true);
}

WeexGlobalObjectV2 *WeexObjectHolderV2::createWeexObject() {
    auto weex_object = new WeexGlobalObjectV2();
    weex_object->makeWeexGlobalObject(this->vm);
    return weex_object;
}

WeexGlobalObjectV2 *WeexObjectHolderV2::createInstancecObject(const std::string &id, const std::string &name) {
    auto weex_object = new WeexGlobalObjectV2();
    weex_object->makeWeexInstanceObject(this->vm, id, name);
    weex_object->initWxEnvironment(globalObject.get()->m_initFrameworkParams, false);
    return weex_object;
}

WeexGlobalObjectV2 *WeexObjectHolderV2::createAppWorkerObject() {
    auto app_work_object = new WeexGlobalObjectV2();
    app_work_object->makeAppWorkerObject(this->vm);
    return app_work_object;
}

