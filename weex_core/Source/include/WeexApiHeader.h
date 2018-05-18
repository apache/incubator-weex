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
// Created by Darin on 12/02/2018.
//

#ifndef WEEXV8_WEEXAPIHEADER_H
#define WEEXV8_WEEXAPIHEADER_H

#include <jni.h>
#include <vector>
#include <IPC/IPCResult.h>

struct WeexString {
    uint32_t length;
    uint16_t content[1];
};

struct WeexByteArray {
    uint32_t length;
    char content[1];
};

struct WeexJSResult{
    char* data = nullptr;
    int  length = 0;
    bool fromMalloc = false;
    bool fromNew = false;
};

inline void WeexJSResultDataFree(WeexJSResult& result){
    if(result.fromMalloc){
        free(result.data);
        return;
    }
    if(result.fromNew){
        delete[] result.data;
        return;
    }
}




typedef void (*FuncSetJSVersion)(const char *jsVersion);

typedef void (*FuncReportException)(const char *pageId, const char *func,
                                    const char *exception_string);

typedef void (*FuncCallNative)(const char *pageId, const char *task, const char *callback);

typedef std::unique_ptr<IPCResult> (*FuncCallNativeModule)(const char *pageId, const char *module,
                                                           const char *method,
                                                           const char *arguments,
                                                           int argumentsLength,
                                                           const char *options,
                                                           int optionsLength);

typedef void (*FuncCallNativeComponent)(const char *pageId, const char *ref, const char *method,
                                        const char *arguments, int argumentsLength, const char *options, int optionsLength);

typedef void (*FuncCallAddElement)(const char *pageId, const char *parentRef, const char *domStr,
                                   const char *index_cstr);

typedef void (*FuncSetTimeout)(const char *callbackId, const char *time);

typedef void (*FuncCallNativeLog)(const char *str_array);

typedef void (*FuncCallCreateBody)(const char *pageId, const char *domStr);

typedef int (*FuncCallUpdateFinish)(const char *pageId, const char *task, const char *callback);

typedef void (*FuncCallCreateFinish)(const char *pageId);

typedef int (*FuncCallRefreshFinish)(const char *pageId, const char *task, const char *callback);

typedef void (*FuncCallUpdateAttrs)(const char *pageId, const char *ref, const char *data);

typedef void (*FuncCallUpdateStyle)(const char *pageId, const char *ref, const char *data);

typedef void (*FuncCallRemoveElement)(const char *pageId, const char *ref);

typedef void (*FuncCallMoveElement)(const char *pageId, const char *ref, const char *parentRef,
                                    int index);

typedef void (*FuncCallAddEvent)(const char *pageId, const char *ref, const char *event);

typedef void (*FuncCallRemoveEvent)(const char *pageId, const char *ref, const char *event);

typedef int (*FuncSetInterval)(const char *pageId, const char *callbackId, const char *_time);

typedef void (*FuncClearInterval)(const char *pageId, const char *callbackId);

typedef const char *(*FuncCallGCanvasLinkNative)(const char *pageId, int type, const char *args);

typedef const char *(*FuncT3dLinkNative)(int type, const char *args);

typedef void (*FuncCallHandlePostMessage)(const char *vimId, const char *data);

typedef void
(*FuncCallDIspatchMessage)(const char *clientId, const char *data, const char *callback,
                           const char *vmId);


