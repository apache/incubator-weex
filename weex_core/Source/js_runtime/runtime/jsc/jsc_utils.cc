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

#include "js_runtime/runtime/jsc/jsc_utils.h"

#include <string>
#include <vector>
#include "JavaScriptCore/JSBase.h"
#include "JavaScriptCore/JSStringRef.h"
#include "JavaScriptCore/JSValueRef.h"

#include "js_runtime/runtime/jsc/runtime_values_jsc.h"
#include "js_runtime/runtime/runtime_object.h"
#include "js_runtime/runtime/runtime_values.h"
#include "js_runtime/utils/log_utils.h"
#include "js_runtime/runtime/js_runtime_conversion.h"

namespace unicorn {

    static JSClassRef kDefaultClass = nullptr;

    JSValueRef JSRuntimeConversion::RuntimeValueToJSRuntimeValue(EngineContext *ctx,
                                                                 JSClassRef class_ref,
                                                                 const RuntimeValues *value) {
        return Conversion::RuntimeValueToJSValue(static_cast<JSGlobalContextRef>(ctx->GetContext()), class_ref, value);
    }

    ScopeValues JSRuntimeConversion::JSRunTimeValueToRuntimeValue(EngineContext *ctx,
                                                                  JSObjectRef thiz,
                                                                  JSValueRef value) {
        return Conversion::JSValueToRuntimeValue(static_cast<JSGlobalContextRef>(ctx->GetContext()), thiz, value);
    }


