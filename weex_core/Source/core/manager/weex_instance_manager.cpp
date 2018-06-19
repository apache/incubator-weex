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

#include "weex_instance_manager.h"

#ifdef iOS
#include <iOS/bridge/jsc_runtime_ios.h>
#include <iOS/bridge/jsc_context_ios.h>
#endif


namespace WeexCore {

    void WXInstanceManager::doInit() {
#ifdef iOS
        this->jsRunTime = new JSCRunTimeIOS();
#endif
        this->jsRunTime->initRunTime();
        this->defaultContext = this->jsRunTime->createContext();
    }

    int WXInstanceManager::createInstance() {
        BaseJSContext *instanceContext = this->jsRunTime->createContext();
        this->instanceMap.insert(
                std::pair<uint32_t, BaseJSContext *>(instanceContext->getContextId(), instanceContext)
        );
    }

    int WXInstanceManager::destroyInstance(uint32_t instanceId) {

    }
}

