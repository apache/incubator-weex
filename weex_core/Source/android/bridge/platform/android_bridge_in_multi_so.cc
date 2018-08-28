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

#include "android_bridge_in_multi_so.h"
#include <dlfcn.h>
#include "android/bridge/platform/android_side.h"
#include "core/layout/layout.h"
#include "core/manager/weex_core_manager.h"
#include "core_side_in_multi_so.h"
#include "include/WeexApiHeader.h"
#include "android/bridge/multi_so_initializer.h"

namespace WeexCore {
AndroidBridgeInMultiSo::AndroidBridgeInMultiSo() {
  set_platform_side(new AndroidSide);
  set_core_side(new CoreSideInMultiSo);

  std::unique_ptr<MultiSoInitializer> initializer(new MultiSoInitializer);
  bool passable = initializer->Init(
      [this](void *handle) {
        LOGE("dlopen so and call function");

        typedef CoreSideFunctionsOfPlatformBridge *(
            *ExchangePlatformBridgeFunctions)(PlatformExposeFunctions *);
        auto exchange_platform_bridge_functions =
            (ExchangePlatformBridgeFunctions)dlsym(
                handle, "ExchangePlatformBridgeFunctions");
        if (!exchange_platform_bridge_functions) {
          return false;
        }

        PlatformExposeFunctions *platform_expose_functions =
            GetExposedFunctions();
        auto core_side_functions =
            exchange_platform_bridge_functions(platform_expose_functions);

        static_cast<CoreSideInMultiSo *>(core_side())
            ->set_core_side_functions(core_side_functions);
        return true;
      },
      [this](const char *status_code, const char *error_msg) {
        platform_side()->ReportNativeInitStatus(status_code, error_msg);
      });
  set_is_passable(passable);
}

AndroidBridgeInMultiSo::~AndroidBridgeInMultiSo() {}

PlatformExposeFunctions *AndroidBridgeInMultiSo::GetExposedFunctions() {
  PlatformExposeFunctions temp = {SetJSVersion,
                                  ReportException,
                                  ReportServerCrash,
                                  ReportNativeInitStatus,
                                  CallNative,
                                  CallNativeModule,
                                  CallNativeComponent,
                                  CallAddElement,
                                  SetTimeout,
                                  CallNativeLog,
                                  CallCreateBody,
                                  CallUpdateFinish,
                                  CallCreateFinish,
                                  CallRenderSuccess,
                                  CallRefreshFinish,
                                  CallUpdateAttr,
                                  CallUpdateStyle,
                                  CallRemoveElement,
                                  CallMoveElement,
                                  CallAddEvent,
                                  CallRemoveEvent,
                                  PostMessage,
                                  DispatchMessage,
                                  CallLayout,
                                  CallAppendTreeCreateFinish,
                                  CallHasTransitionPros,
                                  CallInvokeMeasureFunction,
                                  CallInvokeLayoutBefore,
                                  CallInvokeLayoutAfter};
  auto functions =
      (PlatformExposeFunctions *)malloc(sizeof(PlatformExposeFunctions));
  memset(functions, 0, sizeof(PlatformExposeFunctions));
  memcpy(functions, &temp, sizeof(PlatformExposeFunctions));
  return functions;
}

void AndroidBridgeInMultiSo::SetJSVersion(const char *version) {
  //  LOGE("Android Bridge: SetJSVersion");
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->SetJSVersion(version);
}

void AndroidBridgeInMultiSo::ReportException(const char *pageId,
                                             const char *func,
                                             const char *exception_string) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->ReportException(pageId, func, exception_string);
}

void AndroidBridgeInMultiSo::ReportServerCrash(const char *instance_id) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->ReportServerCrash(instance_id);
}

void AndroidBridgeInMultiSo::ReportNativeInitStatus(const char *status_code,
                                                    const char *error_msg) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->ReportNativeInitStatus(status_code, error_msg);
}

int AndroidBridgeInMultiSo::CallNative(const char *pageId, const char *task,
                                       const char *callback) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNative(pageId, task, callback);
}

std::unique_ptr<ValueWithType> AndroidBridgeInMultiSo::CallNativeModule(
    const char *pageId, const char *module, const char *method,
    const char *arguments, int argumentsLength, const char *options,
    int optionsLength) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNativeModule(pageId, module, method, arguments, argumentsLength,
                         options, optionsLength);
}

void AndroidBridgeInMultiSo::CallNativeComponent(
    const char *pageId, const char *ref, const char *method,
    const char *arguments, int argumentsLength, const char *options,
    int optionsLength) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNativeComponent(pageId, ref, method, arguments, argumentsLength,
                            options, optionsLength);
}

void AndroidBridgeInMultiSo::SetTimeout(const char *callbackID,
                                        const char *time) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->SetTimeout(callbackID, time);
}

