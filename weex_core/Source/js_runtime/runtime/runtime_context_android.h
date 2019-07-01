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

#ifndef FLUTTER_UNICORN_RUNTIME_RUNTIME_CONTEXT_ANDROID_H_
#define FLUTTER_UNICORN_RUNTIME_RUNTIME_CONTEXT_ANDROID_H_

#include <memory>
#include <string>

#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/runtime_vm.h"
#include "js_runtime/runtime/runtime_context.h"

namespace unicorn {

class RuntimeContextAndroid : public RuntimeContext, EngineContextDelegate {
 public:
  explicit RuntimeContextAndroid(RuntimeVM* vm,JSRunTimeClass globalObjectClass);
  ~RuntimeContextAndroid() override;

  // override RuntimeContext
  void CreateJavaScriptContext(JSRunTimeClass globalClass) override;
  void ReleaseJavaScriptContext() override;
  void SetName(const std::string& name) override;
  void InitializeContext() override;
  void UpdateSetting(const std::string& settings) override;
  bool ExecuteJavaScript(const std::string &content,std::string *exception) override;
  ScopeValues ExecuteJavaScriptWithResult(const std::string &content,std::string *exception) override;

  EngineContext* GetEngineContext() const override { return engine_context_.get(); }

  // override EngineContextDelegate
  void OnEngineContextInitialized() override;
  void OnEngineContextFinalized() override;

  void NotifyIdle() override;
  bool Available() override;
  void Destroy() override;

 private:
  bool initialized_;
  RuntimeVM* vm_;
  std::string script_url_;
  std::unique_ptr<EngineContext> engine_context_;
};

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_RUNTIME_CONTEXT_ANDROID_H_
