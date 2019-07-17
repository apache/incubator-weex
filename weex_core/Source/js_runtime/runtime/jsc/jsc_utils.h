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
/**
# Copyright 2018 Taobao (China) Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
**/

#ifndef FLUTTER_UNICORN_RUNTIME_JSC_JSC_UTILS_H_
#define FLUTTER_UNICORN_RUNTIME_JSC_JSC_UTILS_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "JavaScriptCore/JavaScript.h"
#include "js_runtime/runtime/runtime_values.h"

namespace unicorn {

class Conversion {
 public:
  static JSValueRef RuntimeValueToJSValue(JSContextRef ctx,
                                          JSClassRef class_ref,
                                          const RuntimeValues* value);

  static ScopeValues JSValueToRuntimeValue(JSContextRef ctx,
                                           JSObjectRef thiz,
                                           JSValueRef value);

  static void JSValuesArrayToRuntimeValues(JSContextRef ctx, JSObjectRef thiz,
                                    size_t argc, const JSValueRef* argv,
                                    std::vector<ScopeValues>& output);


  static bool JSValueToStdString(JSContextRef ctx, JSValueRef value,std::string* result);

  static void printJSValueRefException(JSContextRef context,JSValueRef exc);


  static JSValueRef ParserUtf8CharJsonToJValueJSContextRef(JSContextRef ctx, const char* utf_8_str);

protected:
    static ScopeValues JSValueToRuntimeValueWithCircleCheck(JSContextRef ctx,
                                             JSObjectRef thiz,
                                             JSValueRef value,
                                             std::vector<JSValueRef > &stack_record, std::string &exception
    );
};

class JSUtils {
 public:
  static bool HasProperty(JSContextRef ctx, JSObjectRef ob,
                          const std::string& name);
  static void SetProperty(JSContextRef ctx, JSObjectRef ob,
                          JSClassRef clz, const std::string& name,
                          RuntimeValues* value);
  static ScopeValues GetProperty(JSContextRef ctx, JSObjectRef ob,
                                 const std::string& name);
  static bool DeleteProperty(JSContextRef ctx, JSObjectRef ob,
                             const std::string& name);
  static void GetPropertyNameArray(JSContextRef ctx, JSObjectRef ob,
                                   std::vector<std::string>& native_array);
  static ScopeValues CallAsFunction(JSContextRef ctx, JSObjectRef thiz,
                                    JSObjectRef function,
                                    size_t argc, const JSValueRef argv[]);
};

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_JSC_JSC_UTILS_H_
