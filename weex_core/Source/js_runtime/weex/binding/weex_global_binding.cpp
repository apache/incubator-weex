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

#include "weex_global_binding.h"
#include "js_runtime/utils/log_utils.h"
#include "weex_binding_utils.h"
#include "core/bridge/script_bridge.h"
#include "js_runtime/weex/utils/weex_conversion_utils.h"
#include <cstdlib>
#include "js_runtime/weex/object/weex_global_object_v2.h"

namespace weex {
    namespace jsengine {
        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callNative)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callNativeModule)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callNativeComponent)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, setTimeoutNative)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, nativeLog)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, notifyTrimMemory)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, markupState)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, atob)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, btoa)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callCreateBody)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callUpdateFinish)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callCreateFinish)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callRefreshFinish)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callUpdateAttrs)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callUpdateStyle)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callAddElement)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callRemoveElement)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callMoveElement)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callAddEvent)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callRemoveEvent)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callGCanvasLinkNative)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, setIntervalWeex)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, clearIntervalWeex)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, callT3DLinkNative)

        CLASS_METHOD_CALLBACK(WeexGlobalBinding, __updateComponentData)

        CLASS_REGISTER_START(WeexGlobalBinding, Global)
            SET_IS_GLOBAL
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callNative)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callNativeModule)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callNativeComponent)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, setTimeoutNative)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, nativeLog)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, notifyTrimMemory)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, markupState)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, atob)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, btoa)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callCreateBody)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callUpdateFinish)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callCreateFinish)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callRefreshFinish)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callUpdateAttrs)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callUpdateStyle)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callAddElement)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callRemoveElement)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callMoveElement)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callAddEvent)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callRemoveEvent)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callGCanvasLinkNative)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, setIntervalWeex)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, clearIntervalWeex)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, callT3DLinkNative)
            REGISTER_METHOD_CALLBACK(WeexGlobalBinding, __updateComponentData)
        CLASS_REGISTER_END(WeexGlobalBinding)


        WeexGlobalBinding::~WeexGlobalBinding() {
            LOG_WEEX_BINDING("WeexGlobalBinding delete");
            this->nativeObject = nullptr;
        }

        WeexGlobalBinding::WeexGlobalBinding(unicorn::EngineContext *context, const OpaqueJSContext *js_ctx)
                : RuntimeObject(context, js_ctx) {
            //SetJSClass(WeexGlobalBinding::s_jsclass_WeexGlobalBinding);
            SetJSClass(WeexGlobalBinding::CreateClassRef(context));
            LOG_WEEX_BINDING("WeexGlobalBinding init");
        }

        unicorn::ScopeValues
        WeexGlobalBinding::setIntervalWeex(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string page_id;
            std::string task_str;
            std::string callback_str;

            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 1, task_str);
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 2, callback_str);

            LOG_WEEX_BINDING("WeexGlobalBinding method :setIntervalWeex  page:% , task:%s , callback:%s",
                             page_id.c_str(), task_str.c_str(), callback_str.c_str());

            int result = nativeObject->js_bridge()->core_side()->SetInterval(page_id.c_str(), task_str.c_str(),
                                                                             callback_str.c_str());
            return unicorn::RuntimeValues::MakeInt(result);
        }

        unicorn::ScopeValues WeexGlobalBinding::clearIntervalWeex(
                const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeBool(false);
            }

            std::string page_id;
            std::string callback_str;

            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 1, callback_str);

            LOG_WEEX_BINDING("WeexGlobalBinding method :clearIntervalWeex page:%s, callBacK :%s", page_id.c_str(),
                             callback_str.c_str());

            nativeObject->js_bridge()->core_side()->ClearInterval(page_id.c_str(), callback_str.c_str());

            return unicorn::RuntimeValues::MakeBool(true);
        }


        unicorn::ScopeValues
        WeexGlobalBinding::callNative(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string page_id;
            std::string task;
            std::string callback_str;

            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 0, page_id);
            bool succeed = WeexConversionUtils::GetCharOrJsonFromArgs(vars, 1, task);
            const char *task_char = succeed ? task.c_str() : nullptr;
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 2, callback_str);

            LOG_WEEX_BINDING("WeexGlobalBinding callNative id_js:%s,task_str:%s,callback_str:%s",
                             page_id.c_str(), task_char, callback_str.c_str());

            this->nativeObject->js_bridge()->core_side()->CallNative(page_id.c_str(), task_char,
                                                                     callback_str.c_str());
            return unicorn::RuntimeValues::MakeUndefined();
        }

        unicorn::ScopeValues WeexGlobalBinding::callGCanvasLinkNative(
                const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexGlobalBinding method :callGCanvasLinkNative");
            return WeexBindingUtils::callGCanvasLinkNative(nativeObject, vars);
        }


        unicorn::ScopeValues WeexGlobalBinding::callT3DLinkNative(const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexGlobalBinding method :callT3DLinkNative");
            return WeexBindingUtils::callT3DLinkNative(nativeObject, vars);
        }

        unicorn::ScopeValues WeexGlobalBinding::callNativeModule(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string instanceId;
            std::string module;
            std::string method;
            Args argument;
            Args options;


            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, instanceId);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, module);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, method);

            LOG_WEEX_BINDING("WeexRuntime: callNativeModule %s %s", module.c_str(), method.c_str());

            WeexConversionUtils::GetWsonFromArgs(vars, 3, argument);
            WeexConversionUtils::GetWsonFromArgs(vars, 4, options);


            auto result = this->nativeObject->js_bridge()->core_side()->CallNativeModule(
                    instanceId.c_str(),
                    module.c_str(),
                    method.c_str(),
                    argument.getValue(),
                    argument.getLength(),
                    options.getValue(),
                    options.getLength()
            );
            auto ret = WeexConversionUtils::WeexValueToRuntimeValue(GetEngineContext(), result.get());
