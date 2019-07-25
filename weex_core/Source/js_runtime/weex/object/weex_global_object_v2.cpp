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
// Created by chenpeihan on 2019/2/22.
//

#include "base/log_defines.h"
#include "core/bridge/script_bridge.h"
#include "js_runtime/weex/utils/weex_jsc_utils.h"
#include "core/bridge/script_bridge.h"
#include "js_runtime/weex/binding/weex_global_binding.h"
#include "js_runtime/weex/binding/weex_instance_binding.h"
#include "js_runtime/weex/binding/app_context_binding.h"
#include "weex_global_object_v2.h"
#include "js_runtime/utils/log_utils.h"
#include "js_runtime/weex/utils/weex_conversion_utils.h"

static bool isGlobalConfigStartUpSet = false;
#define WX_GLOBAL_CONFIG_KEY "global_switch_config"

void WeexGlobalObjectV2::makeWeexGlobalObject(unicorn::RuntimeVM *vm) {
    this->object_type_ = WeexGlobal;
    auto binding_object = new weex::jsengine::WeexGlobalBinding(nullptr, nullptr);
    context = unicorn::RuntimeContext::Create(vm, binding_object->GetJSClass());
    binding_object->SetEngineContext(context->GetEngineContext());
    LOG_RUNTIME("[Context] makeWeexGlobalObject context ptr:%p", context->GetEngineContext()->GetContext());
    auto global_js_object = context->GetEngineContext()->GetGlobalObjectInContext();
    context->GetEngineContext()->BindDataToObject(global_js_object, binding_object);
    binding_object->SetJSObject(global_js_object);
    binding_object->nativeObject = this;
    this->global_object_binding.reset(static_cast<unicorn::RuntimeObject*>(binding_object));
    //binding dom api
    //weex::jsengine::DomManager::BindingDomApi(context->GetEngineContext(), std::string("global"),std::string("weex_global_context"));
}

void
WeexGlobalObjectV2::makeWeexInstanceObject(unicorn::RuntimeVM *vm, const std::string &id, const std::string &name) {
    this->object_type_ = WeexInstance;
    //global object not support bind static method for globalObject now cause jsruntime
    auto binding_object = new weex::jsengine::WeexInstanceBinding(nullptr, nullptr);
    context = unicorn::RuntimeContext::Create(vm, binding_object->GetJSClass());
    binding_object->SetEngineContext(context->GetEngineContext());
    context->GetEngineContext()->SetName(id);
    binding_object->nativeObject = this;
    auto global_js_object = context->GetEngineContext()->GetGlobalObjectInContext();
    context->GetEngineContext()->BindDataToObject(global_js_object, binding_object);
    binding_object->SetJSObject(global_js_object);
    this->global_object_binding.reset(static_cast<unicorn::RuntimeObject*>(binding_object));
}

void WeexGlobalObjectV2::makeAppWorkerObject(unicorn::RuntimeVM *vm) {
    this->object_type_ = AppWorker;
    //global object not support bind static method for globalObject now cause jsruntime
    auto binding_object = new weex::jsengine::AppWorkerBinding(nullptr, nullptr);
    context = unicorn::RuntimeContext::Create(vm, binding_object->GetJSClass());
    binding_object->SetEngineContext(context->GetEngineContext());
    auto global_js_object = context->GetEngineContext()->GetGlobalObjectInContext();
    context->GetEngineContext()->BindDataToObject(global_js_object, binding_object);
    binding_object->nativeObject = this;
    binding_object->SetJSObject(global_js_object);
    this->global_object_binding.reset(static_cast<unicorn::RuntimeObject*>(binding_object));
}

void WeexGlobalObjectV2::addExtraOptions(std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    if (params.size() <= 0) {
        return;
    }
    std::unique_ptr<unicorn::Map> wxExtraOption = unicorn::Map::CreateFromNative(this->context->GetEngineContext(),
                                                                                 unicorn::RuntimeValues::MakeNull());
    for (int i = 0; i < params.size(); i++) {
        INIT_FRAMEWORK_PARAMS *param = params[i];
        wxExtraOption->Insert(
                std::string(param->type->content),
                new unicorn::RuntimeValues(param->value->content, param->value->length)
        );

    }
    this->context->GetEngineContext()->SetGlobalPropertyValue(
            std::string("WXExtraOption"),
            unicorn::RuntimeValues::MakeMap(std::move(wxExtraOption))
    );
}

