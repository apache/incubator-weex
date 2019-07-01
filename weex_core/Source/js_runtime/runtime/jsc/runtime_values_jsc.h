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

#ifndef FLUTTER_UNICORN_RUNTIME_JSC_RUNTIME_VALUES_JSC_H_
#define FLUTTER_UNICORN_RUNTIME_JSC_RUNTIME_VALUES_JSC_H_

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "JavaScriptCore/JavaScript.h"
#include "js_runtime/runtime/runtime_values.h"
#include "JavaScriptCore/JSObjectRef.h"
namespace unicorn {

class JSCFunction : public Function {
 public:
  static std::unique_ptr<Function>
  CreateWithCallback(JSContextRef ctx, std::string name,
                     JSObjectRef thiz,
                     JSObjectCallAsFunctionCallback callback);

  static std::unique_ptr<Function> Create(JSContextRef ctx, std::string name,
                                   JSObjectRef thiz, JSObjectRef function);

  JSCFunction(JSContextRef ctx, std::string name, JSObjectRef thiz,
              JSObjectCallAsFunctionCallback callback);

  JSCFunction(JSContextRef ctx, std::string name, JSObjectRef thiz,
              JSObjectRef function);
  ~JSCFunction();

  bool IsFromNative() override { return is_from_native_; };

  std::unique_ptr<RuntimeValues> Call(JSContextRef cur_context,size_t argc,
                            std::vector<ScopeValues>& argv) const override;

  JSObjectRef GetFunction() const { return function_; }

 private:
  void MemberProtect();
  void MemberUnprotect();

  JSContextRef context_;
  std::string function_name_;
  JSObjectRef thiz_;
  JSObjectRef function_;
  bool is_from_native_;
};

class JSCMap : public Map {
 public:
  static std::unique_ptr<Map> Create(JSContextRef ctx, JSObjectRef thiz);

  JSCMap(JSContextRef ctx, JSObjectRef thiz);
  ~JSCMap();

  JSObjectRef GetThisObject() const { return thiz_; }

 private:
  JSContextRef context_;
  JSObjectRef thiz_;
};

class JSCArray : public Array {
 public:
  static std::unique_ptr<Array> Create(JSContextRef ctx, JSObjectRef thiz);

  JSCArray(JSContextRef ctx, JSObjectRef thiz);
  ~JSCArray();

  JSObjectRef GetThisObject() const { return thiz_; }

 private:
  JSContextRef context_;
  JSObjectRef thiz_;
};

}  // namespace unicorn
#endif  // FLUTTER_UNICORN_RUNTIME_JSC_RUNTIME_VALUES_JSC_H_
