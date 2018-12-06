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

#ifndef WEEX_PROJECT_MULTI_SO_BRIDGE_H
#define WEEX_PROJECT_MULTI_SO_BRIDGE_H

#include "core/bridge/platform_bridge.h"
#include "include/WeexApiHeader.h"

namespace WeexCore {
// For situation in single process and multi so
class AndroidBridgeInMultiSo : public PlatformBridge {
 public:
  AndroidBridgeInMultiSo();
  ~AndroidBridgeInMultiSo();

  PlatformExposeFunctions* GetExposedFunctions();

  static void SetJSVersion(const char* version);

  static void ReportException(const char* pageId, const char* func,
                              const char* exception_string);

  static void ReportServerCrash(const char* instance_id);

  static void ReportNativeInitStatus(const char* status_code,
                                     const char* error_msg);

  static int CallNative(const char* pageId, const char* task,
                        const char* callback);

  static std::unique_ptr<ValueWithType> CallNativeModule(
      const char* pageId, const char* module, const char* method,
      const char* arguments, int argumentsLength, const char* options,
      int optionsLength);

  static void CallNativeComponent(const char* pageId, const char* ref,
                                  const char* method, const char* arguments,
                                  int argumentsLength, const char* options,
                                  int optionsLength);

  static void SetTimeout(const char* callbackID, const char* time);

  static void CallNativeLog(const char* str_array);

  static int CallUpdateFinish(const char* pageId, const char* task, int taskLen,
                              const char* callback, int callbackLen);

  static int CallRefreshFinish(const char* pageId, const char* task,
                               const char* callback);

  static int CallAddEvent(const char* pageId, const char* ref,
                          const char* event);

  static int CallRemoveEvent(const char* pageId, const char* ref,
                             const char* event);

  static int CallCreateBody(const char* pageId, const char* componentType,
                            const char* ref,
                            std::map<std::string, std::string>* styles,
                            std::map<std::string, std::string>* attributes,
                            std::set<std::string>* events,
                            const WXCoreMargin& margins,
                            const WXCorePadding& paddings,
                            const WXCoreBorderWidth& borders);

  static int CallAddElement(const char* pageId, const char* componentType,
                            const char* ref, int& index, const char* parentRef,
                            std::map<std::string, std::string>* styles,
                            std::map<std::string, std::string>* attributes,
                            std::set<std::string>* events,
                            const WXCoreMargin& margins,
                            const WXCorePadding& paddings,
                            const WXCoreBorderWidth& borders,
                            bool willLayout = true);

  static int CallLayout(const char* pageId, const char* ref, int top,
                        int bottom, int left, int right, int height, int width,
                        int index);

  static int CallUpdateStyle(
      const char* pageId, const char* ref,
      std::vector<std::pair<std::string, std::string>>* style,
      std::vector<std::pair<std::string, std::string>>* margin,
      std::vector<std::pair<std::string, std::string>>* padding,
      std::vector<std::pair<std::string, std::string>>* border);

  static int CallUpdateAttr(
      const char* pageId, const char* ref,
      std::vector<std::pair<std::string, std::string>>* attrs);

  static int CallCreateFinish(const char* pageId);

  static int CallRenderSuccess(const char* pageId);

  static int CallRemoveElement(const char* pageId, const char* ref);

  static int CallMoveElement(const char* pageId, const char* ref,
                             const char* parentRef, int index);

  static void PostMessage(const char* vm_id, const char* data, int dataLength);

  static void DispatchMessage(const char* client_id,
                              const char* data, int dataLength, const char* callback, const char* vm_id);

  static int CallAppendTreeCreateFinish(const char* pageId, const char* ref);

  static int CallHasTransitionPros(
      const char* pageId, const char* ref,
      std::vector<std::pair<std::string, std::string>>* style);
  static WXCoreSize CallInvokeMeasureFunction(const char* page_id,
                                              long render_ptr, float width,
                                              int width_measure_mode,
                                              float height,
                                              int height_measure_mode);
  static void CallInvokeLayoutBefore(const char* page_id, long render_ptr);
  static void CallInvokeLayoutAfter(const char* page_id, long render_ptr,
                                    float width, float height);

 private:
  DISALLOW_COPY_AND_ASSIGN(AndroidBridgeInMultiSo);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_MULTI_SO_BRIDGE_H
