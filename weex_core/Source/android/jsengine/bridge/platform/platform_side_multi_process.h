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

#ifndef WEEXV8_MULTI_PROCESS_PLATFORM_SIDE_H
#define WEEXV8_MULTI_PROCESS_PLATFORM_SIDE_H

#include "android/jsengine/weex_ipc_client.h"
#include "core/bridge/platform_bridge.h"

using namespace WeexCore;
class WeexJSServer;

namespace weex {

class PlatformSideInMultiProcess : public WeexCore::PlatformBridge::PlatformSide {
 public:
  PlatformSideInMultiProcess() {}
  virtual ~PlatformSideInMultiProcess() {}
  inline void set_server(WeexJSServer* server) { server_ = server; }
  inline void set_client(WeexIPCClient* client) {client_ = client; }
  WXCoreSize InvokeMeasureFunction(const char* page_id, long render_ptr,
                                   float width, int width_measure_mode,
                                   float height,
                                   int height_measure_mode) override;
  void InvokeLayoutBefore(const char* page_id, long render_ptr) override;
  void InvokeLayoutAfter(const char* page_id, long render_ptr, float width,
                         float height) override;
  void SetJSVersion(const char* version) override;
  void ReportException(const char* page_id, const char* func,
                       const char* exception_string) override;
    void ReportServerCrash(const char* instance_id) override;
    void ReportNativeInitStatus(const char* status_code,
                                const char* error_msg) override;
  int CallNative(const char* page_id, const char* task,
                 const char* callback) override;
  std::unique_ptr<ValueWithType> CallNativeModule(
      const char* page_ld, const char* module, const char* method,
      const char* arguments, int arguments_length, const char* options,
      int options_length) override;
  void CallNativeComponent(const char* page_id, const char* ref,
                           const char* method, const char* arguments,
                           int arguments_length, const char* options,
                           int options_length) override;
  void SetTimeout(const char* callback_id, const char* time) override;
  void NativeLog(const char* str_array) override;
  int UpdateFinish(const char* page_id, const char* task, int taskLen,
                   const char* callback, int callbackLen) override;
  int RefreshFinish(const char* page_id, const char* task,
                    const char* callback) override;
  int AddEvent(const char* page_id, const char* ref,
               const char* event) override;
  int RemoveEvent(const char* page_id, const char* ref,
                  const char* event) override;
  int CreateBody(const char* pageId, const char* componentType, const char* ref,
                 std::map<std::string, std::string>* styles,
                 std::map<std::string, std::string>* attributes,
                 std::set<std::string>* events, const WXCoreMargin& margins,
                 const WXCorePadding& paddings,
                 const WXCoreBorderWidth& borders) override;

  int AddElement(const char* pageId, const char* componentType, const char* ref,
                 int& index, const char* parentRef,
                 std::map<std::string, std::string>* styles,
                 std::map<std::string, std::string>* attributes,
                 std::set<std::string>* events, const WXCoreMargin& margins,
                 const WXCorePadding& paddings,
                 const WXCoreBorderWidth& borders, bool willLayout) override;
  int Layout(const char* page_id, const char* ref, float top, float bottom,
             float left, float right, float height,
             float width, bool isRTL, int index) override;

  int UpdateStyle(
      const char* pageId, const char* ref,
      std::vector<std::pair<std::string, std::string>>* style,
      std::vector<std::pair<std::string, std::string>>* margin,
      std::vector<std::pair<std::string, std::string>>* padding,
      std::vector<std::pair<std::string, std::string>>* border) override;
  int UpdateAttr(
      const char* pageId, const char* ref,
      std::vector<std::pair<std::string, std::string>>* attrs) override;

  int CreateFinish(const char* pageId) override;

  int RenderSuccess(const char* pageId) override;

  int RemoveElement(const char* pageId, const char* ref) override;

  int MoveElement(const char* pageId, const char* ref, const char* parentRef,
                  int index) override;

  int AppendTreeCreateFinish(const char* pageId, const char* ref) override;

  int HasTransitionPros(
      const char* pageId, const char* ref,
      std::vector<std::pair<std::string, std::string>>* style) override;
  void PostMessage(const char* vm_id, const char* data, int dataLength) override;
  void DispatchMessage(const char* client_id,
                       const char* data, int dataLength, const char* callback, const char* vm_id) override;

 private:
  WeexJSServer* server_;
  WeexIPCClient* client_;
  DISALLOW_COPY_AND_ASSIGN(PlatformSideInMultiProcess);
};

}  // namespace weex

#endif  // WEEXV8_MULTI_PROCESS_PLATFORM_SIDE_H
