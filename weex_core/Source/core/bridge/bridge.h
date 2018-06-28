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
#ifndef Bridge_h
#define Bridge_h

//#include <jni.h>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace WeexCore {

  class WXCoreMargin;

  class WXCorePadding;

  class WXCoreBorderWidth;

  class Bridge {
  public:
    /**  platform  impl **/
    virtual void setJSVersion(const char* version) = 0;

    virtual void reportException(const char* pageId, const char *func, const char *exception_string) = 0;

    virtual int callNative(const char* pageId, const char *task, const char *callback) = 0;

    virtual void* callNativeModule(const char* pageId, const char *module, const char *method,
                                     const char *arguments, int argumentsLength, const char *options, int optionsLength) = 0;

    virtual void callNativeComponent(const char* pageId, const char* ref, const char *method,
                                     const char *arguments, int argumentsLength, const char *options, int optionsLength) = 0;

    virtual void setTimeout(const char* callbackID, const char* time) = 0;

    virtual void callNativeLog(const char* str_array) = 0;

    virtual int callUpdateFinish(const char* pageId, const char *task, const char *callback) = 0;

    virtual int callRefreshFinish(const char* pageId, const char *task, const char *callback) = 0;

    virtual int callAddEvent(const char* pageId, const char* ref, const char *event) = 0;

    virtual int callRemoveEvent(const char* pageId, const char* ref, const char *event) = 0;

    virtual int callCreateBody(const char* pageId, const char *componentType, const char* ref,
                               std::map<std::string, std::string> *styles,
                               std::map<std::string, std::string> *attributes,
                               std::set<std::string> *events,
                               const WXCoreMargin &margins,
                               const WXCorePadding &paddings,
                               const WXCoreBorderWidth &borders) = 0;

    virtual int callAddElement(const char* pageId, const char *componentType, const char* ref,
                               int &index, const char* parentRef,
                               std::map<std::string, std::string> *styles,
                               std::map<std::string, std::string> *attributes,
                               std::set<std::string> *events,
                               const WXCoreMargin &margins,
                               const WXCorePadding &paddings,
                               const WXCoreBorderWidth &borders,
                               bool willLayout= true) = 0;

    virtual int callLayout(const char* pageId, const char* ref,
                           int top, int bottom, int left, int right,
                           int height, int width, int index) = 0;

    virtual int callUpdateStyle(const char* pageId, const char* ref,
                                std::vector<std::pair<std::string, std::string>> *style,
                                std::vector<std::pair<std::string, std::string>> *margin,
                                std::vector<std::pair<std::string, std::string>> *padding,
                                std::vector<std::pair<std::string, std::string>> *border) = 0;

    virtual int callUpdateAttr(const char* pageId, const char* ref,
                               std::vector<std::pair<std::string, std::string>> *attrs) = 0;

    virtual int callCreateFinish(const char* pageId) = 0;

    virtual int callRemoveElement(const char* pageId, const char* ref) = 0;

    virtual int callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index) = 0;

    virtual int callAppendTreeCreateFinish(const char* pageId, const char* ref) = 0;
    
    virtual int callHasTransitionPros(const char* pageId, const char* ref,
                                        std::vector<std::pair<std::string, std::string>> *style)=0;

    /**  core api **/
    void setStyleWidth(const char *instanceId, const char *ref, float value);
    void setStyleHeight(const char *instanceId, const char *ref, float value);

    void setMargin(const char *instanceId,const char *ref, int32_t edge, float value);

    void setPadding(const char *instanceId, const char *ref, int32_t edge, float value);

    void setPosition(const char *instanceId, const char *ref, int32_t edge, float value);

    void markDirty(const char *instanceId,const  char *ref, bool dirty);

    void setDefaultHeightAndWidthIntoRootDom(const char *instanceId, const float defaultWidth, const float defaultHeight,
                                             const bool isWidthWrapContent, const bool isHeightWrapContent);
    void setRenderContainerWrapContent(const char* instanceId,bool wrap);

    /**Notice that this method is invoked from JS thread.**/
    void forceLayout(const char *instanceId);

    /**Notice that this method is invoked from JS thread.**/
    bool notifyLayout(const char *instanceId);


    void onInstanceClose(const char *instanceId) ;
  };
} //end WeexCore
#endif //Bridge_h
