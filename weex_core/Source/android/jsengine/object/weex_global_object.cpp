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
// Created by Darin on 11/02/2018.
//

#include "android/jsengine/object/weex_global_object.h"

#include <sys/stat.h>
#include <base/time_calculator.h>
#include "android/jsengine/object/weex_env.h"
#include "android/jsengine/object/weex_console_object.h"
#include "android/jsengine/task/timer_task.h"
#include "android/jsengine/task/timer_queue.h"
#include "android/jsengine/weex_jsc_utils.h"
#include "base/utils/log_utils.h"
#include "core/bridge/script_bridge.h"
#include "include/wtf/text/Base64.h"

#define WX_GLOBAL_CONFIG_KEY "global_switch_config"
//#define GET_CHARFROM_UNIPTR(str) (str) == nullptr ? nullptr : (reinterpret_cast<const char*>((str).get()))
using namespace JSC;

static bool isGlobalConfigStartUpSet = false;

//extern WEEX_CORE_JS_API_FUNCTIONS *weex_core_js_api_functions;

#define JSFUNCTION static EncodedJSValue JSC_HOST_CALL

JSFUNCTION functionGCAndSweep(ExecState *);

JSFUNCTION functionCallNative(ExecState *);

JSFUNCTION functionCallNativeModule(ExecState *);

JSFUNCTION functionCallNativeComponent(ExecState *);

JSFUNCTION functionCallAddElement(ExecState *);

JSFUNCTION functionSetTimeoutNative(ExecState *);

JSFUNCTION functionNativeLog(ExecState *);

JSFUNCTION functionNotifyTrimMemory(ExecState *);

JSFUNCTION functionMarkupState(ExecState *);

JSFUNCTION functionAtob(ExecState *);

JSFUNCTION functionBtoa(ExecState *);

JSFUNCTION functionCallCreateBody(ExecState *);

JSFUNCTION functionCallUpdateFinish(ExecState *);

JSFUNCTION functionCallCreateFinish(ExecState *);

JSFUNCTION functionCallRefreshFinish(ExecState *);

JSFUNCTION functionCallUpdateAttrs(ExecState *);

JSFUNCTION functionCallUpdateStyle(ExecState *);

JSFUNCTION functionCallRemoveElement(ExecState *);

JSFUNCTION functionCallMoveElement(ExecState *);

JSFUNCTION functionCallAddEvent(ExecState *);

JSFUNCTION functionCallRemoveEvent(ExecState *);

JSFUNCTION functionGCanvasLinkNative(ExecState *);

JSFUNCTION functionSetIntervalWeex(ExecState *);

JSFUNCTION functionClearIntervalWeex(ExecState *);

JSFUNCTION functionT3DLinkNative(ExecState *);

JSFUNCTION functionNativeLogContext(ExecState *);

JSFUNCTION functionDisPatchMeaage(ExecState *);

JSFUNCTION functionDispatchMessageSync(ExecState *);

JSFUNCTION functionPostMessage(ExecState *);

JSFUNCTION functionNativeSetTimeout(ExecState *);
JSFUNCTION functionNativeSetInterval(ExecState *);
JSFUNCTION functionNativeClearTimeout(ExecState *);
JSFUNCTION functionNativeClearInterval(ExecState *);

// For data render
JSFUNCTION functionUpdateComponentData(ExecState *);

static JSValue createWeexConsoleProperty(VM& vm, JSObject* object) {
    JSGlobalObject *global = jsCast<JSGlobalObject *>(object);
    return WeexConsoleObject::create(vm, global, WeexConsoleObject::createStructure(vm, global, constructEmptyObject(
            global->globalExec())));
}

const ClassInfo WeexGlobalObject::s_info = {"global", &JSGlobalObject::s_info, nullptr,
                                            CREATE_METHOD_TABLE(WeexGlobalObject)};
const GlobalObjectMethodTable WeexGlobalObject::s_globalObjectMethodTable = {
        &supportsRichSourceInfo,
        &shouldInterruptScript,
        &javaScriptRuntimeFlags,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
};

