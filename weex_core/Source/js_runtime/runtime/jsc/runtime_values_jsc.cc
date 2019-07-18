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

#include "js_runtime/runtime/jsc/runtime_values_jsc.h"

#include "js_runtime/runtime/engine_context.h"
#include "js_runtime/runtime/jsc/jsc_utils.h"
#include "js_runtime/runtime/runtime_object.h"
#include "js_runtime/runtime/runtime_values.h"
#include "js_runtime/runtime/runtime_vm.h"
#include "js_runtime/utils/log_utils.h"

namespace unicorn {

/////////////////////////////////////////////////////////////////////////////
/* class JSCFunction implemention */
std::unique_ptr<Function>
JSCFunction::CreateWithCallback(JSContextRef ctx, std::string name,
                                JSObjectRef thiz,
                                JSObjectCallAsFunctionCallback callback) {
  Function* ptr = static_cast<Function*>(new JSCFunction(ctx, name,
                                             thiz, callback));
  return std::unique_ptr<Function>(ptr);
}

std::unique_ptr<Function>
JSCFunction::Create(JSContextRef ctx, std::string name,
                    JSObjectRef thiz, JSObjectRef function) {
  Function* ptr = static_cast<Function*>(new JSCFunction(ctx, name,
                                             thiz, function));
  return std::unique_ptr<Function>(ptr);
}

JSCFunction::JSCFunction(JSContextRef ctx, std::string name, JSObjectRef thiz,
                        JSObjectCallAsFunctionCallback callback)
                        : Function(),
                          context_(ctx),
                          function_name_(name),
                          thiz_(thiz),
                          is_from_native_(true) {
  if (thiz_ == nullptr) {
    thiz_ = JSContextGetGlobalObject(ctx);
  }

  JSStringRef str = JSStringCreateWithUTF8CString(function_name_.c_str());
  function_ = JSObjectMakeFunctionWithCallback(context_, str, callback);
  JSObjectSetProperty(context_, thiz_, str, function_, 0, nullptr);
  JSStringRelease(str);

  JSGlobalContextRef global_ctx = JSContextGetGlobalContext(ctx);
  EngineContext* context =
        RuntimeVM::GetEngineContext(static_cast<const void*>(global_ctx));
  RuntimeObject* native_thiz = new RuntimeObject(context);
  native_thiz->SetJSObject(function_);
  Function::SetObject(native_thiz);

  MemberProtect();
}

JSCFunction::JSCFunction(JSContextRef ctx, std::string name, JSObjectRef thiz,
                        JSObjectRef function)
                        : context_(ctx),
                          function_name_(name),
                          thiz_(thiz),
                          function_(function),
                          is_from_native_(false) {
  context_ = JSContextGetGlobalContext(ctx);
 // LOG_JS_RUNTIME("JSCFunction constructor,ctx:%p, thiz:%p,func:%p",context_,thiz_,function_);
  JSGlobalContextRef global_ctx = JSContextGetGlobalContext(ctx);
  EngineContext* context =
        RuntimeVM::GetEngineContext(static_cast<const void*>(global_ctx));
  RuntimeObject* native_thiz = new RuntimeObject(context);
  native_thiz->SetJSObject(function_);
  Function::SetObject(native_thiz);

  MemberProtect();
}

JSCFunction::~JSCFunction() {
  MemberUnprotect();
}

void JSCFunction::MemberProtect() {
  JSValueProtect(context_, thiz_);
  JSValueProtect(context_, function_);
}

void JSCFunction::MemberUnprotect() {
  JSValueUnprotect(context_, thiz_);
  JSValueUnprotect(context_, function_);
}

std::unique_ptr<RuntimeValues> JSCFunction::Call(JSContextRef cur_context,size_t argc,
                                    std::vector<ScopeValues>& argv) const {
  LOG_RUNTIME("call js on context:%p",cur_context);
  JSValueRef argv_js[argc];
  for (size_t i = 0; i < argc; i++) {
//    LOG_TEST("[Context] JSCFunction Call func originContext :%p",context_);
//    auto excute = toJS(context_);
//    LOG_TEST("[Context] JSCFunction Call func excute :%p",excute);
//    auto newConText = toRef(excute);
//    LOG_TEST("[Context] JSCFunction Call func newConText :%p",newConText);



//    LOG_TEST("[Context] JSCFunction Call func: cur_ctx:%p,self_context:%p thiz:%p,",cur_context,context_,thiz_);
    argv_js[i] = Conversion::RuntimeValueToJSValue(cur_context, nullptr,
                                                   argv[i].get());
  }

  return JSUtils::CallAsFunction(static_cast<JSContextRef>(cur_context),
                                 static_cast<JSObjectRef>(thiz_),
                                 static_cast<JSObjectRef>(function_),
                                 argc, argv_js);
}

/////////////////////////////////////////////////////////////////////////////
/* class JSCMap implemention */
std::unique_ptr<Map> JSCMap::Create(JSContextRef ctx, JSObjectRef thiz) {
  Map* ptr = static_cast<Map*>(new JSCMap(ctx, thiz));
  return std::unique_ptr<Map>(ptr);
}

// Map create from native
std::unique_ptr<Map> Map::CreateFromNative(EngineContext* context,
                                           ScopeValues thiz) {
  JSContextRef ctx = static_cast<JSContextRef>(context->GetContext());
  JSObjectRef thiz_object = nullptr;
  if (thiz->IsObject()) {
    auto object = thiz->GetAsObject();
    RuntimeObject* native_thiz =
                   static_cast<RuntimeObject*>(object->GetDataPtr());
    thiz_object = native_thiz->GetJSObject();
  }
  return JSCMap::Create(ctx, thiz_object);
}

JSCMap::JSCMap(JSContextRef ctx, JSObjectRef thiz)
        : Map(),
          context_(ctx),
          thiz_(thiz) {
  JSValueProtect(context_, thiz_);
}

JSCMap::~JSCMap() {
  JSValueUnprotect(context_, thiz_);
}

/////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Array> Array::CreateFromNative(EngineContext* context,
                                            ScopeValues thiz){
   JSContextRef ctx = static_cast<JSContextRef>(context->GetContext());
   JSObjectRef thiz_object = nullptr;
   if (thiz->IsObject()) {
        auto object = thiz->GetAsObject();
        RuntimeObject* native_thiz = static_cast<RuntimeObject*>(object->GetDataPtr());
        thiz_object = native_thiz->GetJSObject();
   }
   return JSCArray::Create(ctx, thiz_object);
}



/* class JSCArray implemention */
std::unique_ptr<Array> JSCArray::Create(JSContextRef ctx, JSObjectRef thiz) {
  //LOG_TEST("JSCArray before Create");
  Array* ptr = static_cast<Array*>(new JSCArray(ctx, thiz));
  //LOG_TEST("JSCArray after Create");
  return std::unique_ptr<Array>(ptr);
}

JSCArray::JSCArray(JSContextRef ctx, JSObjectRef thiz)
          : Array(), context_(ctx), thiz_(thiz) {
  //LOG_TEST("JSCArray before JSValueProtect ctx:%p , thiz:%p",context_,thiz_);
  if (nullptr != ctx && nullptr != thiz){
    JSValueProtect(context_, thiz_);
  }
  //
  //LOG_TEST("JSCArray after JSValueProtect ");
}

JSCArray::~JSCArray() {
  if (nullptr != context_ && nullptr != thiz_){
    JSValueUnprotect(context_, thiz_);
  }
  //JSValueUnprotect(context_, thiz_);
}

}  // namespace unicorn
