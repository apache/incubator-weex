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

#include "js_runtime/runtime/jsc/jsc_utils.h"
#include "js_runtime/utils/log_utils.h"

namespace unicorn {

RuntimeObject::~RuntimeObject() {
   if (nullptr != this->object_ && nullptr != this->context_){
     JSObjectSetPrivate(object_,nullptr);
   }
  //LOGE("[release] ~RuntimeObject() this:%p", this);
}

void RuntimeObject::SetParentJSClass(JSRunTimeClass parent_class) {
  parent_class_ = parent_class;
}

bool RuntimeObject::HasProperty(const std::string& name) {
  if (engine_context_) {
    return JSUtils::HasProperty(context_, object_, name);
  }
  return false;
}

void RuntimeObject::SetProperty(const std::string& name, JSRunTimeClass class_ref,
                                ScopeValues value) {
  if (HasProperty(name))
    return;

  JSUtils::SetProperty(context_, object_,
                       class_ref, name, value.get());
}

void RuntimeObject::SetProperty(const std::string& name, RuntimeObject* value) {
  if (value == nullptr || HasProperty(name) || engine_context_ == nullptr)
    return;

  auto holder = RuntimeValues::MakeCommonObject(
                               static_cast<void*>(value), nullptr);
  SetProperty(name, value->GetJSClass(), std::move(holder));
}

ScopeValues RuntimeObject::GetPropertyValue(const std::string& name) {
  if (!HasProperty(name))
    return RuntimeValues::MakeUndefined();

  return JSUtils::GetProperty(context_, object_, name);
}

void RuntimeObject::GetPropertyNameArray(std::vector<std::string>& array) {
  if (engine_context_ == nullptr)
    return;
  JSUtils::GetPropertyNameArray(context_, object_, array);
}

bool RuntimeObject::DeleteProperty(const std::string& name) {
  if (!HasProperty(name))
    return false;

  return JSUtils::DeleteProperty(context_, object_, name);
}

}  // namespace unicorn
