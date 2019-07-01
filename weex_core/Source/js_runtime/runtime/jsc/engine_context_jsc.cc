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

#include "js_runtime/runtime/jsc/engine_context_jsc.h"

#include <memory>
#include <utility>
#include "js_runtime/runtime/jsc/jsc_utils.h"
#include "js_runtime/runtime/jsc/binding_macro_jsc.h"
#include "js_runtime/runtime/runtime_values.h"
#include "js_runtime/runtime/runtime_vm.h"
#include "js_runtime/utils/log_utils.h"

namespace unicorn {

    EngineContextJSC::EngineContextJSC(EngineContextDelegate *delegate,
                                       RuntimeVM *vm, JSClassRef globalObjectClassRef)
            : delegate_(delegate), vm_(vm) {
        InitializeContext(globalObjectClassRef);
    }

    EngineContextJSC::~EngineContextJSC() {
        JSGlobalContextRelease(context_);

        if (delegate_) delegate_->OnEngineContextFinalized();
        delegate_ = nullptr;

        LOGE("[release] release ctx :%p", context_);
    }

    void EngineContextJSC::InitializeContext(JSClassRef globalClassRef) {
        JSContextGroupRef context_group =
                static_cast<JSContextGroupRef>(vm_->EngineVM());
        context_ = JSGlobalContextCreateInGroup(context_group, globalClassRef);
        LOG_JS_RUNTIME("[Context] EngineContextJSC::create context :%p ，globalObject :%p", context_,
                       JSContextGetGlobalObject(context_));

        if (delegate_) delegate_->OnEngineContextInitialized();
    }

    bool EngineContextJSC::RunJavaScript(const std::string &script, std::string *runException) {
        LOG_JS_RUNTIME("EngineContextJSC:: RunJavaScript onContext :%p, script :%s", context_, script.c_str());

        JSStringRef source = JSStringCreateWithUTF8CString(script.c_str());


        JSValueRef exceptionRef = nullptr;
        JSEvaluateScript(context_, source, NULL, NULL, 0, &exceptionRef);
        JSStringRelease(source);
        if (exceptionRef && nullptr != runException) {
            Conversion::JSValueToStdString(context_,exceptionRef,runException);
            return false;
        }
        LOG_JS_RUNTIME("WeexRuntime:: RunJavaScript succeed this");
        return true;
    }

    ScopeValues EngineContextJSC::RunJavaScriptWithResult(const std::string &script, std::string *exception) {
        LOG_JS_RUNTIME("EngineContextJSC:: RunJavaScriptWithResult : %s", script.c_str());
        JSStringRef source = JSStringCreateWithUTF8CString(script.c_str());
        JSValueRef exceptionRef = nullptr;



        JSValueRef result = JSEvaluateScript(context_, source, NULL, NULL, 0, &exceptionRef);
        if (!exceptionRef) {
            Conversion::JSValueToStdString(context_, exceptionRef, exception);
        }

        JSStringRelease(source);
        return Conversion::JSValueToRuntimeValue(context_, nullptr, result);
    }


    void EngineContextJSC::ThrowJSError(const std::string &error) {}

    void EngineContextJSC::ThrowException(const std::string &error) {}

    void EngineContextJSC::SetGlobalPropertyValue(const std::string &property_id,
                                                  ScopeValues value) {
        JSObjectRef global = JSContextGetGlobalObject(context_);
        JSValueRef exc = nullptr;
        JSStringRef str_ref = JSStringCreateWithUTF8CString(property_id.c_str());
        JSObjectSetProperty(
                context_,
                global,
                str_ref,
                Conversion::RuntimeValueToJSValue(context_, nullptr, value.get()),
                kJSPropertyAttributeNone,
                &exc);
        JSStringRelease(str_ref);
    }

    ScopeValues EngineContextJSC::GetGlobalProperty(std::string property_id) {
        JSObjectRef global = JSContextGetGlobalObject(context_);
        JSValueRef exc = nullptr;
        JSStringRef str = JSStringCreateWithUTF8CString(property_id.c_str());
        JSValueRef result = JSObjectGetProperty(context_, global, str, &exc);
        if (exc) {
        }

        JSStringRelease(str);
        return Conversion::JSValueToRuntimeValue(context_, global, result);
    }

    EngineContext *EngineContext::CreateEngineContext(
            EngineContextDelegate *delegate, RuntimeVM *vm, JSClassRef globalObjectClass) {
        return new EngineContextJSC(delegate, vm, globalObjectClass);
    }

