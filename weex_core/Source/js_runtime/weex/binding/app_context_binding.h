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

#ifndef PROJECT_APP_INSTANCE_BINDING_H
#define PROJECT_APP_INSTANCE_BINDING_H

#include "js_runtime/runtime/binding_macro.h"
#include "weex_console_binding.h"

class WeexGlobalObjectV2;

namespace weex {
    namespace jsengine {
        class AppWorkerBinding : public unicorn::RuntimeObject {
        public:
            DECLARE_CLASS_REGISTER_OP(AppWorkerBinding)

            AppWorkerBinding(unicorn::EngineContext *context, const OpaqueJSContext *js_ctx);

            ~AppWorkerBinding() override;

            unicorn::ScopeValues nativeLog(const std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            __dispatch_message__(const std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            __dispatch_message_sync__(const std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            postMessage(const std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            setNativeTimeout(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            setNativeInterval(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            clearNativeTimeout(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues
            clearNativeInterval(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues console();

        public:
            WeexGlobalObjectV2* nativeObject = nullptr;
            std::unique_ptr<WeexConsoleBinding> consoleBinding;
        };
    }
}


#endif //PROJECT_APP_INSTANCE_BINDING_H
