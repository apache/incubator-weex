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
#ifndef BridgeAndroid_h
#define BridgeAndroid_h

#include <core/bridge/bridge.h>
#include "weexcore_impl_android.h"

namespace WeexCore {

  class Bridge_Impl_Android : public Bridge {

  private:
    jobject jThis;
    jobject jVMThis;

  public:
    static Bridge_Impl_Android *m_instance;

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (Bridge_Impl_Android::m_instance) {
          delete Bridge_Impl_Android::m_instance;
        }
      }
    };

    static Garbo garbo;

    Bridge_Impl_Android();

    ~Bridge_Impl_Android();

    static Bridge_Impl_Android *getInstance() {
      if (m_instance == nullptr) {
        m_instance = new Bridge_Impl_Android();
      }
      return m_instance;
    }

    void setGlobalRef(jobject &jRef);

    void setGlobalWMRef(jobject &jRef) {
      jWMThis = jRef;
    }

    void setJSVersion(const char* version);

    void reportException(const char* pageId, const char *func, const char *exception_string);

    int callNative(const char* pageId, const char *task, const char *callback);

    void* callNativeModule(const char* pageId, const char *module, const char *method,
                             const char *arguments, int argumentsLength, const char *options, int optionsLength);

    void callNativeComponent(const char* pageId, const char* ref, const char *method,
                             const char *arguments, int argumentsLength, const char *options, int optionsLength);

    void setTimeout(const char* callbackID, const char* time);

    void callNativeLog(const char* str_array);

    int callUpdateFinish(const char* pageId, const char *task, const char *callback);

    int callRefreshFinish(const char* pageId, const char *task, const char *callback);

    int callAddEvent(const char* pageId, const char* ref, const char *event);

    int callRemoveEvent(const char* pageId, const char* ref, const char *event);

    int callCreateBody(const char* pageId, const char *componentType, const char* ref,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callAddElement(const char* pageId, const char *componentType,
                       const char* ref, int &index, const char* parentRef,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders,
                       bool willLayout= true);

    int callRemoveElement(const char* pageId, const char* ref);

    int callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index);

    int callLayout(const char* pageId, const char* ref,
                   int top, int bottom, int left, int right,
                   int height, int width, int index);

    int callUpdateStyle(const char* pageId, const char* ref,
                        std::vector<std::pair<std::string, std::string>> *style,
                        std::vector<std::pair<std::string, std::string>> *margin,
                        std::vector<std::pair<std::string, std::string>> *padding,
                        std::vector<std::pair<std::string, std::string>> *border);

    int callUpdateAttr(const char* pageId, const char* ref,
                       std::vector<std::pair<std::string, std::string>> *attrs);

    int callCreateFinish(const char* pageId);

    int callAppendTreeCreateFinish(const char* pageId, const char* ref);

    int callHasTransitionPros(const char* pageId, const char* ref,
                              std::vector<std::pair<std::string, std::string>> *style);

    // will change future to char *
    void handlePostMessage(jstring jVmId, jbyteArray jData);

    void handleDispatchMessage(jstring jClientId, jstring jVmId, jbyteArray jData, jstring jCallback);

    jobject getMeasureFunc(const char* pageId, jlong renderObjectPtr);
  };
} //end WeexCore
#endif //BridgeAndroid_h