WeexGlobalObject::WeexGlobalObject(VM &vm, Structure *structure)
        : JSGlobalObject(vm, structure, &s_globalObjectMethodTable), script_bridge_() {
    function_id_ = 0;
    Base::finishCreation(vm);
}

void WeexGlobalObject::SetScriptBridge(WeexCore::ScriptBridge *script_bridge) {
    script_bridge_ = script_bridge;
}

void WeexGlobalObject::addExtraOptions(std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    if(params.size() <= 0) {
        return;
    }
    VM &vm = this->vm();
    JSNonFinalObject *WXExtraOption = SimpleObject::create(vm, this);
    for (int i = 0; i < params.size(); i++) {
        INIT_FRAMEWORK_PARAMS *param = params[i];

        String &&type = String::fromUTF8(param->type->content);
        String &&value = String::fromUTF8(param->value->content);
        addString(vm, WXExtraOption, param->type->content, WTFMove(value));
    }
    addValue(vm, "WXExtraOption", WXExtraOption);
}

void WeexGlobalObject::initWxEnvironment(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool forAppContext, bool isSave) {
    VM &vm = this->vm();
    JSNonFinalObject *WXEnvironment = SimpleObject::create(vm, this);
    for (int i = 0; i < params.size(); i++) {
        INIT_FRAMEWORK_PARAMS *param = params[i];

        String &&type = String::fromUTF8(param->type->content);
        String &&value = String::fromUTF8(param->value->content);
        if (isSave) {
            auto init_framework_params = (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));

            if (init_framework_params == nullptr) {
                return;
            }

            memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
            init_framework_params->type = genWeexByteArraySS(param->type->content, param->type->length);
            init_framework_params->value = genWeexByteArraySS(param->value->content, param->value->length);

            m_initFrameworkParams.push_back(init_framework_params);
        }

        if(!isGlobalConfigStartUpSet){
            if (strncmp(type.utf8().data(), WX_GLOBAL_CONFIG_KEY, strlen(WX_GLOBAL_CONFIG_KEY)) == 0) {
                const char *config = value.utf8().data();
                doUpdateGlobalSwitchConfig(config);
            }
            isGlobalConfigStartUpSet = true;
        }

        // --------------------------------------------------------
        // add for debug mode
        if (String("debugMode") == type && String("true") == value) {
            Weex::LogUtil::setDebugMode(true);
        }
        // --------------------------------------------------------

        //LOGE("initWxEnvironment and value is %s", value.utf8().data());
        addString(vm, WXEnvironment, param->type->content, WTFMove(value));
        //free(params);
    }

    if (forAppContext)
        addValue(vm, "__windmill_environment__", WXEnvironment);
    else
        addValue(vm, "WXEnvironment", WXEnvironment);
}

void WeexGlobalObject::initFunctionForContext() {
    VM &vm = this->vm();
    const HashTableValue JSEventTargetPrototypeTableValues[] = {
            {"nativeLog",             JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeLogContext),  (intptr_t) (5)}},
            {"atob",                  JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionAtob),              (intptr_t) (1)}},
            {"btoa",                  JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionBtoa),              (intptr_t) (1)}},
            {"callGCanvasLinkNative", JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionGCanvasLinkNative), (intptr_t) (3)}},
            {"callT3DLinkNative",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionT3DLinkNative),     (intptr_t) (2)}},
            {"setNativeTimeout",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeSetTimeout),  (intptr_t) (2)}},
            {"setNativeInterval",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeSetInterval),  (intptr_t) (2)}},
            {"clearNativeTimeout",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeClearTimeout),  (intptr_t) (1)}},
            {"clearNativeInterval",   JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeClearInterval),  (intptr_t) (1)}},
            { "console", DontEnum|PropertyCallback, NoIntrinsic, { (intptr_t) static_cast<LazyPropertyCallback>(createWeexConsoleProperty), (intptr_t)(0) } },
            {"__updateComponentData",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionUpdateComponentData),       (intptr_t) (3)}},
    };
    reifyStaticProperties(vm, JSEventTargetPrototypeTableValues, *this);
}

