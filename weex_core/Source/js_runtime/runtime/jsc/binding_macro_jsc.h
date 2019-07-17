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

#ifndef FLUTTER_UNICORN_RUNTIME_JSC_BINDING_MACRO_JSC_H_
#define FLUTTER_UNICORN_RUNTIME_JSC_BINDING_MACRO_JSC_H_

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "js_runtime/runtime/base.h"
#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/jsc/jsc_utils.h"
#include "js_runtime/runtime/runtime_object.h"
#include "js_runtime/runtime/runtime_vm.h"
#include "js_runtime/utils/log_utils.h"

#include "JavaScriptCore/JSObjectRef.h"

typedef JSObjectGetPropertyCallback GetterCallback;
typedef JSObjectSetPropertyCallback SetterCallback;
typedef JSObjectCallAsFunctionCallback FunctionCallback;

__attribute__((unused))
static JSValueRef MakeClassToFunctionType(
        JSContextRef ctx,
        JSObjectRef function,
        JSObjectRef thiz, size_t argc,
        const JSValueRef *argv,
        JSValueRef *exception
) {
    auto ret = unicorn::RuntimeValues::MakeUndefined();
    JSValueRef prop =
            unicorn::Conversion::RuntimeValueToJSValue(ctx, nullptr, ret.get());
    return prop;
}

class SetterAndGetterCallback {
public:
    SetterAndGetterCallback() : getter_(nullptr), setter_(nullptr) {}

    ~SetterAndGetterCallback() = default;

    GetterCallback getter_;
    SetterCallback setter_;
};

// define class member get and set callback
#define GET_CALLBACK_FUNCTION(member_name_) Get ## member_name_ ## Callback
#define SET_CALLBACK_FUNCTION(member_name_) Set ## member_name_ ## Callback

// define class member function callback
#define METHOD_CALLBACK_FUNCTION(method_name_) method_name_##Callback

// define class static member function callback
#define STATIC_METHOD_CALLBACK_FUNCTION(method_name_) method_name_##Callback

// define class constructor callback
#define CONSTRUCTOR_CALLBACK_FUNCTION(class_) class_##ConstructorCallback

// define class object finalize callback
#define FINALIZE_CALLBACK_FUNCTION(class_) class_##FinalizeCallback

// define global class JSClassRef, we need to create class object
// define js class for class
#define DECLARE_CLASS_REGISTER_OP(class_)                                        \
static JSClassRef s_jsclass_##class_;                                              \
static JSClassRef CreateClassRef(unicorn::EngineContext* context);
static bool s_is_global_binding = false;

#define SET_IS_GLOBAL                                                       \
s_is_global_binding = true;

#define CHECK_IS_BASIC_TYPE(type_name_)                                    \
  bool is_basic_type = false;                                              \
  if (std::is_integral<std::remove_reference<type_name_>::type>::value)    \
    is_basic_type = true;                                                  \
  if (std::is_floating_point<                                              \
           std::remove_reference<type_name_>::type>::value)                \
    is_basic_type = true;                                                  \
  if (std::is_same<std::string,                                            \
           std::remove_reference<type_name_>::type>::value)                \
    is_basic_type = true;                                                  \
  if (std::is_same<char[], std::remove_reference<type_name_>::type>::value)\
    is_basic_type = true;                                                  \
  if (std::is_same<char*, std::remove_reference<type_name_>::type>::value) \
    is_basic_type = true;

#define MEMBER_GET_CALLBACK_FUNCTION(class_, member_name_)                 \
static JSValueRef GET_CALLBACK_FUNCTION(member_name_)(                     \
                                        JSContextRef ctx,                  \
                                        JSObjectRef thiz,                  \
                                        JSStringRef propertyName,          \
                                        JSValueRef* exception              \
                                        ) {                                \
  class_* obj = static_cast<class_*>(JSObjectGetPrivate(thiz));            \
  const auto ret = obj->member_name_();                                    \
  std::string name = #member_name_;                                        \
  JSValueRef prop =                                                        \
    unicorn::Conversion::RuntimeValueToJSValue(ctx, nullptr, ret.get());   \
  return prop;                                                             \
}

#define MEMBER_SET_CALLBACK_FUNCTION(class_, member_name_)                 \
static bool SET_CALLBACK_FUNCTION(member_name_)(                           \
                                  JSContextRef ctx,                        \
                                  JSObjectRef thiz,                        \
                                  JSStringRef propertyName,                \
                                  JSValueRef value,                        \
                                  JSValueRef* exception                    \
                                  ) {                                      \
  class_* obj = static_cast<class_*>(JSObjectGetPrivate(thiz));            \
  auto val = unicorn::Conversion::JSValueToRuntimeValue(ctx, thiz, value); \
  obj->member_name_(std::move(val));                                       \
  return true;                                                             \
}

