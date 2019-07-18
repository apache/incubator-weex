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

#ifndef PROJECT_JSON_BINDING_UTILS_H
#define PROJECT_JSON_BINDING_UTILS_H

#include "include/WeexApiHeader.h"
#include "js_runtime/runtime/runtime_context.h"
//#include "js_runtime/dom/impl/weex_element.h"
#include "third_party/json11/json11.hpp"
#include "js_runtime/runtime/runtime_values.h"
#include "js_runtime/weex/object/args.h"

namespace weex {
    namespace jsengine {
        class WeexConversionUtils {
        public:
            //  static json11::Json convertElementToJSon(const Element *element);

            static bool convertKVToJSon(const std::string &name, const ::std::string &value, std::string &result);


            static unicorn::ScopeValues
            WeexValueToRuntimeValue(unicorn::EngineContext *context, VALUE_WITH_TYPE *weexValue);

            static void ConvertRunTimeValueToWeexJSResult(unicorn::ScopeValues &value, WeexJSResult *jsResult);


            static void GetStringFromArgsDefaultEmpty(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                      std::string &result);

            static void GetStringFromArgsDefaultUndefined(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                          std::string &result);

            static bool
            GetCharOrJsonFromArgs(const std::vector<unicorn::ScopeValues> &vars, int index, std::string &result);

            static void
            GetWsonFromArgs(const std::vector<unicorn::ScopeValues> &vars, int index, Args &args);

            static void GetJSONArgsFromArgsByWml(const std::vector<unicorn::ScopeValues> &vars, int index,
                                                 std::string &args);

        private:

            static void ConvertRunTimeVaueToWson(unicorn::RuntimeValues *value, Args &args);

            static json11::Json RunTimeValuesOfObjectToJson(unicorn::RuntimeValues *vars);

            static void convertJSRuntimeValueToStdString(const unicorn::ScopeValues &param, std::string &target);

        };

    }
}


#endif //PROJECT_JSON_BINDING_UTILS_H
