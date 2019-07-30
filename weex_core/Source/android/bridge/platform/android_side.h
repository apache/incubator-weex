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

#ifndef WEEX_PROJECT_PLATFORM_SIDE_INSIMPLE_H
#define WEEX_PROJECT_PLATFORM_SIDE_INSIMPLE_H

#include "android/wrap/wx_bridge.h"
#include "base/common.h"
#include "core/bridge/platform_bridge.h"

namespace WeexCore {
class WMLBridge;
class AndroidSide : public PlatformBridge::PlatformSide {
 public:
  AndroidSide();
  virtual ~AndroidSide();
  WXCoreSize InvokeMeasureFunction(const char* page_id, long render_ptr,
                                   float width, int width_measure_mode,
                                   float height,
                                   int height_measure_mode) override;
  void InvokeLayoutBefore(const char* page_id, long render_ptr) override;
  void InvokeLayoutPlatform(const char* page_id, long render_ptr) override;
  void InvokeLayoutAfter(const char* page_id, long render_ptr, float width,
                         float height) override;
  void TriggerVSync(const char* page_id) override;
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
  int AddChildToRichtext(const char *pageId, const char *nodeType, const char *ref, const char *parentRef,
                         const char *richtextRef, std::map<std::string, std::string> *styles,
                         std::map<std::string, std::string> *attributes) override;
  int RemoveChildFromRichtext(const char *pageId, const char *ref, const char *parent_ref,
                              const char *richtext_ref) override;
  int UpdateRichtextStyle(const char *pageId, const char *ref,
                          std::vector<std::pair<std::string, std::string>> *style, const char *parent_ref,
                          const char *richtext_ref) override;
  int UpdateRichtextChildAttr(const char *pageId, const char *ref,
                              std::vector<std::pair<std::string, std::string>> *attrs,
                              const char *parent_ref, const char *richtext_ref) override;
  int Layout(const char* page_id, const char* ref, float top, float bottom,
             float left, float right, float height, float width, bool isRTL,
             int index) override;

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
  std::unique_ptr<WeexJSResult> DispatchMessageSync(const char* client_id,
                                                    const char* data,
                                                    int dataLength,
                                                    const char* vm_id) override;
  void OnReceivedResult(long callback_id, std::unique_ptr<WeexJSResult>& result) override;

  jobject getMeasureFunc(const char* pageId, jlong renderObjectPtr);

 private:
  WMLBridge* wml_bridge_;
  WXBridge* wx_bridge_;
  DISALLOW_COPY_AND_ASSIGN(AndroidSide);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_PLATFORM_SIDE_INSIMPLE_H
