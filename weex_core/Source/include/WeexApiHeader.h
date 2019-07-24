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

#include <vector>
#include <set>
#include <map>
#include "WeexApiValue.h"

#ifdef OS_ANDROID
#include <jni.h>
#include "third_party/IPC/IPCResult.h"
#endif

namespace WeexCore {
    class WXCoreMargin;
    class WXCorePadding;
    class WXCoreBorderWidth;
    class WXCoreSize;
}  // namespace WeexCore
using namespace WeexCore;

class WeexJSResult{
public:
    std::unique_ptr<char[]> data;
    int length = 0;
    WeexJSResult() : data(nullptr), length(0) {}
    WeexJSResult(std::unique_ptr<char[]> d, int l) : data(std::move(d)), length(l) {}
};

typedef struct InitFrameworkParams {
    WeexByteArray *type;
    WeexByteArray *value;
} INIT_FRAMEWORK_PARAMS;

#ifdef OS_ANDROID

typedef void (*FuncSetJSVersion)(const char *jsVersion);

typedef void (*FuncReportException)(const char *pageId, const char *func, const char *exception_string);
typedef void (*FuncReportServerCrash)(const char *instance_id);
typedef void (*FuncReportNativeInitStatus)(const char *status_code, const char *error_msg);

typedef void (*FuncCallNative)(const char *pageId, const char *task, const char *callback);

typedef std::unique_ptr<ValueWithType> (*FuncCallNativeModule)(const char *pageId, const char *module, const char *method,
                                                           const char *arguments, int argumentsLen, const char *options, int optionsLen);

typedef void (*FuncCallNativeComponent)(const char *pageId, const char *ref,
                                        const char *method, const char *arguments, int argumentsLen, const char *options, int optionsLen);

typedef void (*FuncCallAddElement)(const char *pageId, const char *parentRef, const char *domStr, int domLen,
                                   const char *index_cstr);

typedef void (*FuncSetTimeout)(const char *callbackId, const char *time);

typedef void (*FuncCallNativeLog)(const char *str_array);

typedef void (*FuncCallCreateBody)(const char *pageId, const char *domStr, int domStrLen);

typedef int (*FuncCallUpdateFinish)(const char *pageId, const char *task,int taskLen, const char *callback, int callbackLen);

typedef void (*FuncCallCreateFinish)(const char *pageId);

typedef int (*FuncCallRefreshFinish)(const char *pageId, const char *task, const char *callback);

typedef void (*FuncCallUpdateAttrs)(const char *pageId, const char *ref, const char *data, int dataLen);

typedef void (*FuncCallUpdateStyle)(const char *pageId, const char *ref, const char *data, int dataLen);

typedef void (*FuncCallRemoveElement)(const char *pageId, const char *ref);

typedef void (*FuncCallMoveElement)(const char *pageId, const char *ref, const char *parentRef, int index);

typedef void (*FuncCallAddEvent)(const char *pageId, const char *ref, const char *event);

typedef void (*FuncCallRemoveEvent)(const char *pageId, const char *ref, const char *event);

typedef int (*FuncSetInterval)(const char *pageId, const char *callbackId, const char *_time);

typedef void (*FuncClearInterval)(const char *pageId, const char *callbackId);

typedef const char *(*FuncCallGCanvasLinkNative)(const char *pageId, int type, const char *args);

typedef const char *(*FuncT3dLinkNative)(int type, const char *args);

typedef void (*FuncCallHandlePostMessage)(const char *vimId, const char *data, int dataLength);

typedef void
(*FuncCallDIspatchMessage)(const char *clientId, const char *data, int dataLength, const char *callback, const char *vmId);

typedef std::unique_ptr<WeexJSResult> (*FuncCallDispatchMessageSync)(
    const char *clientId, const char *data, int dataLength, const char *vmId);

