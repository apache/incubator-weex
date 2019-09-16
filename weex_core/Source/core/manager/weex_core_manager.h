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

#ifndef WEEXCORE_WEEX_CORE_MANAGER_H
#define WEEXCORE_WEEX_CORE_MANAGER_H

#include "base/thread/thread.h"
#include "core/bridge/platform_bridge.h"
#include "core/bridge/script_bridge.h"
#include "base/message_loop/message_loop.h"

class IPCFutexPageQueue;

namespace WeexCore {
class WeexCoreManager {
 public:
  enum ProjectMode { MULTI_SO, MULTI_PROCESS, COMMON };

  static WeexCoreManager *Instance() {
    static auto s_instance = new WeexCoreManager();
    return s_instance;
  };

  inline PlatformBridge *getPlatformBridge() { return platform_bridge_; }

  inline void set_platform_bridge(PlatformBridge *bridge) {
    platform_bridge_ = bridge;
  }

  inline ScriptBridge *script_bridge() { return script_bridge_; }

  inline void set_script_bridge(ScriptBridge *script_bridge) {
    script_bridge_ = script_bridge;
  }

  inline void set_measure_function_adapter(MeasureFunctionAdapter *adapter) {
    measure_function_adapter_ = adapter;
  }

  inline MeasureFunctionAdapter *measure_function_adapter() {
    return measure_function_adapter_;
  }

  inline ProjectMode project_mode() { return project_mode_; }

  inline void set_project_mode(ProjectMode mode) { project_mode_ = mode; }

  // Should be called on ScriptThread
  inline void InitScriptThread() {
    script_thread_ =
        new weex::base::Thread(weex::base::MessageLoop::Type::PLATFORM);
    script_thread_->Start();
  }

  inline weex::base::Thread *script_thread() { return script_thread_; }

  IPCFutexPageQueue* client_queue_;
  IPCFutexPageQueue* server_queue_;

 private:
  PlatformBridge *platform_bridge_;
  MeasureFunctionAdapter *measure_function_adapter_;
  ScriptBridge *script_bridge_;
  ProjectMode project_mode_;
  weex::base::Thread *script_thread_;

  WeexCoreManager()
      : platform_bridge_(nullptr),
        measure_function_adapter_(nullptr),
        script_bridge_(nullptr),
        project_mode_(COMMON),
        script_thread_(nullptr),
        client_queue_(nullptr),
        server_queue_(nullptr){};
  ~WeexCoreManager(){};
};
}  // namespace WeexCore

#endif  // WEEXCORE_WEEX_CORE_MANAGER_H
#endif //#ifdef __cplusplus
