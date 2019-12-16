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

#include "js_action.h"
#include "weex_env.h"

std::map<JSObjectRef, JSAction::jsFunction *>
    JSAction::jsFunction::objMap = std::map<JSObjectRef, JSAction::jsFunction *>();
std::mutex JSAction::jsFunction::mutex;

std::map<std::string, JSAction::jsFunction *>
    JSAction::functionMap = std::map<std::string, JSAction::jsFunction *>();
void JSAction::EvaluateScript(std::string js) {
  JSValueRef exception = nullptr;
  JSEvaluateScript(m_ContextRef,
                   JSStringCreateWithUTF8CString(js.c_str()),
                   nullptr,
                   nullptr,
                   0,
                   &exception);

  if (exception != nullptr) {
    if (JSValueIsNull(m_ContextRef, exception)) {
      return;
    }

    JSStringRef str = JSValueToStringCopy(m_ContextRef, exception, nullptr);
    std::string result;
    size_t max_bytes = JSStringGetMaximumUTF8CStringSize(str);
    result.resize(max_bytes);
    size_t bytes_written = JSStringGetUTF8CString(str, &(result)[0], max_bytes);
    if (max_bytes == 0) {
      return;
    }
    result.resize(bytes_written - 1);
    JSStringRelease(str);
    std::unique_ptr<BackToWeexCoreQueue> &ptr = WeexEnv::getEnv()->m_back_to_weex_core_thread;
    if (ptr == nullptr || ptr.get() == nullptr) {
      return;
    }
    BackToWeexCoreQueue *bwq = ptr.get();

    BackToWeexCoreQueue::IPCTask
        *t = new BackToWeexCoreQueue::IPCTask(IPCProxyMsg::JSACTIONCALLBACK);

    auto temp = std::to_string(this->m_ctxContainer);
    t->addParams(temp.c_str(), temp.length());

    temp = std::to_string(static_cast<int32_t >(JSCALLBACK::EXCEPTION));
    t->addParams(temp.c_str(), temp.length());
    t->addParams(result.c_str(), result.length());
    bwq->addTask(t);
  }
}
void JSAction::BindFunction(std::string name) {
  JSAction::jsFunction *&pJsFunction = functionMap[name];
  if (pJsFunction != nullptr) {
    delete pJsFunction;
  }
  JSAction::jsFunction
      *pFunction = new JSAction::jsFunction(name, m_ContextRef, this->m_ctxContainer);
  functionMap[name] = pFunction;
}

JSValueRef JSAction::jsFunction::StaticFunctionCallback(JSContextRef ctx,
                                                        JSObjectRef function,
                                                        JSObjectRef thisObject,
                                                        size_t argumentCount,
                                                        JSValueRef const *arguments,
                                                        JSValueRef *exception) {
  jsFunction *&pFunction = objMap[function];
  if (pFunction == nullptr) {
    return nullptr;
  }

  std::unique_ptr<BackToWeexCoreQueue> &ptr = WeexEnv::getEnv()->m_back_to_weex_core_thread;
  if (ptr == nullptr || ptr.get() == nullptr) {
    return nullptr;
  }

  BackToWeexCoreQueue *bwq = ptr.get();

  const char *name = pFunction->m_name_.c_str();

  std::string result;
  if (argumentCount > 0) {
    JSValueRef const pValue = arguments[argumentCount - 1];
    if (JSValueIsString(ctx, pValue)) {
      JSStringRef pString = JSValueToStringCopy(ctx, pValue, nullptr);
      size_t max_bytes = JSStringGetMaximumUTF8CStringSize(pString);
      result.resize(max_bytes);
      size_t bytes_written = JSStringGetUTF8CString(pString, &(result)[0], max_bytes);
      if (max_bytes == 0) {
        return nullptr;
      }
      result.resize(bytes_written - 1);
      JSStringRelease(pString);
    }
  }

  auto
      *t = new BackToWeexCoreQueue::IPCTask(IPCProxyMsg::JSACTIONCALLBACK);

  auto temp = std::to_string(pFunction->m_ctxContainer);
  t->addParams(temp.c_str(), temp.length());
  temp = std::to_string(static_cast<int32_t >(JSCALLBACK::INVOKE));
  t->addParams(temp.c_str(), temp.length());
  t->addParams(name, strlen(name));
  t->addParams(result.c_str(), result.size());

  LOGD_TAG("JSEngine","CallBack %s -> %s", name,result.c_str());

  auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
      new BackToWeexCoreQueue::Future());
  t->set_future(future.get());
  bwq->addTask(t);
  std::unique_ptr<IPCResult> ipc_ret = future.get()->waitResult();

  if (ipc_ret->getType() == IPCType::CHARARRAY) {
    const char *string = ipc_ret.get()->getByteArrayContent();
    if (string == nullptr) {
      return JSValueMakeNull(ctx);
    }
    JSStringRef pJSString = JSStringCreateWithUTF8CString(string);
    return JSValueMakeString(ctx, pJSString);
  }
  return JSValueMakeNull(ctx);
}
JSObjectRef JSAction::jsFunction::StaticConstructorCallback(JSContextRef ctx,
                                                            JSObjectRef constructor,
                                                            size_t argumentCount,
                                                            JSValueRef const *arguments,
                                                            JSValueRef *exception) {
  return nullptr;
}
bool JSAction::jsFunction::StaticHasInstanceCallback(JSContextRef ctx,
                                                     JSObjectRef constructor,
                                                     JSValueRef possibleInstance,
                                                     JSValueRef *exception) {
  return false;
}
void JSAction::jsFunction::StaticFinalizeCallback(JSObjectRef object) {
}
JSValueRef JSAction::jsFunction::GetProperty(JSContextRef ctx,
                                             JSObjectRef object,
                                             JSStringRef propertyName,
                                             JSValueRef *exception) {

  std::string result;
  size_t max_bytes = JSStringGetMaximumUTF8CStringSize(propertyName);
  result.resize(max_bytes);
  size_t bytes_written = JSStringGetUTF8CString(propertyName, &(result)[0], max_bytes);
  if (max_bytes == 0) {
    return nullptr;
  }
  result.resize(bytes_written - 1);

  if (JSObjectHasProperty(ctx, object, propertyName)) {
    return JSObjectGetProperty(ctx, object, propertyName, nullptr);
  }

  return JSValueMakeUndefined(ctx);
}

namespace {
std::once_flag hostObjectClassOnceFlag;
JSClassRef hostObjectClass{};
} // namespace

JSAction::jsFunction::jsFunction(std::string name, JSContextRef ctx,
                                 long ctxContainer) {
  ctxRef = ctx;
  m_ctxContainer = ctxContainer;

  std::call_once(hostObjectClassOnceFlag, []() {
    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.version = 0;
    definition.finalize = StaticFinalizeCallback;
    definition.callAsFunction = StaticFunctionCallback;
    definition.callAsConstructor = StaticConstructorCallback;
    definition.hasInstance = StaticHasInstanceCallback;
    hostObjectClass = JSClassCreate(&definition);
  });

  m_name_ = name;
  JSStringRef jssName = JSStringCreateWithUTF8CString(name.c_str());
  objRef = JSObjectMake(ctx, hostObjectClass, jssName);
  JSValueProtect(ctx, objRef);
  JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), jssName, objRef, 0,
                      nullptr);
  mutex.lock();
  objMap[objRef] = this;
  mutex.unlock();
}

JSAction::jsFunction::~jsFunction() {

}