//            LOG_WEEX_BINDING("end :callNativeModule instance:%s,module:%s,method:%s",
//                     instanceId.c_str(), module.c_str(), method.c_str());
//                    return ret;
            return ret;
        }

        unicorn::ScopeValues WeexGlobalBinding::callNativeComponent(
                const std::vector<unicorn::ScopeValues> &vars) {

            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string instanceId;
            std::string module;
            std::string method;
            Args argument;
            Args options;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, instanceId);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, module);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, method);

            LOG_WEEX_BINDING("WeexRuntime: callNativeComponent %s %s", module.c_str(), method.c_str());

            WeexConversionUtils::GetWsonFromArgs(vars, 3, argument);
            WeexConversionUtils::GetWsonFromArgs(vars, 4, options);

            this->nativeObject->js_bridge()->core_side()->CallNativeComponent(
                    instanceId.c_str(),
                    module.c_str(),
                    method.c_str(),
                    argument.getValue(),
                    argument.getLength(),
                    options.getValue(),
                    options.getLength()
            );
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callAddElement(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string page_id;
            std::string parent_ref;
            Args dom_str;
            std::string index_str;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, parent_ref);
            WeexConversionUtils::GetWsonFromArgs(vars, 2, dom_str);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 3, index_str);

            LOG_WEEX_BINDING("[WeexGlobalBinding] [AddElementAction] doc:%s,parent:%s,index:%s", page_id.c_str(),
                             parent_ref.c_str(),
                             index_str.c_str());

            nativeObject->js_bridge()->core_side()->AddElement(
                    page_id.c_str(), parent_ref.c_str(), dom_str.getValue(),
                    dom_str.getLength(), index_str.c_str());

            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callCreateBody(const std::vector<unicorn::ScopeValues> &vars) {

            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string page_id;
            Args dom_str;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetWsonFromArgs(vars, 1, dom_str);

            LOG_WEEX_BINDING("[WeexGlobalBinding] [sendCreateBodyAction] doc:%s", page_id.c_str());

            nativeObject->js_bridge()->core_side()->CreateBody(page_id.c_str(), dom_str.getValue(),
                                                               dom_str.getLength());
            return unicorn::RuntimeValues::MakeInt(0);
        }


        unicorn::ScopeValues WeexGlobalBinding::callUpdateFinish(const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexGlobalBinding method :callUpdateFinish");

            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string page_id;
            Args task_str;
            Args call_back_str;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetWsonFromArgs(vars, 1, task_str);
            WeexConversionUtils::GetWsonFromArgs(vars, 2, call_back_str);

            auto res = nativeObject->js_bridge()->core_side()->UpdateFinish(page_id.c_str(), task_str.getValue(),
                                                                            task_str.getLength(),
                                                                            call_back_str.getValue(),
                                                                            call_back_str.getLength());
            return unicorn::RuntimeValues::MakeInt(res);
        }

        unicorn::ScopeValues WeexGlobalBinding::callCreateFinish(const std::vector<unicorn::ScopeValues> &vars) {

            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string page_id;
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            LOG_WEEX_BINDING("[WeexGlobalBinding] [sendCreateFinish] doc:%s, ", page_id.c_str());
            nativeObject->js_bridge()->core_side()->CreateFinish(page_id.c_str());
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callRefreshFinish(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }


            std::string page_id;
            std::string task;
            std::string callBack;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, task);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, callBack);

            LOG_WEEX_BINDING("WeexGlobalBinding method :callRefreshFinish,argSize:%d, page:%s,task:%s,callback:%s",
                             vars.size(),
                             page_id.c_str(), task.c_str(), callBack.c_str()
            );

            auto result = nativeObject->js_bridge()->core_side()->RefreshFinish(page_id.c_str(), task.c_str(),
                                                                                callBack.c_str());
            return unicorn::RuntimeValues::MakeInt(result);
        }


        unicorn::ScopeValues WeexGlobalBinding::callUpdateAttrs(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            LOG_WEEX_BINDING("WeexGlobalBinding method :callUpdateAttrs");

            std::string page_id;
            std::string node_ref;
            Args dom_attrs;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, node_ref);
            WeexConversionUtils::GetWsonFromArgs(vars, 2, dom_attrs);

            nativeObject->js_bridge()->core_side()->UpdateAttrs(
                    page_id.c_str(),
                    node_ref.c_str(),
                    dom_attrs.getValue(),
                    dom_attrs.getLength()
            );
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callUpdateStyle(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }


            LOG_WEEX_BINDING("WeexGlobalBinding method :callUpdateStyle");

            std::string page_id;
            std::string node_ref;
            Args dom_styles;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, node_ref);
            WeexConversionUtils::GetWsonFromArgs(vars, 2, dom_styles);

            nativeObject->js_bridge()->core_side()->UpdateStyle(page_id.c_str(), node_ref.c_str(),
                                                                dom_styles.getValue(),
                                                                dom_styles.getLength());

            return unicorn::RuntimeValues::MakeInt(0);
        }


        unicorn::ScopeValues WeexGlobalBinding::callRemoveElement(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }


            LOG_WEEX_BINDING("WeexGlobalBinding method :callRemoveElement");

            std::string page_id;
            std::string node_ref;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, node_ref);

            vars[0]->GetAsString(&page_id);
            vars[1]->GetAsString(&node_ref);
            nativeObject->js_bridge()->core_side()->RemoveElement(page_id.c_str(), node_ref.c_str());
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callMoveElement(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }


            LOG_WEEX_BINDING("WeexGlobalBinding method :callMoveElement");

            std::string page_id;
            std::string node_ref;
            std::string parent_ref;
            std::string index_char;


            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, node_ref);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, parent_ref);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 3, index_char);

            nativeObject->js_bridge()->core_side()->MoveElement(
                    page_id.c_str(),
                    node_ref.c_str(),
                    parent_ref.c_str(),
                    atoi(index_char.c_str())
            );
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callAddEvent(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }


            LOG_WEEX_BINDING("WeexGlobalBinding method :callAddEvent");

            std::string page_id;
            std::string node_ref;
            std::string event;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, node_ref);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, event);

            nativeObject->js_bridge()->core_side()->AddEvent(
                    page_id.c_str(),
                    node_ref.c_str(),
                    event.c_str()
            );
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues WeexGlobalBinding::callRemoveEvent(const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }


            LOG_WEEX_BINDING("WeexGlobalBinding method :callRemoveEvent");
            std::string page_id;
            std::string node_ref;
            std::string event;


            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, node_ref);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 2, event);

            nativeObject->js_bridge()->core_side()->RemoveEvent(
                    page_id.c_str(),
                    node_ref.c_str(),
                    event.c_str()
            );
            return unicorn::RuntimeValues::MakeInt(0);
        }


        unicorn::ScopeValues WeexGlobalBinding::setTimeoutNative(
                const std::vector<unicorn::ScopeValues> &vars) {

            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeUndefined();
            }

            std::string callback_str;
            std::string time_str;

            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 0, callback_str);
            WeexConversionUtils::GetStringFromArgsDefaultEmpty(vars, 1, time_str);

            LOG_WEEX_BINDING("WeexGlobalBinding method :setTimeoutNative ,callback:%s, time:%d",
                             callback_str.c_str(),
                             time_str.c_str()
            );

            nativeObject->js_bridge()->core_side()->SetTimeout(
                    callback_str.c_str(),
                    time_str.c_str()
            );
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues
        WeexGlobalBinding::nativeLog(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::nativeLog(this->nativeObject, vars, true);
        }

        unicorn::ScopeValues WeexGlobalBinding::notifyTrimMemory(
                const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexGlobalBinding method :notifyTrimMemory");

//            JSLockHolder lock(exec);
//            // exec->heap()->collectAllGarbage();
//            return JSValue::encode(jsNumber(exec->heap()->sizeAfterLastFullCollection()));

            return unicorn::RuntimeValues::MakeUndefined();
        }

        unicorn::ScopeValues
        WeexGlobalBinding::markupState(const std::vector<unicorn::ScopeValues> &vars) {
            LOG_WEEX_BINDING("WeexGlobalBinding method :markupState");
            //not impl in old logic
            return unicorn::RuntimeValues::MakeUndefined();
        }

        unicorn::ScopeValues
        WeexGlobalBinding::atob(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::atob(this->nativeObject, vars);
        }

        unicorn::ScopeValues
        WeexGlobalBinding::btoa(const std::vector<unicorn::ScopeValues> &vars) {
            return WeexBindingUtils::btoa(this->nativeObject, vars);
        }

        unicorn::ScopeValues WeexGlobalBinding::__updateComponentData(
                const std::vector<unicorn::ScopeValues> &vars) {

            return WeexBindingUtils::__updateComponentData(this->nativeObject, vars);

        }
    }

}