#define CLASS_MEMBER_SET_CALLBACK(class_, member_name_)                    \
MEMBER_SET_CALLBACK_FUNCTION(class_, member_name_)                         \

#define CLASS_MEMBER_GET_CALLBACK(class_, member_name_)                    \
MEMBER_GET_CALLBACK_FUNCTION(class_, member_name_)


/* This macro for the binding function without return type */
#define CLASS_VOID_METHOD_CALLBACK(class_, method_name_)                   \
static JSValueRef METHOD_CALLBACK_FUNCTION(method_name_)(                  \
                                        JSContextRef ctx,                  \
                                        JSObjectRef function,              \
                                        JSObjectRef thiz, size_t argc,     \
                                        const JSValueRef* argv,            \
                                        JSValueRef *exception              \
                                        ) {                                \
  class_* obj = static_cast<class_*>(JSObjectGetPrivate(thiz));            \
  std::vector<unicorn::ScopeValues> arguments;                             \
  unicorn::Conversion::JSValuesArrayToRuntimeValues(ctx, thiz, argc,       \
                               argv,                                       \
                               arguments);                                 \
  auto ret = obj->method_name_(arguments);                                 \
  return unicorn::Conversion::RuntimeValueToJSValue(ctx,                   \
                                                    nullptr, ret.get());   \
}

