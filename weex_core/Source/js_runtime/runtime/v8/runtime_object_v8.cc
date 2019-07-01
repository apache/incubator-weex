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

#include "js_runtime/runtime/runtime_object.h"

#include "js_runtime/runtime/engine_context.h"

namespace unicorn {

bool RuntimeObject::HasProperty(std::string& name) {
  return false;
}

void RuntimeObject::SetProperty(std::string name&, void* class_ref,
                                RuntimeValues& value) {
  if (value == nullptr || HasProperty(name))
    return;
}
void RuntimeObject::SetProperty(std::string& name, RuntimeObject* value) {
  if (value == nullptr || HasProperty(name))
    return;

  RuntimeValues holder(static_cast<void*>(value));
  SetProperty(name, value->GetJSClass(), holder);
}

RuntimeValues RuntimeObject::GetPropertyValue(std::string& name) {
  return RuntimeValues();
}

bool RuntimeObject::DeleteProperty(std::string& name) {
  return false;
}

}  // namespace unicorn
