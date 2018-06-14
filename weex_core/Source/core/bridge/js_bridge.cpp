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

#include "js_bridge.h"
#include <core/manager/weex_core_manager.h>
#include <core/render/manager/render_manager.h>

namespace WeexCore {
    void JSBridge::onSetJSVersion(const char *jsVersion) {
        WeexCoreManager::getInstance()->getPlatformBridge()->setJSVersion(jsVersion);
    }

    void JSBridge::onReportException(const char *pageId, const char *func, const char *exception_string) {
        WeexCoreManager::getInstance()->getPlatformBridge()->reportException(pageId, func, exception_string);
    }

    void JSBridge::onCallNative(const char *pageId, const char *task, const char *callback) {
        if (pageId == nullptr || task == nullptr)
            return;
#if JSAPI_LOG
        LOGD("[ExtendJSApi] handleCallNative >>>> pageId: %s, task: %s", pageId, task);
#endif
        if (strcmp(task, "[{\"module\":\"dom\",\"method\":\"createFinish\",\"args\":[]}]") == 0) {
            RenderManager::GetInstance()->CreateFinish(pageId) ? 0 : -1;
        } else {
            WeexCoreManager::getInstance()->getPlatformBridge()->callNative(pageId, task, callback);
        }
    }


    void *JSBridge::onCallNativeModule(const char *pageId, const char *module, const char *method,
                                       const char *arguments, int argumentsLength, const char *options,
                                       int optionsLength) {

        RenderManager::GetInstance()->CallNativeModule(pageId, module, method, arguments, argumentsLength,
                                                       options, optionsLength);

        return WeexCoreManager::getInstance()->getPlatformBridge()->callNativeModule(
                pageId, module, method, arguments,
                argumentsLength, options,
                optionsLength
        );
    }

    void JSBridge::onCallNativeComponent(const char *pageId, const char *ref,
                                         const char *method,
                                         const char *arguments, int argumentsLength, const char *options,
                                         int optionsLength) {
        if (pageId != nullptr && ref != nullptr && method != nullptr) {

#if JSAPI_LOG
            LOGD("[ExtendJSApi] handleCallNativeComponent >>>> pageId: %s, ref: %s, method: %s, arg: %s, opt: %s",
         pageId, ref, method, argString, optString);
#endif
            WeexCoreManager::getInstance()->getPlatformBridge()->callNativeComponent(
                    pageId, ref, method, arguments, argumentsLength, options, optionsLength
            );
        }
    }


    void JSBridge::onCallAddElement(const char *pageId, const char *parentRef, const char *domStr,
                                    const char *index_cstr) {
        const char *indexChar = index_cstr == nullptr ? "\0" : index_cstr;

        int index = atoi(indexChar);
        if (pageId == nullptr || parentRef == nullptr || domStr == nullptr || index < -1)
            return;

#if JSAPI_LOG

        std::string log = "";
  log.append("pageId: ").append(pageId).append(", parentRef: ").append(parentRef).append(", domStr: ").append(domStr);
  int log_index = 0;
  int maxLength = 800;
  std::string sub;
  while (log_index < log.length()) {
    if (log.length() <= log_index + maxLength) {
      sub = log.substr(log_index);
    } else {
      sub = log.substr(log_index, maxLength);
    }


    if (log_index == 0)
      LOGD("[ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());
    else
      LOGD("      [ExtendJSApi] functionCallAddElement >>>> %s", sub.c_str());

    log_index += maxLength;
  }
#endif

        RenderManager::GetInstance()->AddRenderObject(pageId, parentRef, index, domStr);
    }

    void JSBridge::onSetTimeout(const char *callbackId, const char *time) {
        WeexCoreManager::getInstance()->getPlatformBridge()->setTimeout(callbackId, time);
    }

    void JSBridge::onCallNativeLog(const char *str_array) {
        WeexCoreManager::getInstance()->getPlatformBridge()->callNativeLog(str_array);
    }


    void JSBridge::onCallCreateBody(const char *pageId, const char *domStr) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallCreateBody >>>> pageId: %s, domStr: %s", pageId, domStr);
#endif

        RenderManager::GetInstance()->CreatePage(pageId, domStr) ? 0 : -1;
    }

    int JSBridge::onCallUpdateFinish(const char *pageId, const char *task, const char *callback) {
        WeexCoreManager::getInstance()->getPlatformBridge()->callUpdateFinish(pageId, task, callback);
    }

    void JSBridge::onCallCreateFinish(const char *pageId) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallCreateFinish >>>> pageId: %s", pageId);
#endif

        RenderManager::GetInstance()->CreateFinish(pageId);
    }

    int JSBridge::onCallRefreshFinish(const char *pageId, const char *task, const char *callback) {
        if (pageId == nullptr)
            return -1;
        return WeexCoreManager::getInstance()->getPlatformBridge()->callRefreshFinish(pageId, task, callback);
    }

    void JSBridge::onCallUpdateAttrs(const char *pageId, const char *ref, const char *data) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallUpdateAttrs >>>> pageId: %s, ref: %s, data: %s", pageId,
       ref, data);
#endif

        RenderManager::GetInstance()->UpdateAttr(pageId, ref, data);
    }

    void JSBridge::onCallUpdateStyle(const char *pageId, const char *ref, const char *data) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallUpdateStyle >>>> pageId: %s, ref: %s, data: %s", pageId,
       ref, data);
#endif
        RenderManager::GetInstance()->UpdateStyle(pageId, ref, data);
    }


    void JSBridge::onCallRemoveElement(const char *pageId, const char *ref) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s", pageId,
       ref);
#endif
        RenderManager::GetInstance()->RemoveRenderObject(pageId, ref);
    }

    void JSBridge::onCallMoveElement(const char *pageId, const char *ref, const char *parentRef, int index) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallRemoveElement >>>> pageId: %s, ref: %s, parentRef: %s, index: %d",
       pageId, ref, parentRef, index);
#endif
        RenderManager::GetInstance()->MoveRenderObject(pageId, ref, parentRef, index);
    }

    void JSBridge::onCallAddEvent(const char *pageId, const char *ref, const char *event) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallAddEvent >>>> pageId: %s, ref: %s, event: %s", pageId,
       ref, event);
#endif
        RenderManager::GetInstance()->AddEvent(pageId, ref, event);
    }

    void JSBridge::onCallRemoveEvent(const char *pageId, const char *ref, const char *event) {
#if JSAPI_LOG
        LOGD("[ExtendJSApi] functionCallRemoveEvent >>>> pageId: %s, ref: %s, event: %s", pageId,
       ref, event);
#endif
        RenderManager::GetInstance()->RemoveEvent(pageId, ref, event);
    }

    int JSBridge::onSetInterval(const char *pageId, const char *callbackId, const char *_time) {
        return (atoi(pageId) << 16) | (atoi(callbackId));
    }

    void JSBridge::onClearInterval(const char *pageId, const char *callbackId) {
        return;
    }
}