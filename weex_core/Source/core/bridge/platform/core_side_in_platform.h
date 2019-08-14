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

#ifndef CORE_BRIDGE_PLATFORM_CORE_SIDE_IN_SIMPLE_H
#define CORE_BRIDGE_PLATFORM_CORE_SIDE_IN_SIMPLE_H

#include "base/common.h"
#include "core/bridge/platform_bridge.h"

namespace WeexCore {
class CoreSideInPlatform : public PlatformBridge::CoreSide {
 public:
  explicit CoreSideInPlatform();
  virtual ~CoreSideInPlatform();
  void SetDefaultHeightAndWidthIntoRootDom(
      const std::string &instance_id, float default_width, float default_height,
      bool is_width_wrap_content, bool is_height_wrap_content) override;
  void OnInstanceClose(const std::string &instance_id) override;
  void SetStyleWidth(const std::string &instance_id,
                     const std::string &render_ref, float width) override;
  void SetStyleHeight(const std::string &instance_id,
                      const std::string &render_ref, float height) override;
  void SetMargin(const std::string &instance_id, const std::string &render_ref,
                 int edge, float value) override;
  void SetPadding(const std::string &instance_id, const std::string &render_ref,
                  int edge, float value) override;
  void SetPosition(const std::string &instance_id,
                   const std::string &render_ref, int edge,
                   float value) override;
  void MarkDirty(const std::string &instance_id,
                 const std::string &render_ref) override;

  virtual void SetPageRenderType(const std::string &pageId, const std::string &renderType)override;
  virtual void RemovePageRenderType(const std::string &pageId) override;

  void SetViewPortWidth(const std::string &instance_id, float width) override; // set page required view port width
  void SetDeviceDisplayOfPage(const std::string &instance_id, float width, float height /* unused now */) override; // set page required device width
  void SetPageArgument(const std::string &pageId, const std::string& key, const std::string& value) override;
  void SetDeviceDisplay(const std::string &instance_id, float width, float height, float scale) override;
  void SetPageDirty(const std::string &instance_id) override;
  void ForceLayout(const std::string &instance_id) override;
  bool NotifyLayout(const std::string &instance_id) override;
  bool RelayoutUsingRawCssStyles(const std::string& instance_id) override;
  std::vector<int64_t> GetFirstScreenRenderTime(
      const std::string &instance_id) override;
  std::vector<int64_t> GetRenderFinishTime(
      const std::string &instance_id) override;
  void SetRenderContainerWrapContent(const std::string &instance_id,
                                     bool wrap) override;
  void BindMeasurementToRenderObject(long ptr) override;
  void RegisterCoreEnv(const std::string &key,
                       const std::string &value) override;

  long GetRenderObject(const std::string &instance_id,
                       const std::string &render_ref) override;
  void UpdateRenderObjectStyle(long render_ptr, const std::string &key,
                               const std::string &value) override;
  void UpdateRenderObjectAttr(long render_ptr, const std::string &key,
                              const std::string &value) override;
  long CopyRenderObject(long render_ptr) override;
  void SetMeasureFunctionAdapter() override;
  void SetPlatform(const std::string &platformName) override;
  void SetDeviceWidthAndHeight(float width, float height) override;
  void AddOption(const std::string &key, const std::string &value) override;

  int RefreshInstance(const char *instanceId, const char *nameSpace,
                      const char *func,
                      std::vector<VALUE_WITH_TYPE *> &params) override;
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
  std::unique_ptr<WeexJSResult> ExecJSWithResult(
      const char *instanceId, const char *nameSpace, const char *func,
      std::vector<VALUE_WITH_TYPE *> &params) override;
  void ExecJSWithCallback(const char *instanceId, const char *nameSpace,
                          const char *func,
                          std::vector<VALUE_WITH_TYPE *> &params,
                          long callback_id) override;
  int CreateInstance(const char *instanceId, const char *func,
                     const char *script, int script_length, const char *opts, const char *initData,
                     const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params, const char* render_strategy) override;
  std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char *instanceId,
                               const char *script,int type) override;
  int DestroyInstance(const char *instanceId) override;
  int UpdateGlobalConfig(const char *config) override;

  int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc) override;
  void SetLogType(const int logType, const bool isPerf) override;
  double GetLayoutTime(const char* instanceId) const override;

private:
  DISALLOW_COPY_AND_ASSIGN(CoreSideInPlatform);
};
}  // namespace WeexCore

#endif  // CORE_BRIDGE_PLATFORM_CORE_SIDE_IN_SIMPLE_H
