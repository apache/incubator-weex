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
// Created by chenpeihan on 2019/3/4.
//

#include "weex_console_binding.h"

namespace weex {
    namespace jsengine {


        CLASS_METHOD_CALLBACK(WeexConsoleBinding, log)

        CLASS_METHOD_CALLBACK(WeexConsoleBinding, info)

        CLASS_METHOD_CALLBACK(WeexConsoleBinding, debug)

        CLASS_METHOD_CALLBACK(WeexConsoleBinding, warn)

        CLASS_METHOD_CALLBACK(WeexConsoleBinding, error)

        CLASS_REGISTER_START(WeexConsoleBinding, console)
            REGISTER_METHOD_CALLBACK(WeexConsoleBinding, log)
            REGISTER_METHOD_CALLBACK(WeexConsoleBinding, info)
            REGISTER_METHOD_CALLBACK(WeexConsoleBinding, debug)
            REGISTER_METHOD_CALLBACK(WeexConsoleBinding, warn)
            REGISTER_METHOD_CALLBACK(WeexConsoleBinding, error)
        CLASS_REGISTER_END(WeexConsoleBinding)


        WeexConsoleBinding::WeexConsoleBinding(unicorn::EngineContext *context, const OpaqueJSContext *js_ctx)
                : RuntimeObject(context, js_ctx) {
            SetJSClass(WeexConsoleBinding::CreateClassRef(context));
        }

//        enum class MessageLevel {
//            Log = 1,
//            Warning = 2,
//            Error = 3,
//            Debug = 4,
//            Info = 5,
//        };


        unicorn::ScopeValues WeexConsoleBinding::info(std::vector<unicorn::ScopeValues> &vars) {
            return printLog(5, vars);
        }

        unicorn::ScopeValues WeexConsoleBinding::debug(std::vector<unicorn::ScopeValues> &vars) {
            return printLog(4, vars);
        }

        unicorn::ScopeValues WeexConsoleBinding::warn(std::vector<unicorn::ScopeValues> &vars) {
            return printLog(2, vars);
        }

        unicorn::ScopeValues WeexConsoleBinding::error(std::vector<unicorn::ScopeValues> &vars) {
            return printLog(3, vars);
        }

        unicorn::ScopeValues WeexConsoleBinding::log(std::vector<unicorn::ScopeValues> &vars) {
            return printLog(1, vars);
        }

        unicorn::ScopeValues WeexConsoleBinding::printLog(int level, std::vector<unicorn::ScopeValues> &vars) {

            if (vars.size() < 1 || !vars[0]->IsString()) {
                return unicorn::RuntimeValues::MakeUndefined();
            }
            std::string log;
            vars[0]->GetAsString(&log);
            Weex::LogUtil::ConsoleLogPrint(level, "jsLog", log.c_str());
            return unicorn::RuntimeValues::MakeUndefined();
        }
    }
}