/* This macro for the binding function with return type */
#define CLASS_METHOD_CALLBACK(class_, method_name_)                        \
static JSValueRef METHOD_CALLBACK_FUNCTION(method_name_)(                  \
                                        JSContextRef ctx,                  \
                                        JSObjectRef function,              \
                                        JSObjectRef thiz, size_t argc,     \
                                        const JSValueRef* argv,            \
                                        JSValueRef *exception              \
                                        ) {                                \
                                                                                                            \
  auto addr = JSObjectGetPrivate(thiz);                                                                     \
  if(nullptr == addr && s_is_global_binding){                                                               \
      JSObjectRef globalObject = JSContextGetGlobalObject(ctx);                                             \
      addr = JSObjectGetPrivate(globalObject);                                                              \
     LOG_JS_RUNTIME("[Context]try get object %p at globalObject :%p, ",addr,thiz);                        \
  }                                                                                                         \
  if( nullptr == addr){                                                                                     \
      LOGE("[Context]return undefined!! can't get object %p at thiz:%p, method:%s",addr,thiz,#method_name_);                       \
      return JSValueMakeUndefined(ctx);                                                                     \
  }                                                                                                         \
  class_* obj = static_cast<class_*>(addr);                                \
  std::vector<unicorn::ScopeValues> arguments;                             \
  unicorn::Conversion::JSValuesArrayToRuntimeValues(ctx, thiz, argc,       \
                               argv,                                       \
                               arguments);                                 \
  auto ret = obj->method_name_(arguments);                                 \
  JSValueRef prop =                                                        \
    unicorn::Conversion::RuntimeValueToJSValue(ctx, nullptr, ret.get());   \
  return prop;                                                             \
}

#define CLASS_STATIC_METHOD_CALLBACK(class_name_, member_name_)            \
static JSValueRef STATIC_METHOD_CALLBACK_FUNCTION(member_name_)(           \
                                JSContextRef ctx, JSObjectRef function,    \
                                JSObjectRef thiz,                          \
                                size_t argc, const JSValueRef argv[],      \
                                JSValueRef* exception) {                   \
  std::vector<unicorn::ScopeValues> arguments;                             \
  unicorn::Conversion::JSValuesArrayToRuntimeValues(ctx, thiz, argc,       \
                                                    argv, arguments);      \
  JSGlobalContextRef global = JSContextGetGlobalContext(ctx);              \
  EngineContext* context = RuntimeVM::GetEngineContext(                    \
                                         static_cast<const void*>(global));\
  auto thiz_object = new RuntimeObject(context);                           \
  thiz_object->SetJSObject(thiz);                                          \
  auto thiz_value = RuntimeValues::MakeCommonObject(                       \
                                        static_cast<void*>(thiz_object),   \
                                        nullptr);                          \
  auto ret = class_name_::member_name_(std::move(thiz_value), arguments);  \
  return unicorn::Conversion::RuntimeValueToJSValue(ctx,                   \
                                                    nullptr, ret.get());   \
}

#define CLASS_CONSTRUCTOR_CALLBACK(class_)                                 \
static JSObjectRef CONSTRUCTOR_CALLBACK_FUNCTION(class_)(                  \
                               JSContextRef ctx,                           \
                               JSObjectRef constructor,                    \
                               size_t argc,                                \
                               const JSValueRef argv[],                    \
                               JSValueRef* exception                       \
                               ) {                                         \
  std::vector<unicorn::ScopeValues> arguments;                             \
  unicorn::Conversion::JSValuesArrayToRuntimeValues(ctx, nullptr, argc,    \
                                        argv,                              \
                                        arguments);                        \
  JSGlobalContextRef global = JSContextGetGlobalContext(ctx);              \
  unicorn::EngineContext* context = unicorn::RuntimeVM::GetEngineContext(                    \
                                         static_cast<const void*>(global));\
  auto thiz_object = new unicorn::RuntimeObject(context);                           \
  thiz_object->SetJSObject(constructor);                                   \
  auto thiz_value = unicorn::RuntimeValues::MakeCommonObject(                       \
                                        static_cast<void*>(thiz_object),   \
                                        nullptr);                          \
  class_* t =  class_::JSCreate(context,std::move(thiz_value), arguments);         \
  JSObjectRef ob = JSObjectMake(ctx, class_::s_jsclass_##class_,                   \
                                static_cast<void*>(t));                    \
  return ob;                                                               \
}

#define CLASS_FINALIZE_CALLBACK(class_)                                    \
static void FINALIZE_CALLBACK_FUNCTION(class_)(JSObjectRef object) {       \
  class_* t = static_cast<class_*>(JSObjectGetPrivate(object));            \
  class_::JSFinalize(t);                                                   \
}

#define CLASS_REGISTER_START(class_name_, js_class_name_)                  \
JSClassRef class_name_::s_jsclass_##class_name_ = nullptr;                              \
JSClassRef class_name_::CreateClassRef(unicorn::EngineContext* context) {  \
  JSContextRef ctx = nullptr != context                                    \
          ?static_cast<JSContextRef>(context->GetContext())                  \
          :nullptr;                                                             \
  std::vector<JSStaticValue> class_values;                                 \
  std::vector<JSStaticFunction> class_functions;                           \
  JSClassDefinition class_definition = kJSClassDefinitionEmpty;            \
  std::string js_name = #js_class_name_;                                   \
  std::vector<std::string> tmp;                                            \
  std::unordered_map<JSStringRef, JSObjectRef> static_functions;           \
  std::unordered_map<JSStringRef, JSValueRef> static_members;              \
  std::map<std::string, SetterAndGetterCallback> members_map;              \
  bool enable_constructor = false;                                         \
  JSClassRef parent_ref =  nullptr;

#define REGISTER_PARENT_CLASS(parent_name_)                                \
  parent_ref = parent_name_::s_jsclass_##parent_name_;

#define CLASS_CONSTRUCTOR_ENABLE(class_name_)                              \
  enable_constructor = true;                                               \
  if (enable_constructor) {                                                \
    class_definition.finalize = FINALIZE_CALLBACK_FUNCTION(class_name_);   \
    class_definition.callAsConstructor =                                   \
                     CONSTRUCTOR_CALLBACK_FUNCTION(class_name_);           \
    class_definition.callAsFunction = MakeClassToFunctionType;             \
  }

#define REGISTER_GET_CALLBACK(class_name_, member_name_)                   \
  {                                                                        \
    std::string name = #member_name_;                                      \
    auto& temp = members_map[name];                                        \
    temp.getter_ = GET_CALLBACK_FUNCTION(member_name_);                    \
  }

#define REGISTER_SET_CALLBACK(class_name_, member_name_)                   \
  {                                                                        \
    std::string name = #member_name_;                                      \
    auto& temp = members_map[name];                                        \
    temp.setter_ = SET_CALLBACK_FUNCTION(member_name_);                    \
  }

/**
* magic bug :
 * class_functions.push_back(...) with first params
 * use name.c_str() ,the vector result order is confusion
 *      example register method named :log、addEventListener、addEventListenerTest、test
 *      for-each vector,the result item named (log,addEventListenerTest,addEventListenerTest,test)
 * but use #method_name_ ,it works fine
 * so why ??????????????????????????????
*/

#define REGISTER_METHOD_CALLBACK(class_name_, method_name_)                \
  {                                                                        \
    std::string name = #method_name_;                                      \
    tmp.push_back(name);                                                   \
    class_functions.push_back({                                            \
                                #method_name_,                              \
                                METHOD_CALLBACK_FUNCTION(method_name_),   \
                                kJSPropertyAttributeNone                   \
                              });                                          \
  }

#define REGISTER_STATIC_METHOD_CALLBACK(class_name_, method_name_)         \
  {                                                                        \
    std::string name = #method_name_;                                      \
    JSStringRef f_name = JSStringCreateWithUTF8CString(name.c_str());      \
    JSObjectRef object = JSObjectMakeFunctionWithCallback(ctx, f_name,     \
                            STATIC_METHOD_CALLBACK_FUNCTION(method_name_));\
    static_functions[f_name] = object;                                     \
  }

#define REGISTER_STATIC_MEMBER(class_name_, member_name_, type_name_)      \
  {                                                                        \
    std::string name = #member_name_;                                      \
    JSStringRef m_name = JSStringCreateWithUTF8CString(name.c_str());      \
    type_name_* ptr = &class_name_::member_name_;                          \
    auto scope = ToRuntimeValues<type_name_>(                              \
                                 static_cast<void*>(ptr), name);           \
    JSValueRef ref =                                                       \
          unicorn::Conversion::RuntimeValueToJSValue(ctx,                  \
                                                     nullptr, scope.get());\
    static_members[m_name] = ref;                                          \
  }

#define CLASS_REGISTER_END(class_name_)                                    \
  for (auto& h : members_map) {                                            \
    class_values.push_back({                                               \
                               h.first.c_str(),                            \
                               h.second.getter_,                           \
                               h.second.setter_,                           \
                               kJSPropertyAttributeNone                    \
                             });                                           \
  }                                                                        \
  class_values.push_back({0, 0, 0, 0});                                    \
  class_functions.push_back({0, 0, 0});                                    \
  class_definition.version = 0;                                            \
  class_definition.attributes = 0;                                         \
  class_definition.className = js_name.c_str();                            \
  class_definition.parentClass = parent_ref;                               \
  class_definition.staticValues = &class_values.front();                   \
  class_definition.staticFunctions = &class_functions.front();             \
  class_definition.initialize = 0;                                         \
  JSClassRef clazz = JSClassCreate(&class_definition);                     \
  class_name_::s_jsclass_##class_name_ = clazz;                                         \
  if (enable_constructor) {                                                \
    JSStringRef str = JSStringCreateWithUTF8CString(                       \
                             class_definition.className);                  \
    JSObjectRef constructor = JSObjectMake(ctx, clazz, nullptr);           \
    JSObjectRef proto_type = JSObjectMakeConstructor(ctx, clazz,           \
                              class_definition.callAsConstructor);         \
    for (auto& m : static_members) {                                       \
      JSObjectSetProperty(ctx, proto_type, m.first,                        \
                          m.second, 0, nullptr);                           \
      JSStringRelease(m.first);                                            \
    }                                                                      \
    for (auto& f : static_functions) {                                     \
      JSObjectSetProperty(ctx, proto_type, f.first,                        \
                          f.second, 0, nullptr);                           \
      JSStringRelease(f.first);                                            \
    }                                                                      \
    /* this way to make binding class to be function type right or not */  \
    JSObjectSetPrototype(ctx, constructor, proto_type);                    \
    JSObjectRef global = JSContextGetGlobalObject(ctx);                    \
    JSObjectSetProperty(ctx, global, str, constructor,                     \
                        kJSPropertyAttributeNone, NULL);                   \
    JSStringRelease(str);                                                  \
  }                                                                        \
  return clazz;                                                            \
}

#define CLASS_OBJECT_REGISTER(class_name_)                                 \
static void* Register##class_name_##ToJSObject(EngineContext* context,     \
                                           const char* object_name,        \
                                           void* data) {                   \
  if (context == nullptr || data == nullptr ||                             \
      g_##class_name_##Ref == nullptr)                                     \
    return nullptr;                                                        \
  JSContextRef ctx = static_cast<JSContextRef>(context->GetContext());     \
  JSObjectRef global_object = JSContextGetGlobalObject(ctx);               \
  JSObjectRef js_object = JSObjectMake(ctx,                                \
                                       class_name_::s_jsclass_##class_name_,            \
                                       data);                              \
  JSStringRef js_name = JSStringCreateWithUTF8CString(object_name);        \
  JSObjectSetProperty(ctx, global_object, js_name, js_object,              \
                      kJSPropertyAttributeNone, NULL);                     \
  JSStringRelease(js_name);                                                \
  return static_cast<void*>(js_object);                                    \
}

#define GLOBAL_METHOD_REGISTER(method_name_)                               \
{                                                                          \
  std::string name = #method_name_;                                        \
  JSObjectRef global = JSContextGetGlobalObject(ctx);                      \
  JSStringRef function_name =                                              \
              JSStringCreateWithUTF8CString(name.c_str());                 \
  JSObjectRef function_object = JSObjectMakeFunctionWithCallback(ctx,      \
                                   function_name,                          \
                                   &method_name_);                         \
  JSObjectSetProperty(ctx,                                                 \
                      global,                                              \
                      function_name,                                       \
                      function_object,                                     \
                      kJSPropertyAttributeNone,                            \
                      nullptr);                                            \
  JSStringRelease(function_name);                                          \
}

#endif  //  FLUTTER_UNICORN_RUNTIME_JSC_BINDING_MACRO_JSC_H_
