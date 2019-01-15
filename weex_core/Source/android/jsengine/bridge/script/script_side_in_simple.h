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
// Created by yxp on 2018/6/15.
//

#ifndef WEEXV8_SIMPLE_JS_SIDE_H
#define WEEXV8_SIMPLE_JS_SIDE_H

#include "core/bridge/script_bridge.h"

class WeexRuntime;

namespace weex {
namespace bridge {
namespace js {
class ScriptSideInSimple : public WeexCore::ScriptBridge::ScriptSide {
 public:
  int InitFramework(const char *script,
                    std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

  int InitAppFramework(const char *instanceId, const char *appFramework,
                       std::vector<INIT_FRAMEWORK_PARAMS *> &params) override;

  int CreateAppContext(const char *instanceId, const char *jsBundle) override;

  std::unique_ptr<WeexJSResult> ExecJSOnAppWithResult(const char *instanceId,
                              const char *jsBundle) override;

  int CallJSOnAppContext(const char *instanceId, const char *func,
                         std::vector<VALUE_WITH_TYPE *> &params) override;

  int DestroyAppContext(const char *instanceId) override;

  int ExecJsService(const char *source) override;

  int ExecTimeCallback(const char *source) override;

  int ExecJS(const char *instanceId, const char *nameSpace, const char *func,
             std::vector<VALUE_WITH_TYPE *> &params) override;

  std::unique_ptr<WeexJSResult>  ExecJSWithResult(const char *instanceId, const char *nameSpace,
                                const char *func,
                                std::vector<VALUE_WITH_TYPE *> &params) override;

  void ExecJSWithCallback(const char *instanceId, const char *nameSpace,
                                const char *func,
                                std::vector<VALUE_WITH_TYPE *> &params, long callback_id) override;

  int CreateInstance(const char *instanceId, const char *func,
                     const char *script, const char *opts, const char *initData,
                     const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params) override;

  std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char *instanceId, const char *script) override;

  int DestroyInstance(const char *instanceId) override;

  int UpdateGlobalConfig(const char *config) override;

  inline void set_runtime(WeexRuntime *runtime) { runtime_ = runtime; }

  ScriptSideInSimple() : runtime_(nullptr) {}
  ScriptSideInSimple(WeexRuntime *runtime) : runtime_(runtime) {}

  virtual ~ScriptSideInSimple() {}

 private:
  WeexRuntime *runtime_;
  DISALLOW_COPY_AND_ASSIGN(ScriptSideInSimple);
};
}  // namespace js
}  // namespace bridge

}  // namespace weex

#endif  // WEEXV8_SIMPLE_JS_SIDE_H
