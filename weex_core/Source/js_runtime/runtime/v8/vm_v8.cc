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

#include <v8.h>
#include "js_runtime/runtime/runtime_vm.h"

namespace unicorn {

// implemete engine vm create function
void* RuntimeVM::CreateEngineVM() {
  // initialize jsc threading environment
  // create vm
  v8::Isolate* isolate = v8::Isolate::New();
  return static_cast<void*>(isolate);
}

void RuntimeVM::ReleaseEngineVM(void* vm) {
  if (vm == nullptr) return;
  v8::Isolate* isolate = static_cast<v8::Isolate*>(vm);
  isolate->Dispose();
}

}  // namespace unicorn
