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
// Created by yxp on 2018/6/12.
//

#ifndef WEEXV8_PLATFORM_MULTI_SO_BRIDGE_H
#define WEEXV8_PLATFORM_MULTI_SO_BRIDGE_H

#include <map>
#include <set>
#include <vector>
#include "base/common.h"
#include "core/bridge/platform_bridge.h"
#include "include/WeexApiHeader.h"

namespace weex {

class PlatformBridgeInMultiSo : public WeexCore::PlatformBridge {
 public:
  static PlatformBridgeInMultiSo *Instance() {
    if (g_instance == NULL) {
      g_instance = new PlatformBridgeInMultiSo();
    }
    return g_instance;
  }

  static CoreSideFunctionsOfPlatformBridge *GetExposedFunctions();
  static void SetDefaultHeightAndWidthIntoRootDom(
      const std::string &instance_id, float default_width, float default_height,
      bool is_width_wrap_content, bool is_height_wrap_content);
  static void OnInstanceClose(const std::string &instance_id);
  static void SetStyleWidth(const std::string &instance_id,
                            const std::string &render_ref, float width);
  static void SetStyleHeight(const std::string &instance_id,
                             const std::string &render_ref, float height);
  static void SetMargin(const std::string &instance_id,
                        const std::string &render_ref, int edge, float value);
  static void SetPadding(const std::string &instance_id,
                         const std::string &render_ref, int edge, float value);
  static void SetPosition(const std::string &instance_id,
                          const std::string &render_ref, int edge, float value);
  static void MarkDirty(const std::string &instance_id,
                        const std::string &render_ref);
  static void SetViewPortWidth(const std::string &instance_id, float width);
  static void ForceLayout(const std::string &instance_id);
  static bool NotifyLayout(const std::string &instance_id);
  static std::vector<int64_t> GetFirstScreenRenderTime(
      const std::string &instance_id);
  static std::vector<int64_t> GetRenderFinishTime(
      const std::string &instance_id);
  static void SetRenderContainerWrapContent(const std::string &instance_id,
                                            bool wrap);
  static void BindMeasurementToRenderObject(long ptr);
  static void RegisterCoreEnv(const std::string &key, const std::string &value);

  static long GetRenderObject(const std::string &instance_id,
                              const std::string &render_ref);
  static void UpdateRenderObjectStyle(long render_ptr, const std::string &key,
                                      const std::string &value);
  static void UpdateRenderObjectAttr(long render_ptr, const std::string &key,
                                     const std::string &value);
  static long CopyRenderObject(long render_ptr);
  static void SetMeasureFunctionAdapter();
  static void SetPlatform(const std::string &platformName);
  static void SetDeviceWidthAndHeight(float width, float height);
  static void AddOption(const std::string &key, const std::string &value);

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
  static int ExecJsService(const char *source);
  static int ExecTimeCallback(const char *source);
  static int ExecJS(const char *instanceId, const char *nameSpace,
                    const char *func, std::vector<VALUE_WITH_TYPE *> &params);
  static std::unique_ptr<WeexJSResult>  ExecJSWithResult(const char *instanceId,
                                       const char *nameSpace, const char *func,
                                       std::vector<VALUE_WITH_TYPE *> &params);
  static int CreateInstance(const char *instanceId, const char *func,
                            const char *script, const char *opts,
                            const char *initData, const char *extendsApi,
                            std::vector<INIT_FRAMEWORK_PARAMS*>& params);
  static std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char *instanceId, const char *script);
  static int DestroyInstance(const char *instanceId);
  static int UpdateGlobalConfig(const char *config);
  static int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc);



 private:
  PlatformBridgeInMultiSo();
  virtual ~PlatformBridgeInMultiSo();

  static PlatformBridgeInMultiSo *g_instance;
  DISALLOW_COPY_AND_ASSIGN(PlatformBridgeInMultiSo);
};
}  // namespace weex

#endif  // WEEXV8_PLATFORM_MULTI_SO_BRIDGE_H
