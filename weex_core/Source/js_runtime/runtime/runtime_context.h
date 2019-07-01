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

#ifndef FLUTTER_UNICORN_RUNTIME_RUNTIME_CONTEXT_H_
#define FLUTTER_UNICORN_RUNTIME_RUNTIME_CONTEXT_H_

#include <memory>
#include <string>
#include <vector>

#include "js_runtime/runtime/runtime_values.h"
#include "js_runtime/runtime/runtime_vm.h"

namespace unicorn {
class EngineContext;

class RuntimeContext {
 public:
  enum State {
    Unknown,
    Uninitialized,
    Initialized,
    Ready,
    Running,
    Destroyed,
  };
/**
 *
 * @param vm  runtime vm
 * @param globalObjectClass if not null,create as context's globalObject,else use default globalObject
 * @return context
 */
  static std::unique_ptr<RuntimeContext> Create(RuntimeVM* vm,JSRunTimeClass globalObjectClass);

  virtual ~RuntimeContext() {}
  virtual void CreateJavaScriptContext(JSRunTimeClass globalObjectClass) = 0;
  virtual void ReleaseJavaScriptContext() = 0;
  virtual void SetName(const std::string& name) = 0;
  virtual void InitializeContext() = 0;
  virtual EngineContext* GetEngineContext() const = 0;
  virtual void UpdateSetting(const std::string& settings) = 0;
  virtual bool ExecuteJavaScript(const std::string &content,std::string *exception) = 0;
  virtual std::unique_ptr<RuntimeValues> ExecuteJavaScriptWithResult(const std::string& name,std::string *exception) = 0;
  //virtual JSRunTimeValue callJavaScriptFunc(RuntimeObject* target,const std::string& name,std::vector<ScopeValues> &args,std::string *exception) = 0;

  virtual void NotifyIdle() = 0;
  virtual bool Available() = 0;
  virtual void Destroy() = 0;
};

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_RUNTIME_CONTEXT_H_
