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
// Created by chenpeihan on 2019/2/12.
//

#include <js_runtime/weex/utils/weex_conversion_utils.h>
#include "weex_binding_utils.h"
#include "js_runtime/utils/log_utils.h"
#include "core/bridge/script_bridge.h"
#include "js_runtime/weex/object/weex_global_object_v2.h"
#include "js_runtime/utils/base64.hpp"

namespace weex {
    namespace jsengine {
        unicorn::ScopeValues
        WeexBindingUtils::atob(WeexGlobalObjectV2 *nativeObject,
                               const std::vector<unicorn::ScopeValues> &vars) {
            if (vars.size() > 0 && vars[0]->IsString()) {
                std::string origin_data;
                vars[0]->GetAsString(&origin_data);
                std::string res = beast::detail::base64_decode(origin_data);
                return unicorn::RuntimeValues::MakeString(res);
            } else {
                return unicorn::RuntimeValues::MakeUndefined();
            }

        }

        unicorn::ScopeValues
        WeexBindingUtils::btoa(WeexGlobalObjectV2 *nativeObject,
                               const std::vector<unicorn::ScopeValues> &vars) {
            std::string origin_data;

            if (vars.size() > 0 && vars[0]->IsString()) {
                vars[0]->GetAsString(&origin_data);
            } else {
                origin_data = "";
            }
            std::string res = beast::detail::base64_encode(origin_data);
            return unicorn::RuntimeValues::MakeString(res);
        }

        unicorn::ScopeValues WeexBindingUtils::nativeLog(WeexGlobalObjectV2 *nativeObject,
                                                         const std::vector<unicorn::ScopeValues> &vars,
                                                         bool toCoreSide) {
            // LOG_JS("[JS] enter native log :args->size %d", vars.size());
            if (vars.empty()) {
                unicorn::RuntimeValues::MakeBool(true);
            }
            std::string logStr;
            for (int i = 0; i < vars.size(); i++) {
                std::string logItem;
                WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, i, logItem);
                vars[i]->GetAsString(&logItem);
                logStr.append(logItem);
            }

           // LOG_JS("[WeexBindingUtils][JS] %s", logStr.c_str());
            if (toCoreSide && !logStr.empty()) {
                nativeObject->js_bridge()->core_side()->NativeLog(logStr.c_str());
            }
            return unicorn::RuntimeValues::MakeUndefined();
        }

        unicorn::ScopeValues WeexBindingUtils::setNativeTimeout(WeexGlobalObjectV2 *nativeObject,
                                                                std::vector<unicorn::ScopeValues> &vars,
                                                                bool interval) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeInt(0);
            }
            if (vars.size() < 2 || !vars[0]->IsFunction() || !vars[1]->IsNumber()) {
                if (vars.size() < 2) {
                    LOG_WEEX_BINDING("WeexBindingUtils method :setNativeTimeout argsSize check failed, argSize:%d",
                                     vars.size());
                } else {
                    LOG_WEEX_BINDING(
                            "WeexBindingUtils method :setNativeTimeout argsType check failed, 1type::%d,2type:%d",
                            vars[0]->GetType(), vars[1]->GetType());
                }
                return unicorn::RuntimeValues::MakeInt(0);
            }
            unicorn::RuntimeValues *func = vars[0].release();
            int timeout;
            vars[1]->GetAsInteger(&timeout);
            int task_id = nativeObject->setNativeTimeout(func, timeout, interval);
            return unicorn::RuntimeValues::MakeInt(task_id);
        }


        unicorn::ScopeValues
        WeexBindingUtils::clearNativeTimeout(WeexGlobalObjectV2 *nativeObject,
                                             std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeInt(0);
            }
            if (vars.size() > 0 && vars[0]->IsNumber()) {
                int timerId;
                vars[0]->GetAsInteger(&timerId);
                nativeObject->clearNativeTimeout(timerId);
            }
            return unicorn::RuntimeValues::MakeInt(0);
        }

        unicorn::ScopeValues
        WeexBindingUtils::callT3DLinkNative(WeexGlobalObjectV2 *nativeObject,
                                            const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeString(std::string(""));
            }


            int type;
            std::string arg_str;

            vars[0]->GetAsInteger(&type);
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 1, arg_str);

            LOG_WEEX_BINDING("WeexBindingUtils method :callT3DLinkNative type:%d, arg_str:%s", type, arg_str.c_str());

            auto result = nativeObject->js_bridge()->core_side()->CallT3DLinkNative(type, arg_str.c_str());
            std::string result_str = std::string(nullptr == result? "":result);
            return unicorn::RuntimeValues::MakeString(result_str);
        }

        unicorn::ScopeValues
        WeexBindingUtils::callGCanvasLinkNative(WeexGlobalObjectV2 *nativeObject,
                                                const std::vector<unicorn::ScopeValues> &vars) {
            if (nullptr == nativeObject) {
                return unicorn::RuntimeValues::MakeString(std::string(""));
            }

            std::string id_str;
            int type;
            std::string arg_str;

            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 0, id_str);
            vars[1]->GetAsInteger(&type);
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 2, arg_str);

            LOG_WEEX_BINDING("WeexBindingUtils callGCanvasLinkNative id:%s,type:%d,arg:%s",
                             id_str.c_str(),
                             type,
                             arg_str.c_str()
            );

            auto result = nativeObject->js_bridge()->core_side()->CallGCanvasLinkNative(
                    id_str.c_str(), type, arg_str.c_str()
            );

            if (nullptr == result) {
                return unicorn::RuntimeValues::MakeString(std::string(""));
            } else {
                return unicorn::RuntimeValues::MakeString(std::string(result));
            }
        }

        unicorn::ScopeValues
        WeexBindingUtils::__updateComponentData(WeexGlobalObjectV2 *nativeObject,
                                                const std::vector<unicorn::ScopeValues> &vars) {

            std::string page_id;
            std::string cid;
            std::string json_data;

            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 0, page_id);
            WeexConversionUtils::GetStringFromArgsDefaultUndefined(vars, 1, cid);
            bool succeed = WeexConversionUtils::GetCharOrJsonFromArgs(vars, 2, json_data);
            const char *json_data_char = succeed ? json_data.c_str() : nullptr;
            if(json_data_char == nullptr){
                nativeObject->js_bridge()->core_side()->ReportException(page_id.c_str(),"updateComponentData","parse json failed");
                return unicorn::RuntimeValues::MakeUndefined();
            }

            LOG_WEEX_BINDING("WeexGlobalBinding method :__updateComponentData page:%s, cid:%s,json_data:%s",
                             page_id.c_str(), cid.c_str(), json_data_char
            );

            nativeObject->js_bridge()->core_side()->UpdateComponentData(page_id.c_str(), cid.c_str(),
                                                                        json_data_char);
            return unicorn::RuntimeValues::MakeUndefined();
        }


    }
}
