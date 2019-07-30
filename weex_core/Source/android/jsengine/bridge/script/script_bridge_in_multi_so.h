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

#ifndef WEEXV8_MULTI_SO_JS_BRIDGE_H
#define WEEXV8_MULTI_SO_JS_BRIDGE_H

#include "core/bridge/script_bridge.h"

namespace weex {
namespace bridge {
namespace js {
class ScriptBridgeInMultiSo : public WeexCore::ScriptBridge {
 public:
  static FunctionsExposedByJS *GetExposedFunctions();

  static int InitFramework(const char *script,
                           std::vector<INIT_FRAMEWORK_PARAMS *> &params);

  static int InitAppFramework(const char *instanceId, const char *appFramework,
                              std::vector<INIT_FRAMEWORK_PARAMS *> &params);

  static int CreateAppContext(const char *instanceId, const char *jsBundle);

  static std::unique_ptr<WeexJSResult> ExecJSOnAppWithResult(const char *instanceId,
                                     const char *jsBundle);

  static int CallJSOnAppContext(const char *instanceId, const char *func,
                                std::vector<VALUE_WITH_TYPE *> &params);

  static int DestroyAppContext(const char *instanceId);

  static int ExecJSService(const char *source);

  static int ExecTimeCallback(const char *source);

  static int ExecJS(const char *instanceId, const char *nameSpace,
                    const char *func, std::vector<VALUE_WITH_TYPE *> &params);

  static std::unique_ptr<WeexJSResult>  ExecJSWithResult(const char *instanceId,
                                       const char *nameSpace, const char *func,
                                       std::vector<VALUE_WITH_TYPE *> &params);

  static void ExecJSWithCallback(const char *instanceId,
                                       const char *nameSpace, const char *func,
                                       std::vector<VALUE_WITH_TYPE *> &params, long callback_id);


  static int CreateInstance(const char *instanceId, const char *func,
                            const char *script, const char *opts,
                            const char *initData, const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params);

  static std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char *instanceId, const char *script,int type);

  static int DestroyInstance(const char *instanceId);

  static int UpdateGlobalConfig(const char *config);
  
  static int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc);


  static void SetLogType(const int logLevel, const bool isPerf);

 private:
  static ScriptBridgeInMultiSo *g_instance;
  static ScriptBridgeInMultiSo *Instance() {
    if (g_instance == NULL) {
      g_instance = new ScriptBridgeInMultiSo();
    }
    return g_instance;
  }

  ScriptBridgeInMultiSo();
  virtual ~ScriptBridgeInMultiSo();
  DISALLOW_COPY_AND_ASSIGN(ScriptBridgeInMultiSo);
};
}  // namespace js
}  // namespace bridge
}  // namespace weex

#endif  // WEEXV8_MULTI_SO_JS_BRIDGE_H