void
WeexGlobalObjectV2::initWxEnvironment(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool isSave) {
    std::unique_ptr<unicorn::Map> wxEnvironment = unicorn::Map::CreateFromNative(this->context->GetEngineContext(),
                                                                                 unicorn::RuntimeValues::MakeNull());
    for (int i = 0; i < params.size(); i++) {
        INIT_FRAMEWORK_PARAMS *param = params[i];

        std::string &&type = std::string(param->type->content);
        std::string &&value = std::string(param->value->content);
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

        if (!isGlobalConfigStartUpSet) {
            if (strncmp(type.c_str(), WX_GLOBAL_CONFIG_KEY, strlen(WX_GLOBAL_CONFIG_KEY)) == 0) {
                const char *config = value.c_str();
                doUpdateGlobalSwitchConfig(config);
            }
            isGlobalConfigStartUpSet = true;
        }

        // --------------------------------------------------------
        // add for debug mode
        if (std::string("debugMode") == type && std::string("true") == value) {
           // Weex::LogUtil::setDebugMode(true);
            weex::base::LogImplement::getLog()->setDebugMode(true);
            LOGE("jss use %s","runtime");
        }
        // --------------------------------------------------------

        //LOGE("initWxEnvironment and value is %s", value.c_str());
        // addstd::string(vm, WXEnvironment, param->type->content, WTFMove(value));
        //LOGD("initWxEnvironment initWxEnvironment  key:%s ,vaule: %s", type.c_str(), value.c_str());
        wxEnvironment->Insert(std::string(type.c_str()),
                              new unicorn::RuntimeValues(std::string(value.c_str())));
        //free(params);
    }
    const char *key = object_type_ == AppWorker ? "__windmill_environment__" : "WXEnvironment";

    this->context->GetEngineContext()->SetGlobalPropertyValue(
            std::string(key),
            unicorn::RuntimeValues::MakeMap(std::move(wxEnvironment))
    );
}

void WeexGlobalObjectV2::setScriptBridge(WeexCore::ScriptBridge *script_bridge) {
    this->script_bridge_ = script_bridge;
}


unicorn::RuntimeValues *WeexGlobalObjectV2::removeTimer(uint32_t function_id) {
    auto iter = function_maps_.find(function_id);
    if (iter == function_maps_.end()) {
        LOGE("timer do not exist!");
        return nullptr;
    }
    auto funcValue = function_maps_[function_id];
    function_maps_.erase(function_id);
    return funcValue;
}

//uint32_t WeexGlobalObjectV2::genFunctionID() {
//    if (function_id_ > (INT_MAX - 1)) {
//        LOGE(" WeexGlobalObject::genFunctionID timer fucntion id to large, something wrong now, crash!");
//        abort();
//    }
//    return function_id_++;
//}

unicorn::RuntimeValues *WeexGlobalObjectV2::getTimerFunction(uint32_t function_id) {
    auto iter = function_maps_.find(function_id);
    if (iter == function_maps_.end())
        return nullptr;
    return function_maps_[function_id];
}

void WeexGlobalObjectV2::updateInitFrameworkParams(const std::string &key,
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

//void WeexGlobalObjectV2::addTimer(uint32_t function_id, unicorn::RuntimeValues *func) {
//
//    auto iter = function_maps_.find(function_id);
//    if (iter != function_maps_.end()) {
//        LOGE("timer already exist in map, return now");
//        return;
//    }
//    function_maps_[function_id] = func;
//
//}

int WeexGlobalObjectV2::setNativeTimeout(unicorn::RuntimeValues *func, int timeOut, bool interval) {
    if (nullptr == func || nullptr == timeQueue) {
        return 0;
    }
    if (function_id_ > (INT_MAX - 1)) {
        LOGE(" WeexGlobalObject::genFunctionID timer fucntion id to large, something wrong now, crash!");
        abort();
    }
    function_id_++;
    function_maps_[function_id_] = func;
    if (timeOut < 1) {
        timeOut = 1;
    }
    LOG_WEEX_BINDING("WeexBindingUtils setNativeTimeout timeOut :%d , type:%d", timeOut, func->GetType());

    TimerTask *task = new TimerTask(this->id.c_str(), function_id_,
                                    timeOut, interval);
    if (this->getObjectType() == WeexInstance){
        task ->from_instance_ = true;
    } else {
        task->from_instance_ = false;
    }
    timeQueue->addTimerTask(task);
    return task->taskId;
}

void WeexGlobalObjectV2::clearNativeTimeout(int timer_task_id) {
    LOG_WEEX_BINDING("[WeexBindingUtils] method :clearNativeTimeout");
    if (nullptr == timeQueue) {
        return;
    }
    timeQueue->removeTimer(timer_task_id);
}


