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

#include <core/bridge/platform_bridge.h>
#include "weexcore_impl_android.h"

namespace WeexCore {

  class Bridge_Impl_Android : public PlatformBridge {

//  private:
//    jobject jThis;
//    jobject jVMThis;

  public:
    static PlatformBridge *m_instance;

    Bridge_Impl_Android();

    virtual ~Bridge_Impl_Android();

    static void InitInstance(Bridge_Impl_Android* bridge) {
        m_instance = bridge;
    }

    static PlatformBridge *getInstance() {
      if (m_instance == nullptr) {
        m_instance = new PlatformBridge();
      }
      return m_instance;
    }

//    void setGlobalRef(jobject &jRef);
//
//    void setGlobalWMRef(jobject &jRef) {
//      jWMThis = jRef;
//    }
//
//    virtual void SetDefaultHeightAndWidthIntoRootDom(const std::string &instance_id,
//                                             float default_width,
//                                             float default_height,
//                                             bool is_width_wrap_content,
//                                             bool is_height_wrap_content) override ;
//    virtual void OnInstanceClose(const std::string &instance_id) override ;
//      virtual void SetStyleWidth(const std::string &instance_id, const std::string& render_ref, float width) override ;
//      virtual void SetStyleHeight(const std::string &instance_id, const std::string& render_ref, float height) override ;
//      virtual void SetMargin(const std::string &instance_id, const std::string& render_ref, int edge, float value) override ;
//      virtual void SetPadding(const std::string &instance_id, const std::string& render_ref, int edge, float value) override ;
//      virtual void SetPosition(const std::string &instance_id, const std::string& render_ref, int edge, float value) override ;
//      virtual void MarkDirty(const std::string &instance_id, const std::string& render_ref) override ;
//      virtual void SetViewPortWidth(const std::string &instance_id, float width) override ;
//      virtual void ForceLayout(const std::string &instance_id) override ;
//      virtual bool NotifyLayout(const std::string &instance_id) override ;
//      virtual std::vector<int64_t> GetFirstScreenRenderTime(const std::string &instance_id) override ;
//      virtual std::vector<int64_t> GetRenderFinishTime(const std::string &instance_id) override ;
//      virtual void SetRenderContainerWrapContent(const std::string &instance_id, bool wrap) override ;
//      virtual void BindMeasurementToRenderObject(long ptr) override ;
//      virtual void RegisterCoreEnv(const std::string& key, const std::string& value) override ;
//
//      virtual long GetRenderObject(const std::string &instance_id, const std::string& render_ref) override ;
//      virtual void UpdateRenderObjectStyle(long render_ptr, const std::string& key, const std::string& value) override ;
//      virtual void UpdateRenderObjectAttr(long render_ptr, const std::string& key, const std::string& value) override ;
//      virtual long CopyRenderObject(long render_ptr) override ;
//
//    ////////////////////////////////////
//    WXCoreSize InvokeMeasureFunction(const char* page_id, long render_ptr, float width, int width_measure_mode, float height, int height_measure_mode)override ;
//    void InvokeLayoutBefore(const char* page_id, long render_ptr) override ;
//    void InvokeLayoutAfter(const char* page_id, long render_ptr, float width, float height) override ;
//
//    void setJSVersion(const char* version);
//
//    void reportException(const char* pageId, const char *func, const char *exception_string);
//
//    int callNative(const char* pageId, const char *task, const char *callback);
//
//    std::unique_ptr<IPCResult> callNativeModule(const char* pageId, const char *module, const char *method,
//                             const char *arguments, int argumentsLength, const char *options, int optionsLength);
//
//    void callNativeComponent(const char* pageId, const char* ref, const char *method,
//                             const char *arguments, int argumentsLength, const char *options, int optionsLength);
//
//    void setTimeout(const char* callbackID, const char* time);
//
//    void callNativeLog(const char* str_array);
//
//    int callUpdateFinish(const char* pageId, const char *task, const char *callback);
//
//    int callRefreshFinish(const char* pageId, const char *task, const char *callback);
//
//    int callAddEvent(const char* pageId, const char* ref, const char *event);
//
//    int callRemoveEvent(const char* pageId, const char* ref, const char *event);
//
//    int callCreateBody(const char* pageId, const char *componentType, const char* ref,
//                       std::map<std::string, std::string> *styles,
//                       std::map<std::string, std::string> *attributes,
//                       std::set<std::string> *events,
//                       const WXCoreMargin &margins,
//                       const WXCorePadding &paddings,
//                       const WXCoreBorderWidth &borders);
//
//    int callAddElement(const char* pageId, const char *componentType,
//                       const char* ref, int &index, const char* parentRef,
//                       std::map<std::string, std::string> *styles,
//                       std::map<std::string, std::string> *attributes,
//                       std::set<std::string> *events,
//                       const WXCoreMargin &margins,
//                       const WXCorePadding &paddings,
//                       const WXCoreBorderWidth &borders,
//                       bool willLayout= true);
//
//    int callRemoveElement(const char* pageId, const char* ref);
//
//    int callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index);
//
//    int callLayout(const char* pageId, const char* ref,
//                   int top, int bottom, int left, int right,
//                   int height, int width, int index);
//
//    int callUpdateStyle(const char* pageId, const char* ref,
//                        std::vector<std::pair<std::string, std::string>> *style,
//                        std::vector<std::pair<std::string, std::string>> *margin,
//                        std::vector<std::pair<std::string, std::string>> *padding,
//                        std::vector<std::pair<std::string, std::string>> *border);
//
//    int callUpdateAttr(const char* pageId, const char* ref,
//                       std::vector<std::pair<std::string, std::string>> *attrs);
//
//    int callCreateFinish(const char* pageId);
//
//    int callAppendTreeCreateFinish(const char* pageId, const char* ref);
//
//    int callHasTransitionPros(const char* pageId, const char* ref,
//                              std::vector<std::pair<std::string, std::string>> *style);
//    void handlePostMessage(const char* vm_id, const char *data) override;
//
//    void handleDispatchMessage(const char* client_id, const char* vm_id, const char *data, const char* callback) override;
//
//    jobject getMeasureFunc(const char* pageId, jlong renderObjectPtr);
  };
} //end WeexCore
#endif //BridgeAndroid_h