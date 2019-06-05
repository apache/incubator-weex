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
#ifndef Bridge_h
#define Bridge_h

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "base/common.h"
#include "base/closure.h"
#include "include/WeexApiHeader.h"

namespace WeexCore {
class MeasureFunctionAdapter;
class WXCoreMargin;
class WXCorePadding;
class WXCoreBorderWidth;
class WXCoreSize;

class PlatformBridge {
 public:
  class CoreSide {
   public:
    explicit CoreSide() : measure_function_adapter_exist_(false) {}
    virtual ~CoreSide() {}
    virtual void SetDefaultHeightAndWidthIntoRootDom(
        const std::string& instance_id, float default_width,
        float default_height, bool is_width_wrap_content,
        bool is_height_wrap_content) = 0;
    virtual void OnInstanceClose(const std::string& instance_id) = 0;
    virtual void SetStyleWidth(const std::string& instance_id,
                               const std::string& render_ref, float width) = 0;
    virtual void SetStyleHeight(const std::string& instance_id,
                                const std::string& render_ref,
                                float height) = 0;
    virtual void SetMargin(const std::string& instance_id,
                           const std::string& render_ref, int edge,
                           float value) = 0;
    virtual void SetPadding(const std::string& instance_id,
                            const std::string& render_ref, int edge,
                            float value) = 0;
    virtual void SetPosition(const std::string& instance_id,
                             const std::string& render_ref, int edge,
                             float value) = 0;
    virtual void MarkDirty(const std::string& instance_id,
                           const std::string& render_ref) = 0;
    virtual void SetViewPortWidth(const std::string& instance_id,
                                  float width) = 0;
    virtual void SetPageArgument(const std::string &pageId, const std::string& key, const std::string& value) = 0;
    virtual void SetDeviceDisplayOfPage(const std::string &instance_id, float width, float height /* unused now */) = 0;
    virtual void SetDeviceDisplay(const std::string &instance_id, float width, float height, float scale) = 0;

    virtual void SetPageDirty(const std::string& instance_id) = 0;
    virtual void ForceLayout(const std::string& instance_id) = 0;
    virtual bool NotifyLayout(const std::string& instance_id) = 0;
    virtual std::vector<int64_t> GetFirstScreenRenderTime(
        const std::string& instance_id) = 0;
    virtual std::vector<int64_t> GetRenderFinishTime(
        const std::string& instance_id) = 0;
    virtual bool RelayoutUsingRawCssStyles(const std::string& instance_id) = 0; // relayout whole page using raw css styles
    virtual void SetRenderContainerWrapContent(const std::string& instance_id,
                                               bool wrap) = 0;
    virtual void BindMeasurementToRenderObject(long ptr) = 0;
    virtual void RegisterCoreEnv(const std::string& key,
                                 const std::string& value) = 0;

    virtual long GetRenderObject(const std::string& instance_id,
                                 const std::string& render_ref) = 0;
    virtual void UpdateRenderObjectStyle(long render_ptr,
                                         const std::string& key,
                                         const std::string& value) = 0;
    virtual void UpdateRenderObjectAttr(long render_ptr, const std::string& key,
                                        const std::string& value) = 0;
    virtual long CopyRenderObject(long render_ptr) = 0;
    virtual void SetMeasureFunctionAdapter() = 0;
    virtual void SetPlatform(const std::string& platformName) = 0;
    virtual void SetDeviceWidthAndHeight(float width, float height) = 0;
    virtual void AddOption(const std::string& key,
                           const std::string& value) = 0;

