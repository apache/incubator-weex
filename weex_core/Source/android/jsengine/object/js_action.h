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

#ifndef WEEX_PROJECT_RUN_JS_NOT_FOR_WEEX_H
#define WEEX_PROJECT_RUN_JS_NOT_FOR_WEEX_H

#include <string>

#ifdef USE_JS_RUNTIME
#include "js_runtime/weex/task/back_to_weex_core_queue.h"
#include "js_runtime/weex/object/weex_global_object_v2.h"
#include "js_runtime/runtime/jsc/jsc_utils.h"
#else
#include "android/jsengine/task/back_to_weex_core_queue.h"
#include "android/jsengine/object/weex_global_object.h"
#include "android/jsengine/weex_jsc_utils.h"
#endif

class JSAction {
 public:
  JSAction(long ctxContainer) {
    this->m_ctxContainer = ctxContainer;
    JSGlobalContextRef pContext = JSGlobalContextCreateInGroup(nullptr, nullptr);
    m_ContextRef = (JSContextRef) pContext;
    m_globalObjectRef = JSContextGetGlobalObject(m_ContextRef);
    JSValueProtect(m_ContextRef, m_globalObjectRef);
  }

  ~JSAction() {
    if (m_ContextRef != nullptr) {
      JSGlobalContextRelease((JSGlobalContextRef) m_ContextRef);
    }
  }

  void EvaluateScript(std::string js);

  void BindFunction(std::string name);

  void UnBindFunction(std::string name) {
    JSAction::jsFunction *&pFunction = functionMap[name];
    if (pFunction != nullptr) {
      delete pFunction;
    }
  }

  class jsFunction {
   public:
    jsFunction(std::string name, JSContextRef ctx, long ctxContainer);

    ~jsFunction();

   private:
    static JSValueRef StaticFunctionCallback(JSContextRef ctx,
                                             JSObjectRef function,
                                             JSObjectRef thisObject,
                                             size_t argumentCount,
                                             const JSValueRef arguments[],
                                             JSValueRef *exception);
    static JSObjectRef StaticConstructorCallback(JSContextRef ctx,
                                                 JSObjectRef constructor,
                                                 size_t argumentCount,
                                                 const JSValueRef arguments[],
                                                 JSValueRef *exception);
    static bool StaticHasInstanceCallback(JSContextRef ctx, JSObjectRef constructor,
                                          JSValueRef possibleInstance, JSValueRef *exception);

    static void StaticFinalizeCallback(JSObjectRef object);

    static JSValueRef GetProperty(JSContextRef ctx,
                                  JSObjectRef object,
                                  JSStringRef propertyName,
                                  JSValueRef *exception);
    std::string m_name_;

    long m_ctxContainer;

   private:
    JSObjectRef objRef;
    JSClassRef classRef;
    JSContextRef ctxRef;

    static std::map<JSObjectRef, JSAction::jsFunction *> objMap;
    static std::mutex mutex;

  };

 private:
  JSObjectRef m_globalObjectRef;
  JSContextRef m_ContextRef;
  long m_ctxContainer;
  static std::map<std::string, JSAction::jsFunction *> functionMap;
};

#endif //WEEX_PROJECT_RUN_JS_NOT_FOR_WEEX_H