    JSValueRef Conversion::RuntimeValueToJSValue(JSContextRef ctx,
                                                 JSClassRef class_ref,
                                                 const RuntimeValues *value) {
        if (value->IsUndefined()) {
            //      LOG_TEST("RuntimeValueToJSValue -> undefined");
            return JSValueMakeUndefined(ctx);
        } else if (value->IsNull()) {
            //    LOG_TEST("RuntimeValueToJSValue -> null");
            return JSValueMakeNull(ctx);
        } else if (value->IsBool()) {
            bool b = false;
            value->GetAsBoolean(&b);
            ///     LOG_TEST("[Context] RuntimeValueToJSValue -> value %p on ctx:%p, bool :%d", value, ctx, b);
            return JSValueMakeBoolean(ctx, b);
        } else if (value->IsInt()) {
            int num = 0;
            value->GetAsInteger(&num);
            //  LOG_TEST("RuntimeValueToJSValue -> int :%d", num);
            return JSValueMakeNumber(ctx, static_cast<double>(num));
        } else if (value->IsDouble()) {
            double num = 0.0;
            value->GetAsDouble(&num);
            //   LOG_TEST("RuntimeValueToJSValue -> double:%d", num);
            return JSValueMakeNumber(ctx, num);
        } else if (value->IsString()) {
            std::string tmp;
            value->GetAsString(&tmp);
            // LOG_TEST("RuntimeValueToJSValue -> string:%s", tmp.c_str());
            JSStringRef str = JSStringCreateWithUTF8CString(tmp.c_str());
            auto result = JSValueMakeString(ctx, str);
            JSStringRelease(str);
            return result;
        } else if (value->IsJsonObject()) {
            std::string json_str;
            value->GetAsUtf8JsonStr(json_str);
            return Conversion::ParserUtf8CharJsonToJValueJSContextRef(ctx, json_str.c_str());
        } else if (value->IsObject()) {
            // LOG_TEST("RuntimeValueToJSValue -> obj");
            BaseObject *native_ob = value->GetAsObject();
            if (class_ref == nullptr) {
                auto clz = native_ob->GetRuntimeClass();
                if (clz)
                    class_ref = clz->GetJSClass();
            }
            void *data = native_ob->GetDataPtr();
            if (data && !class_ref) {
                // if class_ref == null, so the js object can not store data
                // we use the default empty class instead
                if (!kDefaultClass) {
                    JSClassDefinition class_definition = kJSClassDefinitionEmpty;
                    kDefaultClass = JSClassCreate(&class_definition);
                }
                class_ref = kDefaultClass;
            }
            JSObjectRef js_obj = JSObjectMake(ctx, class_ref, nullptr);
            JSObjectSetPrivate(js_obj, data);
            return js_obj;
        } else if (value->IsMap()) {
            // LOG_TEST("RuntimeValueToJSValue [start]-> map");
            const JSCMap *local = static_cast<const JSCMap *>(value->GetAsMap());
            auto &map = local->GetMap();
            JSObjectRef thiz = local->GetThisObject();
            if (thiz == nullptr) {
                thiz = JSObjectMake(ctx, nullptr, nullptr);
            }
            for (auto &iter : map) {
                //    LOG_TEST("RuntimeValueToJSValue [map key]-> %s", iter.first.c_str());
                JSStringRef str = JSStringCreateWithUTF8CString(iter.first.c_str());
                JSValueRef value_ref = RuntimeValueToJSValue(ctx, nullptr, iter.second);
                JSObjectSetProperty(ctx, thiz, str, value_ref, 0, nullptr);
                JSStringRelease(str);
            }
            // LOG_TEST("RuntimeValueToJSValue [end]-> map");
            return thiz;
        } else if (value->IsFunction()) {
            // LOG_TEST("RuntimeValueToJSValue -> func");
            const JSCFunction *local =
                    static_cast<const JSCFunction *>(value->GetAsFunction());
            return local->GetFunction();
        }
// JSValueIsArray is only used in ios 9.0 or above, so we just enable
// array in android now
//#if defined(OS_ANDROID)
        else if (value->IsArray()) {
            //  LOG_TEST("RuntimeValueToJSValue[start] -> array");
            // const JSCArray *local = static_cast<const JSCArray *>(value->GetAsArray());
//            JSObjectRef thiz = local->GetThisObject();
//            if (thiz == nullptr) {
//                thiz = JSObjectMake(ctx, nullptr, nullptr);
//            }
            auto &array = value->GetAsArray()->GetArray();
            size_t length = array.size();
            //           JSStringRef len_str = JSStringCreateWithUTF8CString("length");
//            JSObjectSetProperty(ctx, thiz, len_str, JSValueMakeNumber(ctx, length),
//                                0, nullptr);
//            JSStringRelease(len_str);
            JSValueRef args[length];
            for (size_t i = 0; i < length; i++) {
                args[i] = RuntimeValueToJSValue(ctx, nullptr, array[i]);
                // JSObjectSetPropertyAtIndex(ctx, thiz, i, value_ref, nullptr);
            }
            JSValueRef js_exception = nullptr;

//            for (int i = 0; i < length; i++) {
//                LOG_TEST("RuntimeValueToJSValue[item type: %d] ,onContext :%p", JSValueGetType(ctx, args[i]), ctx);
//            }

            JSObjectRef js_array = JSObjectMakeArray(ctx, length, args, &js_exception);
            printJSValueRefException(ctx, js_exception);
            //   LOG_TEST("RuntimeValueToJSValue[end] -> array");
            return js_array;
        }
//#endif

        return JSValueMakeUndefined(ctx);
    }


    ScopeValues Conversion::JSValueToRuntimeValue(JSContextRef ctx,
                                                  JSObjectRef thiz,
                                                  JSValueRef value) {
        std::vector<JSValueRef> stack_record;
        std::string exception_str;
        auto res = Conversion::JSValueToRuntimeValueWithCircleCheck(ctx,thiz,value,stack_record,exception_str);
        if(!exception_str.empty()){
            LOGE("[JSValueToRuntimeValue] error :%s",exception_str.c_str());
            return unicorn::RuntimeValues::MakeNull();
        } else {
            return res;
        }
    }

