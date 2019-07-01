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

#ifndef FLUTTER_UNICORN_RUNTIME_JSC_ENGINE_CONTEXT_JSC_H_
#define FLUTTER_UNICORN_RUNTIME_JSC_ENGINE_CONTEXT_JSC_H_

#include "JavaScriptCore/JavaScript.h"
#include <memory>
#include <string>
#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/runtime_values.h"

namespace unicorn {

    class RuntimeVM;

    class EngineContextJSC : public EngineContext {
    public:
        EngineContextJSC(EngineContextDelegate *delegate, RuntimeVM *vm, JSClassRef globalObjectClass);

        virtual ~EngineContextJSC();

        void InitializeContext(JSClassRef globalObjectClass) override;

        bool RunJavaScript(const std::string &script, std::string *exception) override;

        ScopeValues RunJavaScriptWithResult(const std::string &script, std::string *exception) override;

        JSValueRef callJavaScriptFunc(RuntimeObject *target, const std::string &name, std::vector<ScopeValues> &args,
                                      std::string *exception) override;


        void ThrowJSError(const std::string &error) override;

        void ThrowException(const std::string &error) override;

        void *GetContext() const override { return static_cast<void *>(context_); }

        void SetGlobalPropertyValue(const std::string &property_id,
                                    ScopeValues value) override;

        JSObjectRef GetGlobalObjectInContext() override;

        void BindDataToObject(JSObjectRef targetObj, void *data) override;

        void removeObjectBindData(JSObjectRef targetObj) override;

        ScopeValues GetGlobalProperty(std::string property_id) override;

        JSValueRef GetPropertyValueFromObject(std::string property_id, JSObjectRef object) override;


        bool SetObjectPrototypeFromValue(JSObjectRef target, JSValueRef referenceValue) override;

        JSValueRef GetObjectPrototype(JSObjectRef referenceObject) override;

        JSObjectRef toObjectFromValue(JSValueRef js_value) override;


        bool GetObjectPropertyNameArray(JSRunTimeObject object, std::vector<std::string> &nameArrays) override;

        bool setObjectValue(JSRunTimeObject targetObject, const std::string &key, JSRunTimeValue value) override;

        void SetName(const std::string &name) override;

        std::string GetName() override;

         ScopeValues
         CallJavaScriptFuncWithRuntimeValue(RuntimeObject *target, const std::string &name, std::vector<ScopeValues> &args,
                                           std::string *exception)override ;


    private:
        EngineContextDelegate *delegate_;
        RuntimeVM *vm_;

        JSGlobalContextRef context_;

        std::string context_name;
    };

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_JSC_ENGINE_CONTEXT_JSC_H_