    virtual int RefreshInstance(const char* instanceId, const char* nameSpace,
                                const char* func,
                                std::vector<VALUE_WITH_TYPE*>& params) = 0;
    virtual int InitFramework(const char* script,
                              std::vector<INIT_FRAMEWORK_PARAMS*>& params) = 0;
    virtual int InitAppFramework(
        const char* instanceId, const char* appFramework,
        std::vector<INIT_FRAMEWORK_PARAMS*>& params) = 0;
    virtual int CreateAppContext(const char* instanceId,
                                 const char* jsBundle) = 0;
    virtual std::unique_ptr<WeexJSResult> ExecJSOnAppWithResult(const char* instanceId,
                                              const char* jsBundle) = 0;
    virtual int CallJSOnAppContext(const char* instanceId, const char* func,
                                   std::vector<VALUE_WITH_TYPE*>& params) = 0;
    virtual int DestroyAppContext(const char* instanceId) = 0;
    virtual int ExecJsService(const char* source) = 0;
    virtual int ExecTimeCallback(const char* source) = 0;
    virtual int ExecJS(const char* instanceId, const char* nameSpace,
                       const char* func,
                       std::vector<VALUE_WITH_TYPE*>& params) = 0;
    virtual std::unique_ptr<WeexJSResult> ExecJSWithResult(
        const char* instanceId, const char* nameSpace, const char* func,
        std::vector<VALUE_WITH_TYPE*>& params) = 0;
    virtual void ExecJSWithCallback(const char* instanceId,
                                    const char* nameSpace, const char* func,
                                    std::vector<VALUE_WITH_TYPE*>& params,
                                    long callback_id) = 0;
    virtual int CreateInstance(const char* instanceId, const char* func,
                               const char* script, int script_length, const char* opts,
                               const char* initData, const char* extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params,
                               const char* render_strategy) = 0;
    virtual std::unique_ptr<WeexJSResult> ExecJSOnInstance(const char* instanceId,
                                         const char* script,int type) = 0;
    virtual int DestroyInstance(const char* instanceId) = 0;

    virtual int UpdateGlobalConfig(const char* config) = 0;

    virtual int UpdateInitFrameworkParams(const std::string& key, const std::string& value, const std::string& desc) = 0;

    inline PlatformBridge* bridge() { return bridge_; }

   protected:
    bool measure_function_adapter_exist_;

   private:
    PlatformBridge* bridge_;
    friend class PlatformBridge;
    inline void set_bridge(PlatformBridge* bridge) { bridge_ = bridge; }
    DISALLOW_COPY_AND_ASSIGN(CoreSide);
  };

  class PlatformSide {
   public:
    PlatformSide() {}
    virtual ~PlatformSide() {}
    virtual WXCoreSize InvokeMeasureFunction(const char* page_id,
                                             long render_ptr, float width,
                                             int width_measure_mode,
                                             float height,
                                             int height_measure_mode) = 0;
    virtual void InvokeLayoutBefore(const char* page_id, long render_ptr) = 0;
    virtual void InvokeLayoutPlatform(const char* page_id, long render_ptr) = 0;
    virtual void InvokeLayoutAfter(const char* page_id, long render_ptr,
                                   float width, float height) = 0;
    virtual void TriggerVSync(const char* page_id) = 0; // vsync loop may stop on some platforms like iOS
      
    virtual void SetJSVersion(const char* version) = 0;
    virtual void ReportException(const char* page_id, const char* func,
                                 const char* exception_string) = 0;
    virtual void ReportServerCrash(const char* instance_id) = 0;
    virtual void ReportNativeInitStatus(const char* status_code,
                                        const char* error_msg) = 0;
    virtual int CallNative(const char* page_id, const char* task,
                           const char* callback) = 0;
    virtual std::unique_ptr<ValueWithType> CallNativeModule(
        const char* page_id, const char* module, const char* method,
        const char* arguments, int arguments_length, const char* options,
        int options_length) = 0;
    virtual void CallNativeComponent(const char* page_id, const char* ref,
                                     const char* method, const char* arguments,
                                     int arguments_length, const char* options,
                                     int options_length) = 0;
#if OS_IOS
    virtual std::unique_ptr<ValueWithType> RegisterPluginModule(const char *name, const char *class_name, const char *version) = 0;
    virtual std::unique_ptr<ValueWithType> RegisterPluginComponent(const char *name, const char *class_name, const char *version) = 0;
    virtual void PostTaskOnComponentThread(const weex::base::Closure closure) = 0;
#endif
    virtual void SetTimeout(const char* callback_id, const char* time) = 0;
    virtual void NativeLog(const char* str_array) = 0;
    virtual int UpdateFinish(const char* page_id, const char* task, int taskLen,
                             const char* callback, int callbackLen) = 0;
    virtual int RefreshFinish(const char* page_id, const char* task,
                              const char* callback) = 0;
    virtual int AddEvent(const char* page_id, const char* ref,
                         const char* event) = 0;
    virtual int RemoveEvent(const char* page_id, const char* ref,
                            const char* event) = 0;
    virtual int CreateBody(const char* pageId, const char* componentType,
                           const char* ref,
                           std::map<std::string, std::string>* styles,
                           std::map<std::string, std::string>* attributes,
                           std::set<std::string>* events,
                           const WXCoreMargin& margins,
                           const WXCorePadding& paddings,
                           const WXCoreBorderWidth& borders) = 0;