void WeexGlobalObject::initFunction() {
    VM &vm = this->vm();
    const HashTableValue JSEventTargetPrototypeTableValues[] = {
            {"callNative",            JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallNative),          (intptr_t) (3)}},
            {"callNativeModule",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallNativeModule),    (intptr_t) (5)}},
            {"callNativeComponent",   JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallNativeComponent), (intptr_t) (5)}},
            {"callAddElement",        JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallAddElement),      (intptr_t) (5)}},
            {"setTimeoutNative",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionSetTimeoutNative),    (intptr_t) (2)}},
            {"nativeLog",             JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeLog),           (intptr_t) (5)}},
            {"notifyTrimMemory",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNotifyTrimMemory),    (intptr_t) (0)}},
            {"markupState",           JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionMarkupState),         (intptr_t) (0)}},
            {"atob",                  JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionAtob),                (intptr_t) (1)}},
            {"btoa",                  JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionBtoa),                (intptr_t) (1)}},
            {"callCreateBody",        JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallCreateBody),      (intptr_t) (3)}},
            {"callUpdateFinish",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallUpdateFinish),    (intptr_t) (3)}},
            {"callCreateFinish",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallCreateFinish),    (intptr_t) (3)}},
            {"callRefreshFinish",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallRefreshFinish),   (intptr_t) (3)}},
            {"callUpdateAttrs",       JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallUpdateAttrs),     (intptr_t) (4)}},
            {"callUpdateStyle",       JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallUpdateStyle),     (intptr_t) (4)}},
            {"callRemoveElement",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallRemoveElement),   (intptr_t) (3)}},
            {"callMoveElement",       JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallMoveElement),     (intptr_t) (5)}},
            {"callAddEvent",          JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallAddEvent),        (intptr_t) (4)}},
            {"callRemoveEvent",       JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionCallRemoveEvent),     (intptr_t) (4)}},
            {"callGCanvasLinkNative", JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionGCanvasLinkNative),   (intptr_t) (3)}},
            {"setIntervalWeex",       JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionSetIntervalWeex),     (intptr_t) (3)}},
            {"clearIntervalWeex",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionClearIntervalWeex),   (intptr_t) (1)}},
            {"callT3DLinkNative",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionT3DLinkNative),       (intptr_t) (2)}},
            {"__updateComponentData", JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionUpdateComponentData),       (intptr_t) (3)}},
//            {"setNativeTimeout",      JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeSetTimeout),    (intptr_t) (2)}},
//            {"setNativeInterval",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeSetInterval),  (intptr_t) (2)}},
//            {"clearNativeTimeout",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeClearTimeout),  (intptr_t) (1)}},
//            {"clearNativeInterval",   JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeClearInterval),  (intptr_t) (1)}},
    };
    reifyStaticProperties(vm, JSEventTargetPrototypeTableValues, *this);
}

void WeexGlobalObject::initFunctionForAppContext() {
    VM &vm = this->vm();
    const HashTableValue JSEventTargetPrototypeTableValues[] = {
            {"nativeLog",            JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeLogContext), (intptr_t) (5)}},
            {"__dispatch_message__", JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionDisPatchMeaage),   (intptr_t) (3)}},
            {"__dispatch_message_sync__", JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionDispatchMessageSync),   (intptr_t) (3)}},
            {"postMessage",          JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionPostMessage),      (intptr_t) (1)}},
            {"setNativeTimeout",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeSetTimeout), (intptr_t) (2)}},
            {"setNativeInterval",     JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeSetInterval),  (intptr_t) (2)}},
            {"clearNativeTimeout",    JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeClearTimeout),  (intptr_t) (1)}},
            {"clearNativeInterval",   JSC::Function, NoIntrinsic, {(intptr_t) static_cast<NativeFunction>(functionNativeClearInterval),  (intptr_t) (1)}},
            { "console", DontEnum|PropertyCallback, NoIntrinsic, { (intptr_t) static_cast<LazyPropertyCallback>(createWeexConsoleProperty), (intptr_t)(0) } },
    };
    reifyStaticProperties(vm, JSEventTargetPrototypeTableValues, *this);
}

