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

#ifndef FLUTTER_UNICORN_RUNTIME_V8_ENGINE_CONTEXT_V8_H_
#define FLUTTER_UNICORN_RUNTIME_V8_ENGINE_CONTEXT_V8_H_

#include <v8.h>
#include <memory>
#include <string>

#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/runtime_values.h"

namespace unicorn {

class RuntimeVM;

class EngineContextV8 : public EngineContext {
 public:
  EngineContextV8(EngineContextDelegate* delegate, RuntimeVM* vm);
  ~EngineContextV8();

  void InitializeContext() override;
  void RunJavaScript(const std::string& script) override;
  void ThrowJSError(const std::string& error) override;
  void ThrowException(const std::string& error) override;
  void* GetContext() const override { return nullptr; }
  void SetGlobalPropertyValue(const std::string& property_id,
                              ScopeValues value);
  ScopeValues GetGlobalProperty(std::string property_id);

 private:
  v8::Local<v8::Context> get_context() {
    v8::Isolate* isolate = static_cast<v8::Isolate*>(vm_->EngineVM());
    return context_.Get(isolate);
  }

  EngineContextDelegate* delegate_;
  RuntimeVM* vm_;

  v8::Persistent<v8::Context> context_;
};

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_V8_ENGINE_CONTEXT_V8_H_
