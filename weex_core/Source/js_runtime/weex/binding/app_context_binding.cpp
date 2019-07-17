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
// Created by chenpeihan on 2019/1/23.
//


#include "app_context_binding.h"
#include "weex_binding_utils.h"
#include "core/bridge/script_bridge.h"
#include "js_runtime/weex/object/weex_global_object_v2.h"
#include "js_runtime/weex/utils/weex_conversion_utils.h"

namespace weex {
    namespace jsengine {
        CLASS_METHOD_CALLBACK(AppWorkerBinding, nativeLog)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, __dispatch_message__)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, __dispatch_message_sync__)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, postMessage)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, setNativeTimeout)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, setNativeInterval)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, clearNativeTimeout)

        CLASS_METHOD_CALLBACK(AppWorkerBinding, clearNativeInterval)

        CLASS_MEMBER_GET_CALLBACK(AppWorkerBinding, console)

        CLASS_REGISTER_START(AppWorkerBinding, AppWorker)
            SET_IS_GLOBAL
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, nativeLog)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, __dispatch_message__)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, __dispatch_message_sync__)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, postMessage)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, setNativeTimeout)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, setNativeInterval)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, clearNativeTimeout)
            REGISTER_METHOD_CALLBACK(AppWorkerBinding, clearNativeInterval)
            REGISTER_GET_CALLBACK(AppWorkerBinding, console)
        CLASS_REGISTER_END(AppWorkerBinding)

        AppWorkerBinding::AppWorkerBinding(unicorn::EngineContext *context, const OpaqueJSContext *js_ctx)
                : RuntimeObject(context, js_ctx) {
           // SetJSClass(AppWorkerBinding::s_jsclass_AppWorkerBinding);
            consoleBinding.reset(new WeexConsoleBinding(context, js_ctx));
            SetJSClass(AppWorkerBinding::CreateClassRef(context));
        }

        AppWorkerBinding::~AppWorkerBinding() {
            this->nativeObject = nullptr;
        }

        unicorn::ScopeValues
        AppWorkerBinding::nativeLog(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::nativeLog(this->nativeObject, vars, false);
        }

        unicorn::ScopeValues AppWorkerBinding::__dispatch_message__(
                const std::vector<unicorn::ScopeValues> &vars) {

            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__");
            if (nullptr == this->nativeObject){
                return unicorn::RuntimeValues::MakeInt(0);
            }

            std::string client_id;
            std::string data;
            std::string callback;
            std::string vm_id;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, client_id);
            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ client_id is %s", client_id.c_str());

            WeexConversionUtils::GetJSONArgsFromArgsByWml(vars, 1, data);
            //bool succeed = WeexConversionUtils::GetJSONArgsFromArgsByWml(vars, 1, data);
            //const char *data_char = succeed ? data.c_str() : nullptr;
            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ data is %s", data.c_str());

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, callback);
            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ callback is %s", callback.c_str());

            vm_id = this->nativeObject->id;
            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ vm_id is %s", vm_id.c_str());

            this->nativeObject->js_bridge()->core_side()->DispatchMessage(client_id.c_str(), data.c_str(), data.length(),
                                                                          callback.c_str(), vm_id.c_str());
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues
        AppWorkerBinding::__dispatch_message_sync__(
                const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message_sync__");
            if (nullptr == this->nativeObject){
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string client_id;
            std::string data;
            std::string vm_id;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, client_id);
            WeexConversionUtils::GetJSONArgsFromArgsByWml(vars, 1, data);
            vm_id = this->nativeObject->id;

            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ , cliend:%s, vm:%s,data:%s", client_id.c_str(),
                             vm_id.c_str(), data.c_str());

            auto result = this->nativeObject->js_bridge()->core_side()->DispatchMessageSync(client_id.c_str(),
                                                                                            data.c_str(),
                                                                                            data.length(),
                                                                                            vm_id.c_str());

            LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ , result:%s", result->data.get());

            if (result->length == 0) {
                return unicorn::RuntimeValues::MakeUndefined();
            } else {
                return unicorn::RuntimeValues::MakeObjectFromJsonStr(result->data.get());
            }

        }

        unicorn::ScopeValues
        AppWorkerBinding::postMessage(const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexRuntime: postMessage");
            if (nullptr == this->nativeObject){
                return unicorn::RuntimeValues::MakeInt(0);
            }

            std::string data;
            std::string vm_id;


            WeexConversionUtils::GetJSONArgsFromArgsByWml(vars, 0, data);
           // const char *data_char = succeed ? data.c_str() : nullptr;
            //LOG_WEEX_BINDING("WeexRuntime: postMessage data is %s", data_char);


            vm_id = this->nativeObject->id;
            //LOG_WEEX_BINDING("WeexRuntime: __dispatch_message__ vm_id is %s", vm_id.c_str());

            this->nativeObject->js_bridge()->core_side()->PostMessage(vm_id.c_str(), data.c_str(), data.length());

            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues
        AppWorkerBinding::setNativeTimeout(std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("appConteext: setNativeTimeout this:%p,nativeObject:%p", this, nativeObject);
            return WeexBindingUtils::setNativeTimeout(this->nativeObject, vars, false);
        }

        unicorn::ScopeValues AppWorkerBinding::setNativeInterval(
                std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("appConteext: setNativeInterval this:%p,nativeObject:%p", this, nativeObject);
            return WeexBindingUtils::setNativeTimeout(this->nativeObject, vars, true);
        }

        unicorn::ScopeValues AppWorkerBinding::clearNativeTimeout(
                std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::clearNativeTimeout(this->nativeObject, vars);
        }

        unicorn::ScopeValues AppWorkerBinding::clearNativeInterval(
                std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::clearNativeTimeout(this->nativeObject, vars);
        }

        unicorn::ScopeValues
        AppWorkerBinding::console() {
            if(nullptr == nativeObject || nullptr == this->consoleBinding.get()){
                return unicorn::RuntimeValues::MakeUndefined();
            }
            return unicorn::RuntimeValues::MakeCommonObject(
                    static_cast<void *>(this->consoleBinding.get()),
                    new unicorn::RuntimeClass(consoleBinding->GetJSClass())
            );
        }
    }
}