    JSValueRef
    EngineContextJSC::callJavaScriptFunc(RuntimeObject *target, const std::string &name, std::vector<ScopeValues> &args,
                                         std::string *exception) {
        const char *cName = name.c_str();
        JSStringRef funcNameStr = JSStringCreateWithUTF8CString(cName);
        JSValueRef result = nullptr;
        JSValueRef exc = nullptr;
        JSObjectRef targetObjectRef = nullptr;
        if (nullptr == target) {
            LOG_JS_RUNTIME("EngineContextJSC callJavaScriptFunc targetObjectRef GlobalObject :%s", cName);
            targetObjectRef = JSContextGetGlobalObject(context_);
        } else {
            // LOG_TEST("EngineContextJSC callJavaScriptFunc target->GetJSObject() :%s", cName);
            targetObjectRef = target->GetJSObject();
        }

        JSValueRef funcValueRef = JSObjectGetProperty(context_, targetObjectRef, funcNameStr, &exc);

        if(funcNameStr) {
            JSStringRelease(funcNameStr);
        }
        if (exc || JSValueIsUndefined(context_, funcValueRef)) {
            LOG_JS_RUNTIME("EngineContext can't find func Name :%s !!!!!!", cName);
            return nullptr;
        }

        JSObjectRef funcObjectRef = JSValueToObject(context_, funcValueRef, &exc);

        if (exc && JSValueIsString(context_, exc)) {
            Conversion::JSValueToStdString(context_, exc, exception);
        }

        if (exc || !funcObjectRef || !JSObjectIsFunction(context_, funcObjectRef)) {
            return nullptr;
        }

        //    LOG_TEST("EngineContextJSC deal func %s args, agrSize :%d", cName, args.size());
        JSValueRef argv_js[args.size()];
        for (size_t i = 0; i < args.size(); i++) {
            argv_js[i] = Conversion::RuntimeValueToJSValue(context_, nullptr, args[i].get());
        }

        //   LOG_TEST("EngineContextJSC  try call func  :%s", cName);
        result = JSObjectCallAsFunction(context_, funcObjectRef, targetObjectRef, args.size(), argv_js, &exc);

        if (!result || exc) {
            Conversion::JSValueToStdString(context_, exc, exception);
            return nullptr;
        }
        LOG_JS_RUNTIME("EngineContextJSC  call func  :%s succeed ~~~ ,return result :%d", cName, result);
        return result;
    }

    JSValueRef EngineContextJSC::GetPropertyValueFromObject(std::string property_id, JSRunTimeObject object) {
        if (nullptr == object) {
            LOG_JS_RUNTIME("EngineContextJSC GetPropertyValueFromObject check object failed from id:%s",
                           property_id.c_str());
            return nullptr;
        }
        //  LOG_TEST("EngineContextJSC try GetPropertyValueFromObject :%s", property_id.c_str());
        JSStringRef str = JSStringCreateWithUTF8CString(property_id.c_str());
        JSValueRef exc = nullptr;
        JSValueRef resultValue = JSObjectGetProperty(context_, object, str, &exc);
//        if (exc) {
//            Conversion::printJSValueRefException(context_, exc);
//        }
        JSStringRelease(str);
        if (resultValue && !JSValueIsUndefined(context_, resultValue)) {
            //     LOG_TEST("EngineContextJSC GetPropertyValueFromObject succeed :%s", property_id.c_str());
            return resultValue;
        }
        return nullptr;
    }

    bool EngineContextJSC::SetObjectPrototypeFromValue(JSObjectRef target, JSValueRef referenceValue) {
        LOG_JS_RUNTIME("EngineContextJSC SetObjectPrototypeFromValue ");
        if (nullptr == target && referenceValue == nullptr) {
            return false;
        }
        if (nullptr == referenceValue) {
            referenceValue = JSObjectGetPrototype(context_, JSContextGetGlobalObject(context_));
            //  referenceValue = JSContextr
        }
        JSObjectSetPrototype(context_, target, referenceValue);
        return true;
    }

    JSObjectRef EngineContextJSC::toObjectFromValue(JSValueRef js_value) {
        if (nullptr == js_value) {
            LOG_JS_RUNTIME("EngineContextJSC toObjectFromValue check failed !");
            return nullptr;
        }
        JSValueRef exc = nullptr;
        JSObjectRef convertObjectRef = JSValueToObject(context_, js_value, &exc);
        //Conversion::printJSValueRefException(context_, exc);
        if (convertObjectRef) {
            return convertObjectRef;
        }
        return nullptr;
    }

