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

#ifndef FLUTTER_UNICORN_RUNTIME_RUNTIME_OBJECT_H_
#define FLUTTER_UNICORN_RUNTIME_RUNTIME_OBJECT_H_

#include <string>
#include <vector>
#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/runtime_values.h"
#include "JavaScriptCore/JSObjectRef.h"


namespace unicorn {

class RuntimeObject {
 public:
  explicit RuntimeObject(EngineContext* context, JSRunTimeContext js_ctx = nullptr)
                        : engine_context_(context) {
    // if js_ctx is null, so we use global context
    if (js_ctx == nullptr && engine_context_)
      context_ = static_cast<JSRunTimeContext>(engine_context_->GetContext());
    else
      context_ = js_ctx;
  }
  virtual ~RuntimeObject();

  virtual void SetEngineContext(EngineContext* context) {
    engine_context_ = context;
    if (context_ == nullptr && context)
      context_ = static_cast<JSRunTimeContext>(engine_context_->GetContext());
  }
  virtual EngineContext* GetEngineContext() const { return engine_context_; }
  virtual void SetParentJSClass(JSRunTimeClass parent_class);
  virtual JSRunTimeClass ParentJSClass() { return parent_class_; }
  virtual void SetJSClass(JSRunTimeClass class_ref) { class_ = class_ref; }
  virtual JSRunTimeClass GetJSClass() { return class_; }
  virtual void SetJSObject(JSRunTimeObject object) { object_ = object;}
  virtual JSRunTimeObject GetJSObject() { return object_; }
  virtual bool HasProperty(const std::string& name);
  virtual void SetProperty(const std::string& name, JSRunTimeClass clz,
                           ScopeValues value);
  virtual void SetProperty(const std::string& name, RuntimeObject* value);
  virtual ScopeValues GetPropertyValue(const std::string& name);
  virtual void GetPropertyNameArray(std::vector<std::string>& array);
  virtual bool DeleteProperty(const std::string& name);
  // this function is used after your member functuon called by javascript
  // if you want to do something to the jsobject, please override this
  // funciton
  virtual void OnMethodCalledByJS() {}

 private:
  EngineContext* engine_context_;
  JSRunTimeClass parent_class_{nullptr};
  JSRunTimeClass class_{nullptr};
  JSRunTimeObject object_{nullptr};
  JSRunTimeContext context_{nullptr};
};

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_RUNTIME_OBJECT_H_
