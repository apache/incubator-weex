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

#ifndef WEEX_PROJECT_CORE_SIDE_IN_MULTI_SO_H
#define WEEX_PROJECT_CORE_SIDE_IN_MULTI_SO_H

#include "core/bridge/platform_bridge.h"
#include "include/WeexApiHeader.h"

namespace WeexCore {
class CoreSideInMultiSo : public PlatformBridge::CoreSide {
 public:
  CoreSideInMultiSo() {}
  virtual ~CoreSideInMultiSo() {}
  void SetDefaultHeightAndWidthIntoRootDom(
      const std::string& instance_id, float default_width, float default_height,
      bool is_width_wrap_content, bool is_height_wrap_content) override;
  inline void set_core_side_functions(
      CoreSideFunctionsOfPlatformBridge* core_side_functions) {
    core_side_functions_ = core_side_functions;
  }
  void OnInstanceClose(const std::string& instance_id) override;
  void SetStyleWidth(const std::string& instance_id,
                     const std::string& render_ref, float width) override;
  void SetStyleHeight(const std::string& instance_id,
                      const std::string& render_ref, float height) override;
  void SetMargin(const std::string& instance_id, const std::string& render_ref,
                 int edge, float value) override;
  void SetPadding(const std::string& instance_id, const std::string& render_ref,
                  int edge, float value) override;
  void SetPosition(const std::string& instance_id,
                   const std::string& render_ref, int edge,
                   float value) override;
  void MarkDirty(const std::string& instance_id,
                 const std::string& render_ref) override;
  void SetViewPortWidth(const std::string& instance_id, float width) override;
  void ForceLayout(const std::string& instance_id) override;
  bool NotifyLayout(const std::string& instance_id) override;
  std::vector<int64_t> GetFirstScreenRenderTime(
      const std::string& instance_id) override;
  std::vector<int64_t> GetRenderFinishTime(
      const std::string& instance_id) override;
  void SetRenderContainerWrapContent(const std::string& instance_id,
                                     bool wrap) override;
  void BindMeasurementToRenderObject(long ptr) override;
  void RegisterCoreEnv(const std::string& key,
                       const std::string& value) override;

  long GetRenderObject(const std::string& instance_id,
                       const std::string& render_ref) override;
  void UpdateRenderObjectStyle(long render_ptr, const std::string& key,
                               const std::string& value) override;
  void UpdateRenderObjectAttr(long render_ptr, const std::string& key,
                              const std::string& value) override;
  long CopyRenderObject(long render_ptr) override;
  void SetMeasureFunctionAdapter() override;
  void SetPlatform(const std::string& platformName) override;
  void SetDeviceWidthAndHeight(float width, float height) override;
  void AddOption(const std::string& key, const std::string& value) override;

    int RefreshInstance(const char* instanceId, const char* nameSpace,
                                const char* func,
                                std::vector<VALUE_WITH_TYPE *> &params) override ;
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
    std::unique_ptr<WeexJSResult> ExecJSWithResult(const char *instanceId, const char *nameSpace,
                                  const char *func,
                                  std::vector<VALUE_WITH_TYPE *> &params) override;
    int CreateInstance(const char *instanceId, const char *func,
                       const char *script, const char *opts, const char *initData,
                       const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params, const char* render_strategy) override;
    std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char *instanceId, const char *script) override;
    int DestroyInstance(const char *instanceId) override;
    int UpdateGlobalConfig(const char *config) override;
 private:
  CoreSideFunctionsOfPlatformBridge* core_side_functions_;
  DISALLOW_COPY_AND_ASSIGN(CoreSideInMultiSo);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_CORE_SIDE_IN_MULTI_SO_H
