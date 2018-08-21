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

#ifndef WEEXCORE_WEEX_CORE_MANAGER_H
#define WEEXCORE_WEEX_CORE_MANAGER_H

#include <core/bridge/bridge.h>
#include <core/bridge/js_bridge.h>
#include <core/layout/measure_func_adapter.h>

namespace WeexCore {
    class WeexCoreManager {
    private:
        static WeexCoreManager *m_pInstance;
        Bridge *platformBridge = nullptr;
        MeasureFunctionAdapter *measureFunctionAdapter = nullptr;
        JSBridge* jsBridge = nullptr;

    private:
        WeexCoreManager() {};

        ~WeexCoreManager() {
            delete platformBridge;
            platformBridge = nullptr;
            delete jsBridge;
            jsBridge = nullptr;
        };

        //just to release singleton object
        class Garbo {
        public:
            ~Garbo() {
                if (WeexCoreManager::m_pInstance) {
                    delete WeexCoreManager::m_pInstance;
                }
            }
        };

        static Garbo garbo;

    public:
        static WeexCoreManager *getInstance() {
            if (nullptr == m_pInstance) {
                m_pInstance = new WeexCoreManager();
            }
            return m_pInstance;
        };

        Bridge *getPlatformBridge();

        void setPlatformBridge(Bridge *pBridge);

        JSBridge* getJSBridge();

        void setJSBridge(JSBridge *jsBridge);

        void SetMeasureFunctionAdapter(MeasureFunctionAdapter *measureFunctionAdapter);

        MeasureFunctionAdapter *GetMeasureFunctionAdapter();

    };
}

#endif //WEEXCORE_WEEX_CORE_MANAGER_H