    ScopeValues Conversion::JSValueToRuntimeValueWithCircleCheck(JSContextRef ctx, JSObjectRef thiz, JSValueRef value,
                                                                 std::vector<JSValueRef> &stack_record,
                                                                 std::string &exception) {
        if (!exception.empty()){
            return unicorn::RuntimeValues::MakeNull();
        }
        if (JSValueIsNumber(ctx, value)) {
            //LOGW("[JSValueToRuntimeValue] JSValueToRuntimeValue JSValueIsNumber ");
            double origin = JSValueToNumber(ctx, value, nullptr);
            if (origin == static_cast<int>(origin)) {
                return ScopeValues(new RuntimeValues(static_cast<int>(origin)));
            }
            return ScopeValues(new RuntimeValues(origin));
        } else if (JSValueIsBoolean(ctx, value)) {
            //LOGW("[JSValueToRuntimeValue] bool ");
            return ScopeValues(new RuntimeValues(JSValueToBoolean(ctx, value)));
        } else if (JSValueIsNull(ctx, value)) {
            //LOGW("[JSValueToRuntimeValue]  JSValueIsNull ");
            return ScopeValues(new RuntimeValues(nullptr));
        } else if (JSValueIsUndefined(ctx, value)) {
            //LOGW("[JSValueToRuntimeValue]JSValueIsUndefined ");
            return ScopeValues(new RuntimeValues());
        } else if (JSValueIsString(ctx, value)) {
            //LOGW("[JSValueToRuntimeValue]  JSValueIsString enter");
            std::string result;
            if (Conversion::JSValueToStdString(ctx, value, &result)) {
              //  LOGW("[JSValueToRuntimeValue]  JSValueIsString result:%s", result.c_str());
                return RuntimeValues::MakeString(result);
            } else {
               // LOGW("[JSValueToRuntimeValue]  JSValueIsString result: %s", "empty");
                return RuntimeValues::MakeString(std::string(""));
            }
        }// J
        else if(JSValueIsObject(ctx,value)){
            for(int i=0;i<stack_record.size();i++){
                JSValueRef item_value = stack_record[i];
                if (item_value == value){
                    exception.append(std::string("circular reference"));
                    return unicorn::RuntimeValues::MakeNull();
                }
            }
            stack_record.push_back(value);
            JSObjectRef ob = JSValueToObject(ctx, value, nullptr);
            auto native_ob = JSObjectGetPrivate(ob);
            if (native_ob) {
               // LOGW("[JSValueToRuntimeValue]  native_ob :%p", ob);
                stack_record.pop_back();
                return RuntimeValues::MakeCommonObject(native_ob, nullptr);
            }else if (JSObjectIsFunction(ctx, ob)) {
                //LOGW("[JSValueToRuntimeValue]  func :ob %p", ob);
                //LOG_JS_RUNTIME("[Context][Conversion] JSValueToRuntimeValue JSObjectIsFunction context:%p ,func:%p", ctx, ob);
                auto func = JSCFunction::Create(ctx, "", thiz, ob);
                auto func_holder = RuntimeValues::MakeFunction(std::move(func));
                stack_record.pop_back();
                return func_holder;
            } else if (JSValueIsArray(ctx, value)) {
                //SValueIsArray is only used in ios 9.0 or above, so we just enable
               // LOGW("[JSValueToRuntimeValue]  array ");
                JSObjectRef ob = JSValueToObject(ctx, value, nullptr);
                JSStringRef len_str = JSStringCreateWithUTF8CString("length");
                int length = static_cast<int>(JSValueToNumber(ctx,
                                                              JSObjectGetProperty(ctx, ob, len_str, nullptr), nullptr));
                JSStringRelease(len_str);
                auto holder = JSCArray::Create(ctx, ob);
                for (int i = 0; i < length; i++) {
                    JSValueRef temp = JSObjectGetPropertyAtIndex(ctx, ob, i, nullptr);
                    //LOGW("[JSValueToRuntimeValue]  before array index:%d ,valueType:%d,val_addr:%p", i, JSValueGetType(ctx, temp), temp);
                    auto native = JSValueToRuntimeValueWithCircleCheck(ctx, nullptr, temp,stack_record,exception);
                    if (!exception.empty()){
                        exception.append(std::string(" on array index:"));
                        exception.append(std::to_string(i));
                        stack_record.pop_back();
                        return unicorn::RuntimeValues::MakeNull();
                    }
                   // LOGW("[JSValueToRuntimeValue]  after array index:%d ,valueType:%d", i, JSValueGetType(ctx, temp));
                    holder->PushBack(native.release());
                }
                stack_record.pop_back();
                auto array_holder = RuntimeValues::MakeArray(std::move(holder));
                return array_holder;
            } else{
                std::vector<std::string> properties;
                JSUtils::GetPropertyNameArray(ctx, ob, properties);
                //LOG_TEST("[Conversion]  JSCMap::Create");
                auto jsc_map = JSCMap::Create(ctx, ob);
                // LOG_TEST("[Conversion] JSValueToRuntimeValue else properties.size():%lu", properties.size());
                for (size_t i = 0; i < properties.size(); i++) {
                    //  LOG_TEST("[Conversion] JSValueToRuntimeValue JSStringCreateWithUTF8CString name :%s",properties[i].c_str());
                    JSStringRef str_ref = JSStringCreateWithUTF8CString(
                            properties[i].c_str());
                    JSValueRef val = JSObjectGetProperty(ctx, ob, str_ref, nullptr);
                    JSStringRelease(str_ref);
                   // LOGW("[JSValueToRuntimeValue]  map :key:%s,valueType:%d ,value_addr:%p,index:%d",properties[i].c_str(), JSValueGetType(ctx, val), val, i);
                    //  LOG_TEST("[Conversion]  Conversion::JSValueToRuntimeValue(ctx, ob, val)");
                    auto native = Conversion::JSValueToRuntimeValueWithCircleCheck(ctx, ob, val,stack_record,exception);
                    if(!exception.empty()){
                        exception.append(" on property:");
                        exception.append(properties[i]);
                        stack_record.pop_back();
                        return unicorn::RuntimeValues::MakeNull();
                    }
                    jsc_map->Insert(properties[i], native.release());
                }
                // LOG_TEST("[Conversion] map_holder = RuntimeValues::MakeMap(std::move(jsc_map)");
                stack_record.pop_back();
                auto map_holder = RuntimeValues::MakeMap(std::move(jsc_map));
                //  LOG_TEST("[Conversion] return map_holder");
                return map_holder;
            }
        }
        return ScopeValues(new RuntimeValues());
    }


