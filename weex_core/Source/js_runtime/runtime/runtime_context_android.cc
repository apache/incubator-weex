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

#include "js_runtime/runtime/runtime_context_android.h"

#include <cstdlib>

namespace unicorn {

    RuntimeContextAndroid::RuntimeContextAndroid(RuntimeVM *vm,JSRunTimeClass globalObjectClass)
            : initialized_(false), vm_(vm), script_url_("") {
        engine_context_.reset(EngineContext::CreateEngineContext(this, vm_,globalObjectClass));
        RuntimeVM::NotifyContextCreated(this);
    }

    RuntimeContextAndroid::~RuntimeContextAndroid() {
        RuntimeVM::NotifyContextDestorying(this);
        if (engine_context_) {
            engine_context_.reset();
        }
    }

    void RuntimeContextAndroid::CreateJavaScriptContext(JSRunTimeClass globalObjectClass) {
        engine_context_.reset(EngineContext::CreateEngineContext(this, vm_,globalObjectClass));
        RuntimeVM::NotifyContextCreated(this);
    }

    void RuntimeContextAndroid::ReleaseJavaScriptContext() {
        RuntimeVM::NotifyContextDestorying(this);
        engine_context_.reset();
    }

    void RuntimeContextAndroid::SetName(const std::string &name) {}

    void RuntimeContextAndroid::InitializeContext() {}

    void RuntimeContextAndroid::UpdateSetting(const std::string &name) {}

    bool RuntimeContextAndroid::ExecuteJavaScript(const std::string &content,std::string *exception) {
        return engine_context_->RunJavaScript(content,exception);
    }

    ScopeValues RuntimeContextAndroid::ExecuteJavaScriptWithResult(const std::string &content,std::string *exception) {
        return engine_context_->RunJavaScriptWithResult(content,exception);
    }

    void RuntimeContextAndroid::OnEngineContextInitialized() {
        if (!initialized_) initialized_ = true;
    }

    void RuntimeContextAndroid::OnEngineContextFinalized() { initialized_ = false; }

    void RuntimeContextAndroid::NotifyIdle() {
        // TODO(unicorn): we can do gc worker when idle
    }

    bool RuntimeContextAndroid::Available() { return initialized_; }

    void RuntimeContextAndroid::Destroy() {}

    std::unique_ptr<RuntimeContext> RuntimeContext::Create(RuntimeVM *vm,JSRunTimeClass globalObjectClass) {
        return std::unique_ptr<RuntimeContext>(
                static_cast<RuntimeContext *>(new RuntimeContextAndroid(vm,globalObjectClass)));
    }

}  // namespace unicorn