typedef void
(*FuncOnReceivedResult)(long callback_id, std::unique_ptr<WeexJSResult>& result);
typedef void
(*FuncUpdateComponentData)(const char* page_id, const char* cid, const char* json_data);


typedef bool
(*FuncLog)(int level, const char *tag,
           const char *file,
           unsigned long line,
           const char *log);

typedef struct FunctionsExposedByCore {
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
    FuncCallDispatchMessageSync funcCallDispatchMessageSync;
    FuncOnReceivedResult  funcOnReceivedResult;
    FuncUpdateComponentData funcUpdateComponentData;
    FuncLog funcLog;
} FunctionsExposedByCore;

typedef void (*FuncCallSetJSVersion)(const char* version);
typedef int (*CallNative)(const char *pageId, const char *task, const char *callback);
typedef int (*FuncCreateBody)(const char* page_id, const char *component_type, const char* ref,
                              std::map<std::string, std::string> *styles,
                              std::map<std::string, std::string> *attributes,
                              std::set<std::string> *events,
                              const WXCoreMargin &margins,
                              const WXCorePadding &paddings,
                              const WXCoreBorderWidth &borders);
typedef int (*FuncCreateFinish)(const char *pageId);
typedef int (*FuncAddElement)(const char* page_id, const char *component_type, const char* ref,
                              int &index, const char* parentRef,
                              std::map<std::string, std::string> *styles,
                              std::map<std::string, std::string> *attributes,
                              std::set<std::string> *events,
                              const WXCoreMargin &margins,
                              const WXCorePadding &paddings,
                              const WXCoreBorderWidth &borders,
                              bool willLayout);
typedef int (*FuncRemoveElement)(const char *pageId, const char *ref);
typedef int (*FuncMoveElement)(const char *pageId, const char *ref, const char *parentRef, int index);
typedef int (*FuncAddEvent)(const char *pageId, const char *ref, const char *event);
typedef int (*FuncRemoveEvent)(const char *pageId, const char *ref, const char *event);
typedef int (*FuncUpdateStyle)(const char* page_id, const char* ref,
                               std::vector<std::pair<std::string, std::string>> *style,
                               std::vector<std::pair<std::string, std::string>> *margin,
                               std::vector<std::pair<std::string, std::string>> *padding,
                               std::vector<std::pair<std::string, std::string>> *border);
typedef int (*FuncUpdateAttr)(const char* page_id, const char* ref,
                              std::vector<std::pair<std::string, std::string>> *attrs);
typedef int (*FuncLayout)(const char *page_id, const char *ref,
                          int top, int bottom, int left, int right,
                          int height, int width, int index);
typedef int (*FuncAppendTreeCreateFinish)(const char* page_id, const char* ref);
typedef int (*FuncHasTransitionPros)(const char* page_id, const char* ref,
                                     std::vector<std::pair<std::string, std::string>> *style);
typedef WXCoreSize (*InvokeMeasureFunction)(const char* page_id, long render_ptr, float width, int width_measure_mode, float height, int height_measure_mode);
typedef void (*InvokeLayoutBefore)(const char* page_id, long render_ptr);
typedef void (*InvokeLayoutAfter)(const char* page_id, long render_ptr, float width, float height);
typedef struct PlatformExposeFunctions {
    FuncCallSetJSVersion set_js_version;
    FuncReportException report_exception;
    FuncReportServerCrash report_server_crash;
    FuncReportNativeInitStatus report_native_Init_status;
    CallNative call_native;
    FuncCallNativeModule call_native_module;
    FuncCallNativeComponent call_native_component;
    FuncAddElement add_element;
    FuncSetTimeout set_timeout;
    FuncCallNativeLog native_log;
    FuncCreateBody create_body;
    FuncCallUpdateFinish update_finish;
    FuncCreateFinish create_finish;
    FuncCreateFinish render_success;
    FuncCallRefreshFinish refresh_finish;
    FuncUpdateAttr update_attrs;
    FuncUpdateStyle update_style;
    FuncRemoveElement remove_element;
    FuncMoveElement move_element;
    FuncAddEvent add_event;
    FuncRemoveEvent remove_event;
    FuncCallHandlePostMessage post_message;
    FuncCallDIspatchMessage dispatch_message;
    FuncLayout layout;
    FuncAppendTreeCreateFinish append_tree_create_finish;
    FuncHasTransitionPros has_transition_pros;
    InvokeMeasureFunction invoke_measure_function;
    InvokeLayoutBefore invoke_layout_before;
    InvokeLayoutAfter invoke_layout_after;
} PlatformExposeFunctions;