    void Conversion::JSValuesArrayToRuntimeValues(JSContextRef ctx,
                                                  JSObjectRef thiz,
                                                  size_t argc, const JSValueRef *argv,
                                                  std::vector<ScopeValues> &output) {
        if (argc == 0 || argv == nullptr)
            return;
        for (size_t i = 0; i < argc; i++) {
            output.push_back(JSValueToRuntimeValue(ctx, thiz, argv[i]));
        }
    }

    bool Conversion::JSValueToStdString(JSContextRef ctx, JSValueRef value, std::string *result) {
        if (nullptr == value || JSValueIsNull(ctx, value)) {
            return false;
        }
        JSStringRef str = JSValueToStringCopy(ctx, value, nullptr);
        size_t max_bytes = JSStringGetMaximumUTF8CStringSize(str);
        result->resize(max_bytes);
        size_t bytes_written = JSStringGetUTF8CString(str, &(*result)[0], max_bytes);
        if (max_bytes == 0) {
            return false;
        }
        result->resize(bytes_written - 1);
        // LOGE("Conversion::JSValueToStdString result: %s", result->c_str());

        JSStringRelease(str);
        return true;
    }

    JSValueRef Conversion::ParserUtf8CharJsonToJValueJSContextRef(JSContextRef ctx, const char *utf_8_str) {


        //LOG_TEST("Conversion::ParserUtf8CharJson : length:%d, str: %s", std::strlen(utf_8_str), utf_8_str);
        JSStringRef jsonStrRef = JSStringCreateWithUTF8CString(utf_8_str);
        JSValueRef jValue = JSValueMakeFromJSONString(ctx, jsonStrRef);
        if (jsonStrRef) {
            //   LOG_TEST("Conversion::ParserUtf8CharJson : release str");
            JSStringRelease(jsonStrRef);
        }
        return jValue;
    }

    void Conversion::printJSValueRefException(JSContextRef context, JSValueRef exc) {
        if (nullptr == exc || JSValueIsNull(context, exc)) {
            return;
        }
        std::string result;
        JSStringRef str = JSValueToStringCopy(context, exc, nullptr);
        size_t max_bytes = JSStringGetMaximumUTF8CStringSize(str);
        result.resize(max_bytes);
        size_t bytes_written = JSStringGetUTF8CString(str, &result[0], max_bytes);
        if (max_bytes == 0) {
            return;
        }
        result.resize(bytes_written - 1);
        if (!result.empty()) {
            LOG_JS_ERROR("[JS_ERROR] : %s", result.c_str());
        }
        JSStringRelease(str);
    }


/////////////////////////////////////////////////////////////////////////////
/*JSUtils implemention*/
    bool JSUtils::HasProperty(JSContextRef ctx, JSObjectRef ob,
                              const std::string &name) {
        if (!ctx || !ob)
            return false;

        JSContextRef context = static_cast<JSContextRef>(ctx);
        JSObjectRef object = static_cast<JSObjectRef>(ob);
        JSStringRef str = JSStringCreateWithUTF8CString(name.c_str());
        bool ret = JSObjectHasProperty(context, object, str);
        JSStringRelease(str);
        return ret;
    }