    virtual int AddElement(const char* pageId, const char* componentType,
                           const char* ref, int& index, const char* parentRef,
                           std::map<std::string, std::string>* styles,
                           std::map<std::string, std::string>* attributes,
                           std::set<std::string>* events,
                           const WXCoreMargin& margins,
                           const WXCorePadding& paddings,
                           const WXCoreBorderWidth& borders,
                           bool willLayout = true) = 0;
    virtual int Layout(const char* page_id, const char* ref, float top,
                       float bottom, float left, float right, float height,
                       float width, bool isRTL, int index) = 0;

    virtual int UpdateStyle(
        const char* pageId, const char* ref,
        std::vector<std::pair<std::string, std::string>>* style,
        std::vector<std::pair<std::string, std::string>>* margin,
        std::vector<std::pair<std::string, std::string>>* padding,
        std::vector<std::pair<std::string, std::string>>* border) = 0;
    virtual int UpdateAttr(
        const char* pageId, const char* ref,
        std::vector<std::pair<std::string, std::string>>* attrs) = 0;

    virtual int CreateFinish(const char* pageId) = 0;

    virtual int RenderSuccess(const char* pageId) = 0;

    virtual int RemoveElement(const char* pageId, const char* ref) = 0;

    virtual int MoveElement(const char* pageId, const char* ref,
                            const char* parentRef, int index) = 0;

    virtual int AppendTreeCreateFinish(const char* pageId, const char* ref) = 0;

    virtual int HasTransitionPros(
        const char* pageId, const char* ref,
        std::vector<std::pair<std::string, std::string>>* style) = 0;
    virtual void PostMessage(const char* vm_id, const char* data, int dataLength) = 0;
    virtual void DispatchMessage(const char* client_id, const char* data,
                                 int dataLength, const char* callback,
                                 const char* vm_id) = 0;
    virtual std::unique_ptr<WeexJSResult> DispatchMessageSync(
        const char* client_id, const char* data, int dataLength,
        const char* vm_id) = 0;
    virtual void OnReceivedResult(long callback_id,
                                  std::unique_ptr<WeexJSResult>& result) = 0;

    inline PlatformBridge* bridge() { return bridge_; }

   private:
    PlatformBridge* bridge_;
    friend class PlatformBridge;
    inline void set_bridge(PlatformBridge* bridge) { bridge_ = bridge; }
    DISALLOW_COPY_AND_ASSIGN(PlatformSide);
  };

  inline CoreSide* core_side() { return core_side_.get(); }
  inline PlatformSide* platform_side() { return platform_side_.get(); }

  inline void set_core_side(CoreSide* core_side) {
    core_side_.reset(core_side);
    core_side_->set_bridge(this);
  }
  inline void set_platform_side(PlatformSide* platform_side) {
    platform_side_.reset(platform_side);
    platform_side_->set_bridge(this);
  }

  PlatformBridge() : is_passable_(true) {}
  virtual ~PlatformBridge() {}

  inline bool is_passable() { return is_passable_; }

 protected:
  inline void set_is_passable(bool passable) { is_passable_ = passable; }

 private:
  std::unique_ptr<CoreSide> core_side_;
  std::unique_ptr<PlatformSide> platform_side_;
  bool is_passable_;
  DISALLOW_COPY_AND_ASSIGN(PlatformBridge);
};
}  // namespace WeexCore
#endif  // Bridge_h
