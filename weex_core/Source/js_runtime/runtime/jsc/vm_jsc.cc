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
#include <unordered_map>

#include <JavaScriptCore/JavaScript.h>
#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/runtime_context.h"
#include "js_runtime/runtime/runtime_vm.h"

namespace unicorn {

std::unordered_map<JSContextRef, EngineContext*> g_context_map;

// implemete engine vm create function
void* RuntimeVM::CreateEngineVM() {
  // initialize jsc threading environment
  // create vm
  JSContextGroupRef context_group = JSContextGroupCreate();
  return static_cast<void*>(const_cast<OpaqueJSContextGroup*>(context_group));
}

void RuntimeVM::ReleaseEngineVM(void* vm) {
  if (vm == nullptr) return;
  JSContextGroupRelease(static_cast<JSContextGroupRef>(vm));
}

void RuntimeVM::NotifyContextCreated(RuntimeContext* context) {
  if (context == nullptr)
    return;

  EngineContext* engine_ctx = context->GetEngineContext();
  JSContextRef js_ctx = static_cast<JSContextRef>(engine_ctx->GetContext());
  g_context_map[js_ctx] = engine_ctx;
}

void RuntimeVM::NotifyContextDestorying(RuntimeContext* context) {
  if (context == nullptr)
    return;
  EngineContext* engine_ctx = context->GetEngineContext();
  JSContextRef js_ctx = static_cast<JSContextRef>(engine_ctx->GetContext());
  g_context_map.erase(js_ctx);
}

EngineContext* RuntimeVM::GetEngineContext(const void* js_ctx) {
  if (js_ctx == nullptr)
    return nullptr;

  JSContextRef ctx = static_cast<JSContextRef>(js_ctx);
  if (g_context_map.find(ctx) == g_context_map.end())
    return nullptr;

  return g_context_map[ctx];
}

}  // namespace unicorn
