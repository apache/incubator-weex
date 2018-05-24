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
// Created by Darin on 13/02/2018.
//

#ifndef WEEXV8_WEEXJSCOREAPI_H
#define WEEXV8_WEEXJSCOREAPI_H

#include "jni.h"
#include "IPCResult.h"
#include "WeexJSCoreApi.h"

extern "C" {

void _setJSVersion(const char *jsVersion);

void _reportException(const char *pageId, const char *func, const char *exception_string);

void _callNative(const char *pageId, const char *task, const char *callback);


std::unique_ptr<IPCResult> _callNativeModule(const char *pageId, const char *module, const char *method,
                       const char *arguments, int argumentsLength, const char *options, int optionsLength);

void _callNativeComponent(const char *pageId, const char *ref,
                          const char *method,
                          const char *arguments, int argumentsLength, const char *options, int optionsLength);

void _callAddElement(const char *pageId, const char *parentRef, const char *domStr,
                     const char *index_cstr);

void _setTimeout(const char *callbackId, const char *time);

void _callNativeLog(const char *str_array);

void _callCreateBody(const char *pageId, const char *domStr);

int _callUpdateFinish(const char *pageId, const char *task, const char *callback);

void _callCreateFinish(const char *pageId);

int _callRefreshFinish(const char *pageId, const char *task, const char *callback);

void _callUpdateAttrs(const char *pageId, const char *ref, const char *data);

void _callUpdateStyle(const char *pageId, const char *ref, const char *data);

void _callRemoveElement(const char *pageId, const char *ref);

void _callMoveElement(const char *pageId, const char *ref, const char *parentRef, int index);

void _callAddEvent(const char *pageId, const char *ref, const char *event);

void _callRemoveEvent(const char *pageId, const char *ref, const char *event);

int _setInterval(const char *pageId, const char *callbackId, const char *_time);

void _clearInterval(const char *pageId, const char *callbackId);

const char *_callGCanvasLinkNative(const char *pageId, int type, const char *args);

const char *_t3dLinkNative(int type, const char *args);

void callHandlePostMessage(const char *vimId, const char *data);

void
callDIspatchMessage(const char *clientId, const char *data, const char *callback, const char *vmId);
};
#endif //WEEXV8_WEEXJSCOREAPI_H
