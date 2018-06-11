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
#include "base_js_runtime.h"
#include <core/manager/weex_core_manager.h>

namespace WeexCore {
    static uint32_t baseRunTimeId = 0;
    uint32_t baseContextId = 0;

    bool BaseJSRunTime::initRunTime() {
        baseRunTimeId++;
        this->runTimeId = baseRunTimeId;
        bool success = WeexCoreManager::getInstance()->getJSBridge()->createJSRunTime(this->runTimeId, nullptr);
        return success;
    }

    void BaseJSRunTime::destroy() {
        for (auto &iter : contextMap) {
            destroyContext(iter.second);
        }
        contextMap.clear();
    }

    BaseJSContext *BaseJSRunTime::createContext() {
        BaseJSContext *context = new BaseJSContext();
        baseContextId++;
        context->contextId = baseContextId;
        context->jsRunTime = this;
        context->onInit();
        contextMap.insert(std::pair<uint32_t, BaseJSContext *>(context->contextId, context));
        return context;

    }

    void BaseJSRunTime::destroyContext(BaseJSContext *context) {
        context->onDestroy();
        context->jsRunTime = nullptr;
    }

}
