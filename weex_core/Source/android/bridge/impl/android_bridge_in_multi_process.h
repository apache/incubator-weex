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

#ifndef WEEX_PROJECT_IPC_BRIDGE_IMPL_H
#define WEEX_PROJECT_IPC_BRIDGE_IMPL_H

#include "bridge_impl_android.h"

class IPCArguments;
class IPCResult;
class IPCSender;

namespace WeexCore {

// For multi-process and multi so
class AndroidBridgeInMultiProcess : public Bridge_Impl_Android {
 public:
  AndroidBridgeInMultiProcess();
  virtual ~AndroidBridgeInMultiProcess();

  void RegisterIPCCallback(IPCHandler *handler);

#define IPC_METHOD(method_name) \
  static std::unique_ptr<IPCResult> method_name(IPCArguments *arguments);

  IPC_METHOD(InvokeMeasureFunction)
  IPC_METHOD(InvokeLayoutBefore)
  IPC_METHOD(InvokeLayoutAfter)
  IPC_METHOD(SetJSVersion)
  IPC_METHOD(ReportException)
  IPC_METHOD(CallNative)
  IPC_METHOD(CallNativeModule)
  IPC_METHOD(CallNativeComponent)
  IPC_METHOD(SetTimeout)
  IPC_METHOD(NativeLog)
  IPC_METHOD(UpdateFinish)
  IPC_METHOD(RefreshFinish)
  IPC_METHOD(AddEvent)
  IPC_METHOD(RemoveEvent)
  IPC_METHOD(CreateBody)
  IPC_METHOD(AddElement)
  IPC_METHOD(Layout)
  IPC_METHOD(UpdateStyle)
  IPC_METHOD(UpdateAttr)
  IPC_METHOD(CreateFinish)
  IPC_METHOD(RemoveElement)
  IPC_METHOD(MoveElement)

  IPC_METHOD(AppendTreeCreateFinish)
  IPC_METHOD(HasTransitionPros)
  IPC_METHOD(PostMessage)
  IPC_METHOD(DispatchMessage)

    private:
        std::unique_ptr<WeexJSConnection> connection_;
        std::unique_ptr<IPCHandler> handler_;
        DISALLOW_COPY_AND_ASSIGN(AndroidBridgeInMultiProcess);

  // For Environment
  //    void SetPlatform(const std::string& platformName) override ;
  //    void SetDeviceWidthAndHeight(float width, float height) override ;
  //    void AddOption(const std::string& key, const std::string& value)
  //    override ;
  //
  //        void SetDefaultHeightAndWidthIntoRootDom(const std::string
  //        &instance_id,
  //                                                 float default_width,
  //                                                 float default_height,
  //                                                 bool is_width_wrap_content,
  //                                                 bool
  //                                                 is_height_wrap_content)
  //                                                 override ;
  //        void OnInstanceClose(const std::string &instance_id) override ;
  //        void SetStyleWidth(const std::string &instance_id, const
  //        std::string& render_ref, float width) override ; void
  //        SetStyleHeight(const std::string &instance_id, const std::string&
  //        render_ref, float height) override ; void SetMargin(const
  //        std::string &instance_id, const std::string& render_ref, int edge,
  //        float value) override ; void SetPadding(const std::string
  //        &instance_id, const std::string& render_ref, int edge, float value)
  //        override ; void SetPosition(const std::string &instance_id, const
  //        std::string& render_ref, int edge, float value) override ; void
  //        MarkDirty(const std::string &instance_id, const std::string&
  //        render_ref) override ; void SetViewPortWidth(const std::string
  //        &instance_id, float width) override ; void ForceLayout(const
  //        std::string &instance_id) override ; bool NotifyLayout(const
  //        std::string &instance_id) override ; std::vector<int64_t>
  //        GetFirstScreenRenderTime(const std::string &instance_id) override ;
  //        std::vector<int64_t> GetRenderFinishTime(const std::string
  //        &instance_id) override ; void SetRenderContainerWrapContent(const
  //        std::string &instance_id, bool wrap) override ; void
  //        BindMeasurementToRenderObject(long ptr) override ;
  //
  //        long GetRenderObject(const std::string &instance_id, const
  //        std::string& render_ref) override ; void
  //        UpdateRenderObjectStyle(long render_ptr, const std::string& key,
  //        const std::string& value) override ; void
  //        UpdateRenderObjectAttr(long render_ptr, const std::string& key,
  //        const std::string& value) override ; long CopyRenderObject(long
  //        render_ptr) override ; void
  //        SetMeasureFunctionAdapter(MeasureFunctionAdapter
  //        *measureFunctionAdapter) override ; void RegisterCoreEnv(const
  //        std::string& key, const std::string& value) override ;

  //  static std::unique_ptr<IPCResult> HandleSetJSVersion(IPCArguments
  //  *arguments); static std::unique_ptr<IPCResult> HandleReportException(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleCallNative(IPCArguments
  //  *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleCallNativeModule(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleCallNativeComponent(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleCallAddElement(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleSetTimeout(IPCArguments
  //  *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleCallNativeLog(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallCreateBody(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallUpdateFinish(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallCreateFinish(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallRefreshFinish(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallUpdateAttrs(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallUpdateStyle(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallRemoveElement(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallMoveElement(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallAddEvent(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> FunctionCallRemoveEvent(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleSetInterval(IPCArguments
  //  *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleClearInterval(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleCallGCanvasLinkNative(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleT3DLinkNative(
  //      IPCArguments *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandlePostMessage(IPCArguments
  //  *arguments);
  //
  //  static std::unique_ptr<IPCResult> HandleDispatchMessage(
  //      IPCArguments *arguments);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_IPC_BRIDGE_IMPL_H