typedef int (*FuncInitFramework)(const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params);


typedef int (*FuncInitAppFramework)(const char *instanceId, const char *appFramework,
                                    std::vector<INIT_FRAMEWORK_PARAMS *> &params);

typedef int (*FuncCreateAppContext)(const char *instanceId, const char *jsBundle);

typedef std::unique_ptr<WeexJSResult> (*FuncExeJSOnAppWithResult)(const char *instanceId, const char *jsBundle);

typedef int (*FuncCallJSOnAppContext)(const char *instanceId, const char *func, std::vector<VALUE_WITH_TYPE *> &params);

typedef int (*FuncDestroyAppContext)(const char *instanceId);

typedef int (*FuncExeJsService)(const char *source);

typedef int (*FuncExeCTimeCallback)(const char *source);

typedef int (*FuncExeJS)(const char *instanceId, const char *nameSpace, const char *func,
                         std::vector<VALUE_WITH_TYPE *> &params);

typedef std::unique_ptr<WeexJSResult>(*FuncExeJSWithResult)(const char *instanceId, const char *nameSpace, const char *func,
                                           std::vector<VALUE_WITH_TYPE *> &params);

typedef void(*FuncExeJSWithResultId)(const char *instanceId, const char *nameSpace, const char *func,
                                           std::vector<VALUE_WITH_TYPE *> &params, long callback_id);

typedef int (*FuncCreateInstance)(const char *instanceId, const char *func, const char *script, const char *opts,
                                  const char *initData, const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params);

typedef std::unique_ptr<WeexJSResult> (*FuncExeJSOnInstance)(const char *instanceId, const char *script,int type);

typedef int (*FuncDestroyInstance)(const char *instanceId);

typedef int (*FuncUpdateGlobalConfig)(const char *config);

typedef int (*FuncUpdateInitFrameworkParams)(const std::string& key, const std::string& value, const std::string& desc);

typedef void (*FuncSetLogType)(const int logLevel, const bool isPerf);

typedef struct FunctionsExposedByJS {
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
    FuncExeJSWithResultId funcExeJSWithResultId;
    FuncCreateInstance funcCreateInstance;
    FuncExeJSOnInstance funcExeJSOnInstance;
    FuncDestroyInstance funcDestroyInstance;
    FuncUpdateGlobalConfig funcUpdateGlobalConfig;
    FuncUpdateInitFrameworkParams funcUpdateInitFrameworkParams;
    FuncSetLogType funcSetLogType;
} FunctionsExposedByJS;


typedef void (*SetDefaultHeightAndWidthIntoRootDomCallback)(const std::string &instance_id,
                                                    float default_width,
                                                    float default_height,
                                                    bool is_width_wrap_content,
                                                    bool is_height_wrap_content);
