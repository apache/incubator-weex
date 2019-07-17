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

#ifndef FLUTTER_UNICORN_RUNTIME_ENGINE_CONTEXT_H_
#define FLUTTER_UNICORN_RUNTIME_ENGINE_CONTEXT_H_

#include <cstdlib>
#include <memory>
#include <string>
#include "js_runtime/runtime/runtime_values.h"

namespace unicorn {
    class RuntimeVM;

    class EngineContextDelegate {
    public:
        virtual void OnEngineContextInitialized() = 0;

        virtual void OnEngineContextFinalized() = 0;
    };

    class EngineContext {
    public:
        static EngineContext *CreateEngineContext(EngineContextDelegate *delegate,
                                                  RuntimeVM *vm, JSRunTimeClass globalClass);

        virtual ~EngineContext() {}

        virtual void InitializeContext(JSRunTimeClass globalObjectClass) = 0;

        virtual void SetName(const std::string& name) = 0;

        virtual std::string GetName() = 0;

        virtual JSRunTimeObject GetGlobalObjectInContext() =0;

        virtual void BindDataToObject(JSRunTimeObject targetObj,void* data) = 0;

        virtual void removeObjectBindData(JSRunTimeObject targetObj) = 0;

        virtual bool RunJavaScript(const std::string &script, std::string *exception) = 0;

        virtual ScopeValues RunJavaScriptWithResult(const std::string &script, std::string *exception) = 0;

        virtual JSRunTimeValue
        callJavaScriptFunc(RuntimeObject *target, const std::string &name, std::vector<ScopeValues> &args,
                           std::string *exception) = 0;

        virtual ScopeValues
        CallJavaScriptFuncWithRuntimeValue(RuntimeObject *target, const std::string &name, std::vector<ScopeValues> &args,
                           std::string *exception) = 0;

        virtual void ThrowJSError(const std::string &error) = 0;

        virtual void ThrowException(const std::string &error) = 0;

        virtual void *GetContext() const { return nullptr; }

        virtual void SetGlobalPropertyValue(const std::string &property_id, ScopeValues value) = 0;

        virtual ScopeValues GetGlobalProperty(std::string property_id) = 0;

        virtual JSRunTimeValue GetPropertyValueFromObject(std::string property_id, JSRunTimeObject object) = 0;

        virtual bool GetObjectPropertyNameArray(JSRunTimeObject object,std::vector<std::string> &nameArrays) = 0;

        virtual bool setObjectValue(JSRunTimeObject targetObject, const std::string &key, JSRunTimeValue value) = 0;

        /**
         * @abstract The JSObject whose prototype you want to set.
         * @param target the object you want change Prototype, if null,use globalObject
         * @param value JSValue to set as the object's prototype. if null,use globalObjectValue
         * @return success or false
         */
        virtual bool SetObjectPrototypeFromValue(JSRunTimeObject target, JSValueRef value) = 0;

        virtual JSRunTimeValue GetObjectPrototype(JSRunTimeObject referenceValue) = 0;

        virtual JSRunTimeObject toObjectFromValue(JSRunTimeValue js_value) = 0;
    };

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_ENGINE_CONTEXT_H_