typedef struct WeexCoreJsCoreApiFunctions {
    FuncSetJSVersion funcSetJSVersion;
    FuncReportException funcReportException;
    FuncCallNative funcCallNative;
    FuncCallNativeModule funcCallNativeModule;
    FuncCallNativeComponent funcCallNativeComponent;
    FuncCallAddElement funcCallAddElement;
    FuncSetTimeout funcSetTimeout;
    FuncCallNativeLog funcCallNativeLog;
    FuncCallCreateBody funcCallCreateBody;
    FuncCallUpdateFinish funcCallUpdateFinish;
    FuncCallCreateFinish funcCallCreateFinish;
    FuncCallRefreshFinish funcCallRefreshFinish;
    FuncCallUpdateAttrs funcCallUpdateAttrs;
    FuncCallUpdateStyle funcCallUpdateStyle;
    FuncCallRemoveElement funcCallRemoveElement;
    FuncCallMoveElement funcCallMoveElement;
    FuncCallAddEvent funcCallAddEvent;
    FuncCallRemoveEvent funcCallRemoveEvent;
    FuncSetInterval funcSetInterval;
    FuncClearInterval funcClearInterval;
    FuncCallGCanvasLinkNative funcCallGCanvasLinkNative;
    FuncT3dLinkNative funcT3dLinkNative;
    FuncCallHandlePostMessage funcCallHandlePostMessage;
    FuncCallDIspatchMessage funcCallDIspatchMessage;
} WEEX_CORE_JS_API_FUNCTIONS;

typedef struct InitFrameworkParams {
    WeexByteArray *type;
    WeexByteArray *value;
} INIT_FRAMEWORK_PARAMS;


enum class ParamsType {
    INT32 = 1,
    INT64,
    FLOAT,
    DOUBLE,
    JSONSTRING,
    STRING,
    BYTEARRAY, /* terminated with zero. */
    VOID,
    JSUNDEFINED,
    END,
};


typedef union ExecJsParamValue {
    int32_t int32Value;
    int64_t int64Value;
    float floatValue;
    double doubleValue;
    WeexString *string;
    WeexByteArray *byteArray;
} EXEC_JS_PARAM_VALUE;

typedef struct ValueWithType {
    ParamsType type;
    EXEC_JS_PARAM_VALUE value;
} VALUE_WITH_TYPE;


typedef int (*FuncInitFramework)(const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> params);


typedef int (*FuncInitAppFramework)(const char *instanceId, const char *appFramework,
                                    std::vector<INIT_FRAMEWORK_PARAMS *> params);

typedef int (*FuncCreateAppContext)(const char *instanceId, const char *jsBundle);

typedef char *(*FuncExeJSOnAppWithResult)(const char *instanceId, const char *jsBundle);

typedef int (*FuncCallJSOnAppContext)(const char *instanceId, const char *func,
                                      std::vector<VALUE_WITH_TYPE *> params);

typedef int (*FuncDestroyAppContext)(const char *instanceId);

typedef int (*FuncExeJsService)(const char *source);

typedef int (*FuncExeCTimeCallback)(const char *source);

typedef int (*FuncExeJS)(const char *instanceId, const char *nameSpace, const char *func,
                         std::vector<VALUE_WITH_TYPE *> params);

typedef WeexJSResult(*FuncExeJSWithResult)(const char *instanceId, const char *nameSpace,
                                     const char *func,
                                     std::vector<VALUE_WITH_TYPE *> params);

typedef int (*FuncCreateInstance)(const char *instanceId, const char *func, const char *script,
                                  const char *opts,
                                  const char *initData, const char *extendsApi);

typedef char *(*FuncExeJSOnInstance)(const char *instanceId, const char *script);

typedef int (*FuncDestroyInstance)(const char *instanceId);

typedef int (*FuncUpdateGlobalConfig)(const char *config);

typedef struct WeexCoreJsServerApiFunctions {
    FuncInitFramework funcInitFramework;
    FuncInitAppFramework funcInitAppFramework;
    FuncCreateAppContext funcCreateAppContext;
    FuncExeJSOnAppWithResult funcExeJSOnAppWithResult;
    FuncCallJSOnAppContext funcCallJSOnAppContext;
    FuncDestroyAppContext funcDestroyAppContext;
    FuncExeJsService funcExeJsService;
    FuncExeCTimeCallback funcExeCTimeCallback;
    FuncExeJS funcExeJS;
    FuncExeJSWithResult funcExeJSWithResult;
    FuncCreateInstance funcCreateInstance;
    FuncExeJSOnInstance funcExeJSOnInstance;
    FuncDestroyInstance funcDestroyInstance;
    FuncUpdateGlobalConfig funcUpdateGlobalConfig;
} WEEX_CORE_JS_SERVER_API_FUNCTIONS;

#endif //WEEXV8_WEEXAPIHEADER_H