uint32_t WeexGlobalObject::genFunctionID() {
    if (function_id_ > (INT_MAX - 1)) {
       LOGE(" WeexGlobalObject::genFunctionID timer fucntion id to large, something wrong now, crash!");
       abort();
    }
    return function_id_++;
}

void WeexGlobalObject::addTimer(uint32_t function_id, JSC::Strong<JSC::Unknown>&& function) {
    MapIterator iter = function_maps_.find(function_id);
    if (iter != function_maps_.end()) {
       LOGE("timer already exist in map, return now");
       return;
    }
    function_maps_.insert(std::pair<uint32_t, JSC::Strong<JSC::Unknown>>(function_id, WTFMove(function)));
}

void WeexGlobalObject::removeTimer(uint32_t function_id) {
    MapIterator iter = function_maps_.find(function_id);
    if (iter == function_maps_.end()) {
       LOGE("timer do not exist!");
       return;
    }
    function_maps_.erase(function_id);
}

JSValue WeexGlobalObject::getTimerFunction(uint32_t function_id) {
    MapIterator iter = function_maps_.find(function_id);
    if (iter == function_maps_.end())
        return jsUndefined();
   return function_maps_[function_id].get();
}

void WeexGlobalObject::updateInitFrameworkParams(const std::string &key,
                                                 const std::string &value) {

    LOGE("updateInitFrameworkParams %s %s ", key.data(), value.data());
    for(INIT_FRAMEWORK_PARAMS* param : m_initFrameworkParams){
        if(key.length() == param->type->length){
            if(strncmp(key.data(), param->type->content, key.length()) == 0){
                WeexByteArray * oldValue = param->value;
                param->value = genWeexByteArraySS(value.data(), value.length());
                free(oldValue);
            }
        }
    }
}

JSFUNCTION functionGCAndSweep(ExecState *exec) {
    JSLockHolder lock(exec);
    // exec->heap()->collectAllGarbage();
    return JSValue::encode(jsNumber(exec->heap()->sizeAfterLastFullCollection()));
}

JSFUNCTION functionSetIntervalWeex(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionSetIntervalWeex");
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    JSValue id_js = state->argument(0);
    String id_str = id_js.toWTFString(state);
    JSValue task_js = state->argument(1);
    String task_str = task_js.toWTFString(state);
    JSValue callback_js = state->argument(2);
    String callback_str = callback_js.toWTFString(state);
    auto result = globalObject->js_bridge()->core_side()->SetInterval(id_str.utf8().data(),
                                                                      task_str.utf8().data(),
                                                                      callback_str.utf8().data());
    return JSValue::encode(jsNumber(result));
}

JSFUNCTION functionClearIntervalWeex(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionClearIntervalWeex");
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    JSValue id_js = state->argument(0);
    String id_str = id_js.toWTFString(state);
    JSValue callback_js = state->argument(1);
    String callback_str = callback_js.toWTFString(state);

    globalObject->js_bridge()->core_side()->ClearInterval(id_str.utf8().data(), callback_str.utf8().data());

    return JSValue::encode(jsBoolean(true));
}

JSFUNCTION functionCallNative(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "callNative");

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    JSValue id_js = state->argument(0);
    String id_str = id_js.toWTFString(state);
    auto task = getCharOrJSONStringFromState(state, 1);
    JSValue callback_js = state->argument(2);
    String callback_str = callback_js.toWTFString(state);

    globalObject->js_bridge()->core_side()->CallNative(id_str.utf8().data(), task.get(),
                                                       callback_str.utf8().data());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionGCanvasLinkNative(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "callGCanvasLinkNative");

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    JSValue id_js = state->argument(0);
    String id_str = id_js.toWTFString(state);
    int type = state->argument(1).asInt32();
    JSValue arg_js = state->argument(2);
    String arg_str = arg_js.toWTFString(state);

    auto result = globalObject->js_bridge()->core_side()->CallGCanvasLinkNative(id_str.utf8().data(),
                                                                                type, arg_str.utf8().data());
    return JSValue::encode(String2JSValue(state, result));
}

