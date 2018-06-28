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

#ifndef WEEXCORE_JS_BRIDGE_H
#define WEEXCORE_JS_BRIDGE_H

namespace WeexCore {
    class JSBridge {
    public:
        JSBridge(){};

        ~JSBridge(){};

    public:

        void onSetJSVersion(const char *jsVersion);

        void onReportException(const char *pageId, const char *func, const char *exception_string);

        void onCallNative(const char *pageId, const char *task, const char *callback);


        void *onCallNativeModule(const char *pageId, const char *module, const char *method,
                                 const char *arguments, int argumentsLength, const char *options, int optionsLength);

        void onCallNativeComponent(const char *pageId, const char *ref,
                                   const char *method,
                                   const char *arguments, int argumentsLength, const char *options, int optionsLength);

        void onCallAddElement(const char *pageId, const char *parentRef, const char *domStr,
                              const char *index_cstr);

        void onSetTimeout(const char *callbackId, const char *time);

        void onCallNativeLog(const char *str_array);

        void onCallCreateBody(const char *pageId, const char *domStr);

        int onCallUpdateFinish(const char *pageId, const char *task, const char *callback);

        void onCallCreateFinish(const char *pageId);

        int onCallRefreshFinish(const char *pageId, const char *task, const char *callback);

        void onCallUpdateAttrs(const char *pageId, const char *ref, const char *data);

        void onCallUpdateStyle(const char *pageId, const char *ref, const char *data);

        void onCallRemoveElement(const char *pageId, const char *ref);

        void onCallMoveElement(const char *pageId, const char *ref, const char *parentRef, int index);

        void onCallAddEvent(const char *pageId, const char *ref, const char *event);

        void onCallRemoveEvent(const char *pageId, const char *ref, const char *event);

        int onSetInterval(const char *pageId, const char *callbackId, const char *_time);

        void onClearInterval(const char *pageId, const char *callbackId);
    };
}

#endif //WEEXCORE_JS_BRIDGE_H