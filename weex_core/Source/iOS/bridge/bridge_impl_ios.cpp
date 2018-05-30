/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"){} you may not use this file except in compliance
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

#include "bridge_impl_ios.h"
#include <core/bridge/wx_type_define.h>


namespace WeexCore {
    
    void Bridge_Impl_iOS::setJSVersion(const char* version){
        bridgeImpl->setJSVersion(version);
    }
    
    void Bridge_Impl_iOS::reportException(const char* pageId, const char *func, const char *exception_string){
        bridgeImpl->reportException(pageId, func, exception_string);
    }
    
    int Bridge_Impl_iOS::callNative(const char* pageId, const char *task, const char *callback){
        return bridgeImpl->callNative(pageId, task, callback);
    }
    
    void* Bridge_Impl_iOS::callNativeModule(const char* pageId, const char *module, const char *method,
                                   const char *arguments, int argumentsLength, const char *options, int optionsLength){
        return bridgeImpl->callNativeModule(pageId, module, method, arguments, argumentsLength, options, optionsLength);
    }
    
    void Bridge_Impl_iOS::callNativeComponent(const char* pageId, const char* ref, const char *method,
                             const char *arguments, int argumentsLength, const char *options, int optionsLength){
        bridgeImpl->callNativeComponent(pageId, ref, method, arguments, argumentsLength, options, optionsLength);
    }
    
    void Bridge_Impl_iOS::setTimeout(const char* callbackID, const char* time){
        bridgeImpl->setTimeout(callbackID, time);
    }
    
    void Bridge_Impl_iOS::callNativeLog(const char* str_array){
        bridgeImpl->callNativeLog(str_array);
    }
    
    int Bridge_Impl_iOS::callUpdateFinish(const char* pageId, const char *task, const char *callback){
        return bridgeImpl->callUpdateFinish(pageId, task, callback);
    }
    
    int Bridge_Impl_iOS::callRefreshFinish(const char* pageId, const char *task, const char *callback){
        return bridgeImpl->callRefreshFinish(pageId, task, callback);
    }
    
    int Bridge_Impl_iOS::callAddEvent(const char* pageId, const char* ref, const char *event){
        return bridgeImpl->callAddEvent(pageId, ref, event);
    }
    
    int Bridge_Impl_iOS::callRemoveEvent(const char* pageId, const char* ref, const char *event){
        return bridgeImpl->callRemoveEvent(pageId, ref, event);
    }
    
    int Bridge_Impl_iOS::callCreateBody(const char* pageId, const char *componentType, const char* ref,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders){
        return bridgeImpl->callCreateBody(pageId, componentType, ref, styles, attributes, events, margins, paddings, borders);
    }
    
    int Bridge_Impl_iOS::callAddElement(const char* pageId, const char *componentType, const char* ref,
                       int &index, const char* parentRef,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders,
                       bool willLayout){
        return bridgeImpl->callAddElement(pageId, componentType, ref, index, parentRef, styles, attributes, events, margins, paddings, borders);
    }
    
    int Bridge_Impl_iOS::callLayout(const char* pageId, const char* ref,
                   int top, int bottom, int left, int right,
                   int height, int width, int index){
        return bridgeImpl->callLayout(pageId, ref, top, bottom, left, right, height, width, index);
    }
    
    int Bridge_Impl_iOS::callUpdateStyle(const char* pageId, const char* ref,
                        std::vector<std::pair<std::string, std::string>> *style,
                        std::vector<std::pair<std::string, std::string>> *margin,
                        std::vector<std::pair<std::string, std::string>> *padding,
                        std::vector<std::pair<std::string, std::string>> *border){
        return bridgeImpl->callUpdateStyle(pageId, ref, style, margin, padding, border);
    }
    
    int Bridge_Impl_iOS::callUpdateAttr(const char* pageId, const char* ref,
                       std::vector<std::pair<std::string, std::string>> *attrs){
        return bridgeImpl->callUpdateAttr(pageId, ref, attrs);
    }
    
    int Bridge_Impl_iOS::callCreateFinish(const char* pageId){
        return bridgeImpl->callCreateFinish(pageId);
    }
    
    int Bridge_Impl_iOS::callRemoveElement(const char* pageId, const char* ref){
        return bridgeImpl->callRemoveElement(pageId, ref);
    }
    
    int Bridge_Impl_iOS::callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index){
        return bridgeImpl->callMoveElement(pageId, ref, parentRef, index);
    }
    
    int Bridge_Impl_iOS::callAppendTreeCreateFinish(const char* pageId, const char* ref){
        return bridgeImpl->callAppendTreeCreateFinish(pageId, ref);
    }
}