void AndroidBridgeInMultiSo::CallNativeLog(const char *str_array) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->NativeLog(str_array);
}

int AndroidBridgeInMultiSo::CallUpdateFinish(const char *pageId,
                                             const char *task, int taskLen,
                                             const char *callback,
                                             int callbackLen) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->UpdateFinish(pageId, task, taskLen, callback, callbackLen);
}

int AndroidBridgeInMultiSo::CallRefreshFinish(const char *pageId,
                                              const char *task,
                                              const char *callback) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RefreshFinish(pageId, task, callback);
}

int AndroidBridgeInMultiSo::CallAddEvent(const char *pageId, const char *ref,
                                         const char *event) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->AddEvent(pageId, ref, event);
}

int AndroidBridgeInMultiSo::CallRemoveEvent(const char *pageId, const char *ref,
                                            const char *event) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RemoveEvent(pageId, ref, event);
}

int AndroidBridgeInMultiSo::CallCreateBody(
    const char *pageId, const char *componentType, const char *ref,
    std::map<std::string, std::string> *styles,
    std::map<std::string, std::string> *attributes,
    std::set<std::string> *events, const WXCoreMargin &margins,
    const WXCorePadding &paddings, const WXCoreBorderWidth &borders) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CreateBody(pageId, componentType, ref, styles, attributes, events,
                   margins, paddings, borders);
}

int AndroidBridgeInMultiSo::CallAddElement(
    const char *pageId, const char *componentType, const char *ref, int &index,
    const char *parentRef, std::map<std::string, std::string> *styles,
    std::map<std::string, std::string> *attributes,
    std::set<std::string> *events, const WXCoreMargin &margins,
    const WXCorePadding &paddings, const WXCoreBorderWidth &borders,
    bool willLayout) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->AddElement(pageId, componentType, ref, index, parentRef, styles,
                   attributes, events, margins, paddings, borders, willLayout);
}

int AndroidBridgeInMultiSo::CallLayout(const char *pageId, const char *ref,
                                       int top, int bottom, int left, int right,
                                       int height, int width, int index) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->Layout(pageId, ref, top, bottom, left, right, height, width, index);
}

int AndroidBridgeInMultiSo::CallUpdateStyle(
    const char *pageId, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style,
    std::vector<std::pair<std::string, std::string>> *margin,
    std::vector<std::pair<std::string, std::string>> *padding,
    std::vector<std::pair<std::string, std::string>> *border) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->UpdateStyle(pageId, ref, style, margin, padding, border);
}

int AndroidBridgeInMultiSo::CallUpdateAttr(
    const char *pageId, const char *ref,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->UpdateAttr(pageId, ref, attrs);
}

int AndroidBridgeInMultiSo::CallCreateFinish(const char *pageId) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CreateFinish(pageId);
}

int AndroidBridgeInMultiSo::CallRenderSuccess(const char *pageId) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RenderSuccess(pageId);
}

int AndroidBridgeInMultiSo::CallRemoveElement(const char *pageId,
                                              const char *ref) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RemoveElement(pageId, ref);
}

int AndroidBridgeInMultiSo::CallMoveElement(const char *pageId, const char *ref,
                                            const char *parentRef, int index) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->MoveElement(pageId, ref, parentRef, index);
}

void AndroidBridgeInMultiSo::PostMessage(const char *vm_id, const char *data,
                                         int dataLength) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->PostMessage(vm_id, data, dataLength);
}

void AndroidBridgeInMultiSo::DispatchMessage(const char *client_id,
                                             const char *data,
                                             int dataLength,
                                             const char *callback,
                                             const char *vm_id) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->DispatchMessage(client_id, data, dataLength, callback, vm_id);
}

int AndroidBridgeInMultiSo::CallAppendTreeCreateFinish(const char *pageId,
                                                       const char *ref) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->AppendTreeCreateFinish(pageId, ref);
}

int AndroidBridgeInMultiSo::CallHasTransitionPros(
    const char *pageId, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->HasTransitionPros(pageId, ref, style);
}

WXCoreSize AndroidBridgeInMultiSo::CallInvokeMeasureFunction(
    const char *page_id, long render_ptr, float width, int width_measure_mode,
    float height, int height_measure_mode) {
  return WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->InvokeMeasureFunction(page_id, render_ptr, width, width_measure_mode,
                              height, height_measure_mode);
}

void AndroidBridgeInMultiSo::CallInvokeLayoutBefore(const char *page_id,
                                                    long render_ptr) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->InvokeLayoutBefore(page_id, render_ptr);
}

void AndroidBridgeInMultiSo::CallInvokeLayoutAfter(const char *page_id,
                                                   long render_ptr, float width,
                                                   float height) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->InvokeLayoutAfter(page_id, render_ptr, width, height);
}

}  // namespace WeexCore