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

#include "core/bridge/platform_bridge.h"
#include "core/layout/measure_func_adapter.h"

namespace WeexCore
{
    class IOSSide : public PlatformBridge::PlatformSide
    {
    public:
        WXCoreSize InvokeMeasureFunction(const char* page_id,
                                         long render_ptr, float width,
                                         int width_measure_mode,
                                         float height,
                                         int height_measure_mode) override { return WXCoreSize(); };
        void InvokeLayoutBefore(const char* page_id, long render_ptr) override {};
        void InvokeLayoutPlatform(const char* page_id, long render_ptr) override;
        void InvokeLayoutAfter(const char* page_id, long render_ptr,
                               float width, float height) override {};
        
        void TriggerVSync(const char* page_id) override;
        
        void SetJSVersion(const char* version) override;
        
        void ReportException(const char* pageId, const char *func, const char *exception_string) override;
        void ReportServerCrash(const char* instance_id) override {};
        void ReportNativeInitStatus(const char* status_code,
                                    const char* error_msg) override {};
        
        int CallNative(const char* pageId, const char *task, const char *callback) override;
        
        std::unique_ptr<ValueWithType> CallNativeModule(const char* pageId, const char *module, const char *method,
                               const char *arguments, int argumentsLength, const char *options, int optionsLength) override;
        
        void CallNativeComponent(const char* pageId, const char* ref, const char *method,
                                 const char *arguments, int argumentsLength, const char *options, int optionsLength) override;
        
        void SetTimeout(const char* callbackID, const char* time) override ;
        
        void NativeLog(const char* str_array) override ;
        
        int UpdateFinish(const char* page_id, const char* task, int taskLen,
                         const char* callback, int callbackLen) override;
        
        int RefreshFinish(const char* pageId, const char *task, const char *callback) override;
        
        int AddEvent(const char* pageId, const char* ref, const char *event) override;
        
        int RemoveEvent(const char* pageId, const char* ref, const char *event) override;
        
        int CreateBody(const char* pageId, const char *componentType, const char* ref,
                           std::map<std::string, std::string> *styles,
                           std::map<std::string, std::string> *attributes,
                           std::set<std::string> *events,
                           const WXCoreMargin &margins,
                           const WXCorePadding &paddings,
                           const WXCoreBorderWidth &borders) override;
        
        int AddElement(const char* pageId, const char *componentType, const char* ref,
                           int &index, const char* parentRef,
                           std::map<std::string, std::string> *styles,
                           std::map<std::string, std::string> *attributes,
                           std::set<std::string> *events,
                           const WXCoreMargin &margins,
                           const WXCorePadding &paddings,
                           const WXCoreBorderWidth &borders,
                           bool willLayout= true) override;
        
        int Layout(const char* pageId, const char* ref,
                       float top, float bottom, float left, float right,
                       float height, float width, int index) override;
        
        int UpdateStyle(const char* pageId, const char* ref,
                            std::vector<std::pair<std::string, std::string>> *style,
                            std::vector<std::pair<std::string, std::string>> *margin,
                            std::vector<std::pair<std::string, std::string>> *padding,
                            std::vector<std::pair<std::string, std::string>> *border) override;
        
        int UpdateAttr(const char* pageId, const char* ref,
                           std::vector<std::pair<std::string, std::string>> *attrs) override;
        
        int CreateFinish(const char* pageId) override;
        
        int RenderSuccess(const char* pageId) override;
        
        int RemoveElement(const char* pageId, const char* ref) override;
        
        int MoveElement(const char* pageId, const char* ref, const char* parentRef, int index) override;
        
        int AppendTreeCreateFinish(const char* pageId, const char* ref) override;
    
        int HasTransitionPros(const char* pageId, const char* ref,
                                          std::vector<std::pair<std::string, std::string>> *style) override;
        void PostMessage(const char* vm_id, const char* data, int dataLength) override {};
        void DispatchMessage(const char* client_id,
                             const char* data, int dataLength, const char* callback, const char* vm_id) override {};
        void OnReceivedResult(long callback_id, std::unique_ptr<WeexJSResult>& result) override {};
    };
    
    class WXCoreMeasureFunctionBridge : public MeasureFunctionAdapter
    {
    public:
        WXCoreSize Measure(const char* page_id, long render_ptr, float width, MeasureMode widthMeasureMode,
                           float height, MeasureMode heightMeasureMode);
        
        void LayoutBefore(const char* page_id, long render_ptr);
        
        void LayoutAfter(const char* page_id, long render_ptr, float width, float height);
    };
}

#endif

#import <Foundation/Foundation.h>

// For Objective-C use

@interface WXCoreBridge : NSObject

+ (void)install;

+ (void)createDataRenderInstance:(NSString *)pageId template:(NSString *)temp options:(NSDictionary *)options data:(id)data;

+ (void)createDataRenderInstance:(NSString *)pageId contents:(NSData *)contents options:(NSDictionary *)options  data:(id)data;

+ (void)destroyDataRenderInstance:(NSString *)pageId;

+ (void)refreshDataRenderInstance:(NSString *)pageId data:(NSString *)data;

+ (void)setDefaultDimensionIntoRoot:(NSString*)pageId width:(CGFloat)width height:(CGFloat)height
                 isWidthWrapContent:(BOOL)isWidthWrapContent
                isHeightWrapContent:(BOOL)isHeightWrapContent;

+ (void)setViewportWidth:(NSString*)pageId width:(CGFloat)width;

+ (void)layoutPage:(NSString*)pageId forced:(BOOL)forced;

+ (void)closePage:(NSString*)pageId;

+ (void)layoutRenderObject:(void*)object size:(CGSize)size page:(NSString*)pageId;

+ (void*)copyRenderObject:(void*)source replacedRef:(NSString*)ref;

+ (void)addChildRenderObject:(void*)child toParent:(void*)parent;

+ (void)removeRenderObjectFromMap:(NSString*)pageId object:(void*)object;

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

+ (void)fireEvent:(NSString *)pageId ref:(NSString *)ref event:(NSString *)event args:(NSDictionary *)args;

+ (void)registerModules:(NSDictionary *)modules;

@end

#endif