JSFUNCTION functionT3DLinkNative(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionT3DLinkNative");

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    int type = state->argument(0).asInt32();
    JSValue arg_js = state->argument(1);
    String arg_str = arg_js.toWTFString(state);

    auto result = globalObject->js_bridge()->core_side()->CallT3DLinkNative(type, arg_str.utf8().data());
    return JSValue::encode(String2JSValue(state, result));
}

JSFUNCTION functionCallNativeModule(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "callNativeModule");
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    Args instanceId;
    Args moduleChar;
    Args methodChar;
    Args arguments;
    Args options;
    getStringArgsFromState(state, 0, instanceId);
    getStringArgsFromState(state, 1, moduleChar);
    getStringArgsFromState(state, 2, methodChar);
    getWsonOrJsonArgsFromState(state, 3, arguments);
    getWsonOrJsonArgsFromState(state, 4, options);

//    String a;
//    a.append("functionCallNativeModule:");
//    a.append(moduleChar.getValue());
//    a.append(":");
//    a.append(methodChar.getValue());
//    weex::base::TimeCalculator timeCalculator(weex::base::TaskPlatform::JSS_ENGINE, a.utf8().data(), instanceId.getValue());
//    timeCalculator.taskStart();
    auto result = globalObject->js_bridge()->core_side()->CallNativeModule(instanceId.getValue(),
                                                                           moduleChar.getValue(),
                                                                           methodChar.getValue(),
                                                                           arguments.getValue(),
                                                                           arguments.getLength(),
                                                                           options.getValue(),
                                                                           options.getLength());
//    timeCalculator.taskEnd();
    JSValue ret;
    switch (result->type) {
        case ParamsType::DOUBLE:
            ret = jsNumber(result->value.doubleValue);
            break;
        case ParamsType::STRING:
            ret = jString2JSValue(state, result->value.string->content, result->value.string->length);
            break;
        case ParamsType::JSONSTRING: {
            String val = jString2String(result->value.string->content, result->value.string->length);
            ret = parseToObject(state, val);
            free(result->value.string);
        }
            break;
        case ParamsType::BYTEARRAY: {
            ret = wson::toJSValue(state, (void *) result->value.byteArray->content, result->value.byteArray->length);
            free(result->value.byteArray);
        }
            break;
        default:
            ret = jsUndefined();
    }
    return JSValue::encode(ret);


}

JSFUNCTION functionCallNativeComponent(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "callNativeComponent");
    Args instanceId;
    Args moduleChar;
    Args methodChar;
    Args arguments;
    Args options;
    getStringArgsFromState(state, 0, instanceId);
    getStringArgsFromState(state, 1, moduleChar);
    getStringArgsFromState(state, 2, methodChar);
    getWsonOrJsonArgsFromState(state, 3, arguments);
    getWsonOrJsonArgsFromState(state, 4, options);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    globalObject->js_bridge()->core_side()->CallNativeComponent(instanceId.getValue(),
                                                                moduleChar.getValue(),
                                                                methodChar.getValue(),
                                                                arguments.getValue(),
                                                                arguments.getLength(),
                                                                options.getValue(),
                                                                options.getLength());
    return JSValue::encode(jsNumber(0));


}