    void JSUtils::SetProperty(JSContextRef ctx, JSObjectRef ob,
                              JSClassRef clz, const std::string &name,
                              RuntimeValues *value) {
        if (!ctx || !ob)
            return;

        JSContextRef context = static_cast<JSContextRef>(ctx);
        JSObjectRef object = static_cast<JSObjectRef>(ob);
        JSClassRef clasz = static_cast<JSClassRef>(clz);
        JSStringRef str = JSStringCreateWithUTF8CString(name.c_str());
        JSValueRef ret = Conversion::RuntimeValueToJSValue(context, clasz, value);
        JSObjectSetProperty(context, object, str, ret, 0, nullptr);
        JSStringRelease(str);
    }

    bool JSUtils::DeleteProperty(JSContextRef ctx, JSObjectRef ob,
                                 const std::string &name) {
        if (!ctx || !ob)
            return false;

        JSContextRef context = static_cast<JSContextRef>(ctx);
        JSObjectRef object = static_cast<JSObjectRef>(ob);
        JSStringRef str = JSStringCreateWithUTF8CString(name.c_str());
        bool ret = JSObjectDeleteProperty(context, object, str, nullptr);
        JSStringRelease(str);
        return ret;
    }

    ScopeValues JSUtils::GetProperty(JSContextRef ctx, JSObjectRef ob,
                                     const std::string &name) {
        if (!ctx || !ob)
            return ScopeValues(new RuntimeValues());

        JSContextRef context = static_cast<JSContextRef>(ctx);
        JSObjectRef object = static_cast<JSObjectRef>(ob);
        JSStringRef str = JSStringCreateWithUTF8CString(name.c_str());
        JSValueRef ret = JSObjectGetProperty(context, object, str, nullptr);
        JSStringRelease(str);
        return Conversion::JSValueToRuntimeValue(context, object, ret);
    }

    void JSUtils::GetPropertyNameArray(JSContextRef ctx, JSObjectRef ob,
                                       std::vector<std::string> &native_array) {
        if (!ctx || !ob)
            return;
        //  LOG_TEST("[jsc_utils] GetPropertyNameArray ");

        JSContextRef context = static_cast<JSContextRef>(ctx);
        JSObjectRef object = static_cast<JSObjectRef>(ob);
        JSPropertyNameArrayRef array_ref = JSObjectCopyPropertyNames(context,
                                                                     object);
        size_t count = JSPropertyNameArrayGetCount(array_ref);

        //  LOG_TEST("[jsc_utils] JSPropertyNameArrayGetCount :%lu", count);
        for (size_t i = 0; i < count; i++) {
            JSStringRef str_ref = JSPropertyNameArrayGetNameAtIndex(array_ref, i);
            size_t max_len = JSStringGetMaximumUTF8CStringSize(str_ref);
            std::string holder;
            holder.resize(max_len);
            char *ptr = &holder[0];
            size_t bytes_written = JSStringGetUTF8CString(str_ref, ptr, max_len);
            holder.resize(bytes_written - 1);
            native_array.push_back(holder);
        }
        JSPropertyNameArrayRelease(array_ref);
    }

    ScopeValues JSUtils::CallAsFunction(JSContextRef ctx, JSObjectRef thiz,
                                        JSObjectRef function,
                                        size_t argc, const JSValueRef argv[]) {
        if (!ctx || !function)
            return ScopeValues(new RuntimeValues());

        JSContextRef context = static_cast<JSContextRef>(ctx);
        JSObjectRef thiz_ob = static_cast<JSObjectRef>(thiz);
        JSObjectRef function_ob = static_cast<JSObjectRef>(function);
        JSValueRef ret = JSObjectCallAsFunction(context, function_ob, thiz_ob,
                                                argc, argv, nullptr);
        return Conversion::JSValueToRuntimeValue(context, thiz_ob, ret);
    }
}  // namespace unicorn
