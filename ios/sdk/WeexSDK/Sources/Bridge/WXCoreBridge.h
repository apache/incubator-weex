/*
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

#ifndef WXCORE_BRIDGE_PLATFORM_H
#define WXCORE_BRIDGE_PLATFORM_H

#if defined __cplusplus

#include "core/bridge/bridge.h"
#include "core/layout/measure_func_adapter.h"

namespace WeexCore
{
    class WXCoreBridge : public Bridge
    {
    public:
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
        
        int callAddElement(const char* pageId, const char *componentType, const char* ref,
                           int &index, const char* parentRef,
                           std::map<std::string, std::string> *styles,
                           std::map<std::string, std::string> *attributes,
                           std::set<std::string> *events,
                           const WXCoreMargin &margins,
                           const WXCorePadding &paddings,
                           const WXCoreBorderWidth &borders,
                           bool willLayout= true);
        
        int callLayout(const char* pageId, const char* ref,
                       float top, float bottom, float left, float right,
                       float height, float width, int index);
        
        int callUpdateStyle(const char* pageId, const char* ref,
                            std::vector<std::pair<std::string, std::string>> *style,
                            std::vector<std::pair<std::string, std::string>> *margin,
                            std::vector<std::pair<std::string, std::string>> *padding,
                            std::vector<std::pair<std::string, std::string>> *border);
        
        int callUpdateAttr(const char* pageId, const char* ref,
                           std::vector<std::pair<std::string, std::string>> *attrs);
        
        int callCreateFinish(const char* pageId);
        
        int callRenderSuccess(const char* pageId);
        
        int callRemoveElement(const char* pageId, const char* ref);
        
        int callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index);
        
        int callAppendTreeCreateFinish(const char* pageId, const char* ref);
    
        int callHasTransitionPros(const char* pageId, const char* ref,
                                          std::vector<std::pair<std::string, std::string>> *style);
    };
    
    class WXCoreMeasureFunctionBridge : public MeasureFunctionAdapter
    {
    public:
        WXCoreSize Measure(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                           float height, MeasureMode heightMeasureMode);
        
        void LayoutBefore(WXCoreLayoutNode *node);
        
        void LayoutAfter(WXCoreLayoutNode *node, float width, float height);
    };
}

#endif

#import <Foundation/Foundation.h>

// For Objective-C use

@interface WXCoreBridge : NSObject

+ (void)install;

+ (void)createDataRenderInstance:(NSString *)pageId
                        template:(NSString *)temp
                         options:(NSDictionary *)options;

+ (void)destroyDataRenderInstance:(NSString *)pageId;

+ (void)refreshDataRenderInstance:(NSString *)pageId data:(id)data;

+ (void)setDefaultDimensionIntoRoot:(NSString*)pageId width:(CGFloat)width height:(CGFloat)height
                 isWidthWrapContent:(BOOL)isWidthWrapContent
                isHeightWrapContent:(BOOL)isHeightWrapContent;

+ (void)setViewportWidth:(NSString*)pageId width:(CGFloat)width;

+ (void)layoutPage:(NSString*)pageId size:(CGSize)size forced:(BOOL)forced;

+ (void)closePage:(NSString*)pageId;

+ (void)layoutRenderObject:(void*)object size:(CGSize)size page:(NSString*)pageId;

+ (void*)copyRenderObject:(void*)source replacedRef:(NSString*)ref;

+ (void)addChildRenderObject:(void*)child toParent:(void*)parent;

+ (void)callAddElement:(NSString*)pageId parentRef:(NSString*)parentRef data:(NSDictionary*)data index:(int)index;

+ (void)callCreateBody:(NSString*)pageId data:(NSDictionary*)data;

+ (void)callRemoveElement:(NSString*)pageId ref:(NSString*)ref;

+ (void)callMoveElement:(NSString*)pageId ref:(NSString*)ref parentRef:(NSString*)parentRef index:(int)index;

+ (void)callUpdateAttrs:(NSString*)pageId ref:(NSString*)ref data:(NSDictionary*)data;

+ (void)callUpdateStyle:(NSString*)pageId ref:(NSString*)ref data:(NSDictionary*)data;

+ (void)callAddEvent:(NSString*)pageId ref:(NSString*)ref event:(NSString*)event;

+ (void)callRemoveEvent:(NSString*)pageId ref:(NSString*)ref event:(NSString*)event;

+ (void)callCreateFinish:(NSString*)pageId;

+ (void)callRefreshFinish:(NSString*)pageId;

+ (void)callUpdateFinish:(NSString*)pageId;

@end

#endif