JSFUNCTION functionCallAddElement(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "callAddElement");
    Args instanceId;
    Args parentRefChar;
    Args domStr;
    Args index_cstr;

    getStringArgsFromState(state, 0, instanceId);
    getStringArgsFromState(state, 1, parentRefChar);
    getWsonArgsFromState(state, 2, domStr);
    getStringArgsFromState(state, 3, index_cstr);


    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    globalObject->js_bridge()->core_side()->AddElement(instanceId.getValue(),
                                                       parentRefChar.getValue(),
                                                       domStr.getValue(),
                                                       domStr.getLength(),
                                                       index_cstr.getValue());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallCreateBody(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "callCreateBody");
    Args pageId;
    Args domStr;
    getStringArgsFromState(state, 0, pageId);
    getWsonArgsFromState(state, 1, domStr);


    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    globalObject->js_bridge()->core_side()->CreateBody(pageId.getValue(), domStr.getValue(), domStr.getLength());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallUpdateFinish(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallUpdateFinish");

    Args idChar;
    Args taskChar;
    Args callBackChar;
    getStringArgsFromState(state, 0, idChar);
    getWsonArgsFromState(state, 1, taskChar);
    getWsonArgsFromState(state, 2, callBackChar);
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
//    weex::base::TimeCalculator timeCalculator(weex::base::TaskPlatform::JSS_ENGINE, "functionCallUpdateFinish",globalObject->id);
//    timeCalculator.taskStart();


    auto result = globalObject->js_bridge()->core_side()->UpdateFinish(idChar.getValue(), taskChar.getValue(),
                                                                       taskChar.getLength(), callBackChar.getValue(),
                                                                       callBackChar.getLength());

//    timeCalculator.taskEnd();
    return JSValue::encode(jsNumber(result));
}

