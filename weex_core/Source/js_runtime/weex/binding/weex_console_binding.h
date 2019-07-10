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

#ifndef PROJECT_WEEX_CONSOLE_BINDING_H
#define PROJECT_WEEX_CONSOLE_BINDING_H

#include "js_runtime/runtime/runtime_object.h"
#include "js_runtime/runtime/binding_macro.h"

namespace weex {
    namespace jsengine {
        class WeexConsoleBinding : public unicorn::RuntimeObject {

        public:
            DECLARE_CLASS_REGISTER_OP(WeexConsoleBinding)

            WeexConsoleBinding(unicorn::EngineContext *context, const OpaqueJSContext *js_ctx);

            unicorn::ScopeValues log(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues info(std::vector<unicorn::ScopeValues> &vars);


            unicorn::ScopeValues debug(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues warn(std::vector<unicorn::ScopeValues> &vars);

            unicorn::ScopeValues error(std::vector<unicorn::ScopeValues> &vars);

        private:
            unicorn::ScopeValues printLog(int level, std::vector<unicorn::ScopeValues> &vars);

        };
    }
}


#endif //PROJECT_WEEX_CONSOLE_BINDING_H