typedef void (*OnInstanceCloseCallback)(const std::string &instance_id);
typedef void (*SetStyleWidthCallback)(const std::string &instance_id, const std::string& render_ref, float width);
typedef void (*SetStyleHeightCallback)(const std::string &instance_id, const std::string& render_ref, float height);
typedef void (*SetMarginCallback)(const std::string &instance_id, const std::string& render_ref, int edge, float value);
typedef void (*SetPaddingCallback)(const std::string &instance_id, const std::string& render_ref, int edge, float value);
typedef void (*SetPositionCallback)(const std::string &instance_id, const std::string& render_ref, int edge, float value);
typedef void (*MarkDirtyCallback)(const std::string &instance_id, const std::string& render_ref);
typedef void (*SetViewPortWidthCallback)(const std::string &instance_id, float width);
typedef void (*ForceLayoutCallback)(const std::string &instance_id);
typedef bool (*NotifyLayoutCallback)(const std::string &instance_id);
typedef std::vector<int64_t> (*GetFirstScreenRenderTimeCallback)(const std::string &instance_id);
typedef std::vector<int64_t> (*GetRenderFinishTimeCallback)(const std::string &instance_id);
typedef void (*SetRenderContainerWrapContentCallback)(const std::string &instance_id, bool wrap);
typedef void (*BindMeasurementToRenderObjectCallback)(long ptr);
typedef void (*RegisterCoreEnvCallback)(const std::string& key, const std::string& value);

typedef long (*GetRenderObjectCallback)(const std::string &instance_id, const std::string& render_ref);
typedef void (*UpdateRenderObjectStyleCallback)(long render_ptr, const std::string& key, const std::string& value);
typedef void (*UpdateRenderObjectAttrCallback)(long render_ptr, const std::string& key, const std::string& value);
typedef long (*CopyRenderObjectCallback)(long render_ptr);
typedef void (*SetMeasureFunctionAdapter)();
typedef void (*SetPlatform)(const std::string& platformName);
typedef void (*SetDeviceWidthAndHeight)(float width, float height);
typedef void (*AddOption)(const std::string& key, const std::string& value);
typedef struct CoreSideFunctionsOfPlatformBridge {
    SetDefaultHeightAndWidthIntoRootDomCallback set_default_height_and_width_into_root_dom;
    OnInstanceCloseCallback on_instance_close;
    SetStyleWidthCallback set_style_width;
    SetStyleHeightCallback set_style_height;
    SetMarginCallback set_margin;
    SetPaddingCallback set_padding;
    SetPositionCallback set_position;
    MarkDirtyCallback mark_dirty;
    SetViewPortWidthCallback set_view_port_width;
    ForceLayoutCallback force_layout;
    NotifyLayoutCallback notify_layout;
    GetFirstScreenRenderTimeCallback get_first_screen_render_time;
    GetRenderFinishTimeCallback get_render_finish_time;
    SetRenderContainerWrapContentCallback set_render_container_wrap_content;
    BindMeasurementToRenderObjectCallback bind_measurement_to_render_object;
    RegisterCoreEnvCallback register_core_env;
    GetRenderObjectCallback get_render_object;
    UpdateRenderObjectStyleCallback update_render_object_style;
    UpdateRenderObjectAttrCallback update_render_object_attr;
    CopyRenderObjectCallback copy_render_object;
    SetMeasureFunctionAdapter set_measure_function_adapter;
    SetPlatform set_platform;
    SetDeviceWidthAndHeight set_device_width_and_height;
    AddOption add_option;

    FuncInitFramework init_framework;
    FuncInitAppFramework init_app_framework;
    FuncCreateAppContext create_app_context;
    FuncExeJSOnAppWithResult exec_js__on_app_with_result;
    FuncCallJSOnAppContext call_js_on_app_context;
    FuncDestroyAppContext destroy_app_context;
    FuncExeJsService exec_js_service;
    FuncExeCTimeCallback exec_time_callback;
    FuncExeJS exec_js;
    FuncExeJSWithResult exec_js_with_result;
    FuncCreateInstance create_instance;
    FuncExeJSOnInstance exec_js_on_instance;
    FuncDestroyInstance destroy_instance;
    FuncUpdateGlobalConfig update_global_config;
} CoreSideFunctionsOfPlatformBridge;
#endif

#endif //WEEXV8_WEEXAPIHEADER_H
