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

#include "js_runtime/runtime/runtime_vm.h"

#include <sys/stat.h>
#include <mutex>
#include <vector>

namespace unicorn {

//static std::once_flag gVMInitialization;
//static RuntimeVM* gRVM;
//
//RuntimeVM* RuntimeVM::ForProcess() {
//  std::call_once(gVMInitialization, []() mutable { gRVM = new RuntimeVM(); });
//
//  return gRVM;
//}

    RuntimeVM::RuntimeVM() {
        this->vm_ = RuntimeVM::CreateEngineVM();
    }

    RuntimeVM::~RuntimeVM() {
        if (this->vm_) {
            RuntimeVM::ReleaseEngineVM(this->vm_);
            this->vm_ = nullptr;
        }
    }

    void *RuntimeVM::EngineVM() { return RuntimeVM::vm_; }

    bool RuntimeVM::Shutdown() {
        RuntimeVM::ReleaseEngineVM(this->vm_);
        this->vm_ = nullptr;
        return true;
    }

// private
    void RuntimeVM::Initialize() {
        // TODO(zhangxiao): for vm initializing
    }

}  // namespace unicorn
