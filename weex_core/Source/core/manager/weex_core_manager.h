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

#include "base/thread/thread.h"
#include "core/bridge/platform_bridge.h"
#include "core/bridge/script_bridge.h"

namespace WeexCore {
    class WeexCoreManager {

    public:
        enum ProjectMode {
            MULTI_SO,
            MULTI_PROCESS,
            COMMON
        };

        static WeexCoreManager *getInstance() {
            if (nullptr == m_pInstance) {
                m_pInstance = new WeexCoreManager();
            }
            return m_pInstance;
        };

        PlatformBridge *getPlatformBridge();

        void setPlatformBridge(PlatformBridge *pBridge);

        ScriptBridge* script_bridge();
        void set_script_bridge(ScriptBridge *script_bridge);

        void SetMeasureFunctionAdapter(MeasureFunctionAdapter *measureFunctionAdapter);

        MeasureFunctionAdapter *GetMeasureFunctionAdapter();

        inline ProjectMode project_mode() {
            return project_mode_;
        }

        inline void set_project_mode(ProjectMode mode) {
            project_mode_ = mode;
        }

        // Should be called on ScriptThread
        inline void InitScriptThread() {
            script_thread_ = new weex::base::Thread(weex::base::MessageLoop::Type::PLATFORM);
            script_thread_->Start();
        }

        inline weex::base::Thread* script_thread() {
            return script_thread_;
        }

    private:
        static WeexCoreManager *m_pInstance;
        PlatformBridge *platformBridge;
        MeasureFunctionAdapter *measureFunctionAdapter;
        ScriptBridge* script_bridge_;
        ProjectMode project_mode_;
        weex::base::Thread* script_thread_;


        WeexCoreManager() : platformBridge(nullptr), measureFunctionAdapter(nullptr), script_bridge_(
                nullptr), project_mode_(COMMON), script_thread_(nullptr){};
        ~WeexCoreManager() {};
    };
}

#endif //WEEXCORE_WEEX_CORE_MANAGER_H