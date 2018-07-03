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

#include "weex_core_manager.h"

namespace WeexCore {

    WeexCoreManager *WeexCoreManager::m_pInstance = nullptr;

    void WeexCoreManager::setPlatformBridge(PlatformBridge *pBridge) {
        this->platformBridge = pBridge;
    }

    PlatformBridge* WeexCoreManager::getPlatformBridge() {
        return this->platformBridge;
    }

    void WeexCoreManager::set_script_bridge(ScriptBridge *script_bridge) {
        script_bridge_ = script_bridge;
    }

    ScriptBridge* WeexCoreManager::script_bridge() {
        return script_bridge_;
    }

    void WeexCoreManager::SetMeasureFunctionAdapter(MeasureFunctionAdapter *measureFunctionAdapter) {
      this->measureFunctionAdapter = measureFunctionAdapter;
    }

    MeasureFunctionAdapter * WeexCoreManager::GetMeasureFunctionAdapter() {
      return this->measureFunctionAdapter;
    }
}