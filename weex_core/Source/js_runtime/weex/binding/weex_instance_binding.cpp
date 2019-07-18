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

#include "weex_instance_binding.h"
#include "js_runtime/utils/log_utils.h"
#include "js_runtime/weex/object/weex_global_object_v2.h"
#include "core/bridge/script_bridge.h"
#include "weex_binding_utils.h"

namespace weex {
    namespace jsengine {

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, nativeLog)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, atob)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, btoa)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, callGCanvasLinkNative)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, callT3DLinkNative)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, setNativeTimeout)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, setNativeInterval)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, clearNativeTimeout)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, clearNativeInterval)

        CLASS_MEMBER_GET_CALLBACK(WeexInstanceBinding, console)

        CLASS_METHOD_CALLBACK(WeexInstanceBinding, __updateComponentData)

        CLASS_REGISTER_START(WeexInstanceBinding, Instance)
            SET_IS_GLOBAL
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, nativeLog)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, atob)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, btoa)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, callGCanvasLinkNative)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, callT3DLinkNative)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, setNativeTimeout)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, setNativeInterval)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, clearNativeTimeout)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, clearNativeInterval)
            REGISTER_METHOD_CALLBACK(WeexInstanceBinding, __updateComponentData)
            REGISTER_GET_CALLBACK(WeexInstanceBinding, console)
        CLASS_REGISTER_END(WeexInstanceBinding)


        WeexInstanceBinding::WeexInstanceBinding(unicorn::EngineContext *context, const OpaqueJSContext *js_ctx)
                : RuntimeObject(context, js_ctx) {
            //SetJSClass(WeexInstanceBinding::s_jsclass_WeexInstanceBinding);
            LOG_WEEX_BINDING("WeexInstanceBinding init");
            consoleBinding.reset(new WeexConsoleBinding(context, js_ctx));
            SetJSClass(weex::jsengine::WeexInstanceBinding::CreateClassRef(context));
        }

        WeexInstanceBinding::~WeexInstanceBinding() {
            LOG_WEEX_BINDING("[release] WeexInstanceBinding delete");
            this->nativeObject = nullptr;

        }

        unicorn::ScopeValues
        WeexInstanceBinding::nativeLog(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::nativeLog(this->nativeObject, vars, false);
        }

        unicorn::ScopeValues
        WeexInstanceBinding::atob(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::atob(this->nativeObject, vars);
        }

        unicorn::ScopeValues
        WeexInstanceBinding::btoa(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::btoa(this->nativeObject, vars);
        }

        unicorn::ScopeValues WeexInstanceBinding::callGCanvasLinkNative(
                const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexInstanceBinding method :callGCanvasLinkNative");
            return WeexBindingUtils::callGCanvasLinkNative(nativeObject, vars);
        }

        unicorn::ScopeValues WeexInstanceBinding::callT3DLinkNative(
                const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexInstanceBinding method :callT3DLinkNative");
            return WeexBindingUtils::callT3DLinkNative(nativeObject, vars);
        }

        unicorn::ScopeValues WeexInstanceBinding::setNativeTimeout(
                std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("instanceConteext: setNativeTimeout this:%p,nativeObject:%p", this,nativeObject);
            return WeexBindingUtils::setNativeTimeout(nativeObject, vars, false);
        }

        unicorn::ScopeValues WeexInstanceBinding::setNativeInterval(
                std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("instanceConteext: setNativeInterval this:%p,nativeObject:%p", this,nativeObject);
            return WeexBindingUtils::setNativeTimeout(nativeObject, vars, true);
        }

        unicorn::ScopeValues WeexInstanceBinding::clearNativeTimeout(
                std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::clearNativeTimeout(this->nativeObject, vars);
        }

        unicorn::ScopeValues WeexInstanceBinding::clearNativeInterval(
                std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::clearNativeTimeout(this->nativeObject, vars);
        }

        unicorn::ScopeValues WeexInstanceBinding::console() {
            LOG_WEEX_BINDING("[console] WeexInstanceBinding  console :%p",this->consoleBinding.get());
            if (nullptr == nativeObject || nullptr == this->consoleBinding.get()){
                return unicorn::RuntimeValues::MakeUndefined();
            }
            return unicorn::RuntimeValues::MakeCommonObject(
                    static_cast<void *>(this->consoleBinding.get()),
                    new unicorn::RuntimeClass(consoleBinding->GetJSClass())
            );
        }

        unicorn::ScopeValues WeexInstanceBinding::__updateComponentData(
                const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexInstanceBinding method :__updateComponentData");
            return WeexBindingUtils::__updateComponentData(this->nativeObject,vars);
        }
    }
}