    JSValueRef EngineContextJSC::GetObjectPrototype(JSObjectRef object) {
        JSValueRef type = nullptr;
        if (nullptr == object) {
            // LOG_TEST("EngineContextJSC get global objectPrototype ");
            type = JSObjectGetPrototype(context_, JSContextGetGlobalObject(context_));
        } else {
            // LOG_TEST("EngineContextJSC get target objectPrototype ");
            type = JSObjectGetPrototype(context_, object);
        }
        return type;
    }

    bool EngineContextJSC::GetObjectPropertyNameArray(JSRunTimeObject object, std::vector<std::string> &nameArrays) {

        if (nullptr == object) {
            LOG_JS_RUNTIME("[EngineContextJSC] GetObjectPropertyNameArray use globalObject ");
            object = JSContextGetGlobalObject(context_);
        }

        auto pArray = JSObjectCopyPropertyNames(context_, object);
        size_t keyCount = JSPropertyNameArrayGetCount(pArray);
        for (size_t i = 0; i < keyCount; ++i) {
            auto propertyName_ = JSPropertyNameArrayGetNameAtIndex(pArray, i);
            if (propertyName_ == nullptr) {
                LOG_JS_RUNTIME("[EngineContextJSC]  create instance propertyName_ == null");
                return false;
            }
//            auto propertyValue_ = JSObjectGetProperty(context_, object, propertyName_, NULL);
//            if(propertyValue_ == nullptr) {
//                LOG_TEST("dyy create instance propertyValue_ == null");
//                continue;
//            }
            std::string result;
            size_t max_bytes = JSStringGetMaximumUTF8CStringSize(propertyName_);
            result.resize(max_bytes);
            size_t bytes_written = JSStringGetUTF8CString(propertyName_, &result[0], max_bytes);
            if (max_bytes == 0) {
                return false;
            }
            result.resize(bytes_written - 1);

            LOG_JS_RUNTIME("[EngineContextJSC] GetObjectPropertyName item :%s", result.c_str());

            //JSStringRelease(propertyName_);
            nameArrays.push_back(result);
        }
        JSPropertyNameArrayRelease(pArray);
        return true;
    }

    bool EngineContextJSC::setObjectValue(JSRunTimeObject targetObject, const std::string &key, JSRunTimeValue value) {
        LOG_JS_RUNTIME("EngineContextJSC enter func  setObjectValue onContext %p, [%s,%d],val:%p",
                       context_,
                       key.c_str(),
                       JSValueGetType(context_, value),
                       value
        );
        if (nullptr == targetObject) {
            //LOG_TEST("EngineContextJSC  setObjectValue to global ");
            targetObject = JSContextGetGlobalObject(context_);
        }

        auto str = JSStringCreateWithUTF8CString(key.c_str());
        JSValueRef exe = NULL;
        // LOG_TEST("try EngineContextJSC setObjectValue :%s", key.c_str());
        JSObjectSetProperty(context_, targetObject, str, value, 0, &exe);
        //LOG_TEST("EngineContextJSC setObjectValue succeed ");
        JSStringRelease(str);
        if (exe) {
            std::string jse;
            Conversion::JSValueToStdString(context_, exe, &jse);
            LOGE("[JSExcepion] ------> %s", jse.c_str());

            return false;
        } else {
            return true;
        }
    }

    JSObjectRef EngineContextJSC::GetGlobalObjectInContext() {
        return JSContextGetGlobalObject(context_);
    }

    void EngineContextJSC::BindDataToObject(JSRunTimeObject targetObj, void *data) {
        if (nullptr == targetObj) {
            return;
        }
        JSObjectSetPrivate(targetObj, data);
    }

    void EngineContextJSC::removeObjectBindData(JSRunTimeObject targetObj) {
        if (nullptr == targetObj) {
            return;
        }
        JSObjectSetPrivate(targetObj, nullptr);
    }

    void EngineContextJSC::SetName(const std::string &name) {
        this->context_name = name;
    }

    std::string EngineContextJSC::GetName() {
        return this->context_name;
    }

    ScopeValues EngineContextJSC::CallJavaScriptFuncWithRuntimeValue(RuntimeObject *target, const std::string &name,
                                                                     std::vector<ScopeValues> &args,
                                                                     std::string *exception) {
        JSValueRef  jsValue = this->callJavaScriptFunc(target,name,args,exception);
        return Conversion::JSValueToRuntimeValue(context_, nullptr, jsValue);
    }

}
// namespace unicorn
