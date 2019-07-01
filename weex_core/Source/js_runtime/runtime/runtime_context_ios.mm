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

#include "js_runtime/runtime/runtime_context_ios.h"

#include <cstdlib>

namespace unicorn {

RuntimeContextIOS::RuntimeContextIOS(RuntimeVM* vm) : vm_(vm), initialized_(false) {
  CreateJavaScriptContext();
}

RuntimeContextIOS::~RuntimeContextIOS() { vm_ = nullptr; }

void RuntimeContextIOS::CreateJavaScriptContext() {
  // TODO
  engine_context_.reset(EngineContext::CreateEngineContext(this, vm_));
  RuntimeVM::NotifyContextCreated(this);
}

void RuntimeContextIOS::ReleaseJavaScriptContext() {
  RuntimeVM::NotifyContextDestorying(this);
  engine_context_.reset();
}

void RuntimeContextIOS::SetName(const std::string& name) {}

void RuntimeContextIOS::InitializeContext() {}

void RuntimeContextIOS::UpdateSetting(const std::string& name) {}

bool RuntimeContextIOS::ExecuteJavaScript(const std::string& content) {
  if (!engine_context_)
    return false;
  engine_context_->RunJavaScript(content);
  return true;
}

std::unique_ptr<RuntimeValues> RuntimeContextIOS::ExecuteJavaScriptWithResult(
    const std::string& content) {
  // TODO
  return std::make_unique<RuntimeValues>(
      new RuntimeValues(nullptr));
}

void RuntimeContextIOS::NotifyIdle() {}

bool RuntimeContextIOS::Available() {
  return engine_context_ ? true : false ;
}

void RuntimeContextIOS::Destroy() {}

void RuntimeContextIOS::OnEngineContextInitialized() {
  if (!initialized_) initialized_ = true;
}

void RuntimeContextIOS::OnEngineContextFinalized() { initialized_ = false; }

std::unique_ptr<RuntimeContext> RuntimeContext::Create(RuntimeVM* vm) {
  return std::unique_ptr<RuntimeContext>(
              static_cast<RuntimeContext*>(new RuntimeContextIOS(vm)));
}

}  // namespace unicorn
