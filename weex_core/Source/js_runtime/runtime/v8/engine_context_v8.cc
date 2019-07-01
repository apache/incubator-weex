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

#include "js_runtime/runtime/v8/engine_context_v8.h"

#include "js_runtime/runtime/runtime_controller.h"
#include "js_runtime/runtime/runtime_vm.h"

namespace unicorn {

EngineContextV8::EngineContextV8(EngineContextDelegate* delegate, RuntimeVM* vm)
    : delegate_(delegate), vm_(vm) {}

EngineContextV8::~EngineContextV8() {
  JSGlobalContextRelease(context_);

  if (delegate_) delegate_->OnEngineContextFinalized();
  delegate_ = NULL;
}

void EngineContextV8::InitializeContext() {
  v8::Isolate* isolate = static_cast<v8::Isolate*>(vm_->EngineVM());
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  auto context = Context::New(isolate, NULL, global);
  context_.reset(isolate, context);
}

void EngineContextV8::RunJavaScript(const std::string& script) {}

void EngineContextV8::ThrowJSError(const std::string& error) {}

void EngineContextV8::ThrowException(const std::string& error) {}

void EngineContextV8::SetGlobalPropertyValue(const std::string& property_id,
                                             ScopeValues value) {

}

ScopeValues EngineContextV8::GetGlobalProperty(std::string property_id) {
  return RuntimeValues::MakeUndefined();
}

EngineContext* EngineContext::CreateEngineContext(
    EngineContextDelegate* delegate, RuntimeVM* vm) {
  return new EngineContextV8(delegate, vm);
}
}  // namespace unicorn