JSFUNCTION functionCallCreateFinish(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallCreateFinish");

    Args idChar;
    getStringArgsFromState(state, 0, idChar);
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->CreateFinish(idChar.getValue());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallRefreshFinish(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallRefreshFinish");

    Args idChar;
    Args taskChar;
    Args callBackChar;
    getStringArgsFromState(state, 0, idChar);
    getStringArgsFromState(state, 1, taskChar);
    getStringArgsFromState(state, 2, callBackChar);
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
//    weex::base::TimeCalculator timeCalculator(weex::base::TaskPlatform::JSS_ENGINE, "functionCallRefreshFinish",globalObject->id);
//    timeCalculator.taskStart();

    int result = globalObject->js_bridge()->core_side()->RefreshFinish(idChar.getValue(),
                                                                       taskChar.getValue(),
                                                                       callBackChar.getValue());

//    timeCalculator.taskEnd();
    return JSValue::encode(jsNumber(result));
}


JSFUNCTION functionCallUpdateAttrs(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallUpdateAttrs");

    Args instanceId;
    Args ref;
    Args domAttrs;
    getStringArgsFromState(state, 0, instanceId);
    getStringArgsFromState(state, 1, ref);
    getWsonArgsFromState(state, 2, domAttrs);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->UpdateAttrs(instanceId.getValue(),
                                                        ref.getValue(),
                                                        domAttrs.getValue(), domAttrs.getLength());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallUpdateStyle(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallUpdateStyle");

    Args instanceId;
    Args ref;
    Args domStyles;
    getStringArgsFromState(state, 0, instanceId);
    getStringArgsFromState(state, 1, ref);
    getWsonArgsFromState(state, 2, domStyles);


    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->UpdateStyle(instanceId.getValue(),
                                                        ref.getValue(),
                                                        domStyles.getValue(), domStyles.getLength());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallRemoveElement(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallRemoveElement");

    Args idChar;
    Args dataChar;
    getStringArgsFromState(state, 0, idChar);
    getStringArgsFromState(state, 1, dataChar);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->RemoveElement(idChar.getValue(), dataChar.getValue());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallMoveElement(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallMoveElement");

    Args idChar;
    Args refChar;
    Args dataChar;
    Args indexChar;
    getStringArgsFromState(state, 0, idChar);
    getStringArgsFromState(state, 1, refChar);
    getStringArgsFromState(state, 2, dataChar);
    getStringArgsFromState(state, 3, indexChar);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->MoveElement(idChar.getValue(),
                                                        refChar.getValue(),
                                                        dataChar.getValue(),
                                                        atoi(indexChar.getValue()));
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallAddEvent(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallAddEvent");


    Args idChar;
    Args refChar;
    Args eventChar;
    getStringArgsFromState(state, 0, idChar);
    getStringArgsFromState(state, 1, refChar);
    getStringArgsFromState(state, 2, eventChar);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->AddEvent(idChar.getValue(),
                                                     refChar.getValue(),
                                                     eventChar.getValue());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionCallRemoveEvent(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionCallRemoveEvent");

    Args idChar;
    Args refChar;
    Args eventChar;
    getStringArgsFromState(state, 0, idChar);
    getStringArgsFromState(state, 1, refChar);
    getStringArgsFromState(state, 2, eventChar);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->RemoveEvent(idChar.getValue(),
                                                        refChar.getValue(),
                                                        eventChar.getValue());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionSetTimeoutNative(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "setTimeoutNative");

    Args callbackChar;
    Args timeChar;
    getStringArgsFromState(state, 0, callbackChar);
    getStringArgsFromState(state, 1, timeChar);

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    globalObject->js_bridge()->core_side()->SetTimeout(callbackChar.getValue(), timeChar.getValue());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionNativeLog(ExecState *state) {
    bool result = false;
    StringBuilder sb;
    for (int i = 0; i < state->argumentCount(); i++) {
        sb.append(state->argument(i).toWTFString(state));
    }

    if (!sb.isEmpty()) {

        WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
        globalObject->js_bridge()->core_side()->NativeLog(sb.toString().utf8().data());
    }
    return JSValue::encode(jsBoolean(true));
}

JSFUNCTION functionNativeLogContext(ExecState *state) {
    //bool result = false;
    StringBuilder sb;
    for (int i = 0; i < state->argumentCount(); i++) {
        sb.append(state->argument(i).toWTFString(state));
    }

    if (!sb.isEmpty()) {

        WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
        globalObject->js_bridge()->core_side()->NativeLog(sb.toString().utf8().data());
    }
    return JSValue::encode(jsBoolean(true));
}

JSFUNCTION functionPostMessage(ExecState *state) {

    Args dataChar;
    getJSONArgsFromState(state, 0, dataChar);
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    String id(globalObject->id.c_str());
    std::string data(dataChar.getValue());
    globalObject->js_bridge()->core_side()->PostMessage(id.utf8().data(), data.c_str(), dataChar.getLength());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionDisPatchMeaage(ExecState *state) {

    Args clientIdChar;
    Args dataChar;
    Args callBackChar;
    getStringArgsFromState(state, 0, clientIdChar);
    getJSONArgsFromState(state, 1, dataChar);
    getStringArgsFromState(state, 2, callBackChar);
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    String id(globalObject->id.c_str());
    std::string data(dataChar.getValue());
    globalObject->js_bridge()->core_side()->DispatchMessage(clientIdChar.getValue(), data.c_str(), dataChar.getLength(),
                                                            callBackChar.getValue(), id.utf8().data());
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionDispatchMessageSync(ExecState *state) {
    Args clientIdChar;
    Args dataChar;
    getStringArgsFromState(state, 0, clientIdChar);
    getJSONArgsFromState(state, 1, dataChar);
    WeexGlobalObject *globalObject =
      static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    String id(globalObject->id.c_str());
    std::string data(dataChar.getValue());
    auto result = globalObject->js_bridge()->core_side()->DispatchMessageSync(
      clientIdChar.getValue(), data.c_str(), dataChar.getLength(),
      id.utf8().data());
    if (result->length == 0) {
        return JSValue::encode(jsUndefined());
    } else {
        String result_str = String::fromUTF8(result->data.get(), result->length);
        return JSValue::encode(parseToObject(state, result_str));
    }
}

JSFUNCTION functionNotifyTrimMemory(ExecState *state) {
    return functionGCAndSweep(state);
}

JSFUNCTION functionMarkupState(ExecState *state) {
    markupStateInternally();
    return JSValue::encode(jsUndefined());
}

JSFUNCTION functionAtob(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "atob");
    JSValue ret = jsUndefined();
    JSValue val = state->argument(0);

    if (!val.isUndefined()) {
        String original = val.toWTFString(state);
        // std::string input_str(original.utf8().data());
        // std::string output_str;
        // Base64Decode(input_str, &output_str);
        // WTF::String s(output_str.c_str());
        // ret = jsNontrivialString(&state->vm(), WTFMove(s));
        Vector<char> out;
        if (base64Decode(original, out, Base64ValidatePadding | Base64IgnoreSpacesAndNewLines)) {
            WTF::String output = String(out.data(), out.size());
            ret = jsNontrivialString(&state->vm(), WTFMove(output));
        }
    } else {
        //ret = "";
    }
    return JSValue::encode(ret);
}

JSFUNCTION functionBtoa(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "btoa");

    JSValue ret = jsUndefined();
    JSValue val = state->argument(0);
    String original = val.toWTFString(state);
    String out;
    if (original.isNull())
        out = String("");

    if (original.containsOnlyLatin1()) {
        out = base64Encode(original.latin1());
    }
    ret = jsNontrivialString(&state->vm(), WTFMove(out));
    return JSValue::encode(ret);
}

JSFUNCTION functionNativeSetTimeout(ExecState *state) {
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    size_t i = state->argumentCount();
    if (i < 2)
        return JSValue::encode(jsNumber(0));
    VM &vm = globalObject->vm();
    const JSValue value = state->argument(0);
    const JSValue jsValue = state->argument(1);
    TimerQueue *timerQueue = globalObject->timeQueue;
    if (timerQueue != nullptr) {
        uint32_t function_id = globalObject->genFunctionID();
        globalObject->addTimer(function_id, JSC::Strong<JSC::Unknown> { vm, JSC::asObject(value) });
        uint64_t timeout = static_cast<uint64_t>(jsValue.asInt32());
        if(timeout < 1)
            timeout = 1;
        TimerTask *task = new TimerTask(globalObject->id.c_str(), function_id,
                                        timeout, globalObject, false);

        timerQueue->addTimerTask(task);
        return JSValue::encode(jsNumber(task->taskId));;
    }

    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionNativeSetInterval(ExecState *state) {
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    size_t i = state->argumentCount();
    if (i < 2)
        return JSValue::encode(jsNumber(0));
    VM &vm = globalObject->vm();
    const JSValue value = state->argument(0);
    const JSValue jsValue = state->argument(1);
    TimerQueue *timerQueue =globalObject->timeQueue;
    if (timerQueue != nullptr) {
        uint32_t function_id = globalObject->genFunctionID();
        globalObject->addTimer(function_id, JSC::Strong<JSC::Unknown> { vm, JSC::asObject(value) });
        TimerTask *task = new TimerTask(globalObject->id.c_str(), function_id,
                                        static_cast<uint64_t>(jsValue.asInt32()), globalObject, true);
        timerQueue->addTimerTask(task);
        return JSValue::encode(jsNumber(task->taskId));;
    }

    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionNativeClearTimeout(ExecState *state) {
    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());
    TimerQueue *timerQueue = globalObject->timeQueue;
    const JSValue& value = state->argument(0);
    if (timerQueue != nullptr) {
        timerQueue->removeTimer(value.asInt32());
    }
    return JSValue::encode(jsNumber(0));
}

JSFUNCTION functionNativeClearInterval(ExecState *state) {
    return functionNativeClearTimeout(state);
}

JSFUNCTION functionUpdateComponentData(ExecState *state) {
    base::debug::TraceScope traceScope("weex", "functionUpdateComponentData");

    WeexGlobalObject *globalObject = static_cast<WeexGlobalObject *>(state->lexicalGlobalObject());

    auto page_id = getCharOrJSONStringFromState(state, 0);
    auto cid = getCharOrJSONStringFromState(state, 1);
    auto json_data = getCharOrJSONStringFromState(state, 2);

    globalObject->js_bridge()->core_side()->UpdateComponentData(page_id.get(), cid.get(), json_data.get());
    return JSValue::encode(jsUndefined());
}
