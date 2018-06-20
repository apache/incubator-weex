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

#ifdef __cplusplus

#ifndef WEEXCORE_WEEX_INSTANCE_MANAGER_H
#define WEEXCORE_WEEX_INSTANCE_MANAGER_H


#include <core/js_engin/base_js_runtime.h>
#include <core/js_engin/base_js_context.h>


namespace WeexCore {
    class WXInstanceManager {

    protected:
        BaseJSRunTime *jsRunTime = nullptr;
        BaseJSContext *defaultContext = nullptr;
        std::map<uint32_t, BaseJSContext *> instanceMap;

    public:

        WXInstanceManager() {
        }

        ~WXInstanceManager() {
            if (nullptr != jsRunTime) {
                delete jsRunTime;
                jsRunTime = nullptr;
            }
            if (nullptr != defaultContext) {
               // delete defaultContext;
                defaultContext = nullptr;
            }
            instanceMap.clear();
        }

        void doInit();


    public:
        int createInstance();

        int destroyInstance(uint32_t instanceId);

    };
}

#endif //WEEXCORE_WEEX_INSTANCE_MANAGER_H

#endif
