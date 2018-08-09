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

#import "WXCoreBridge.h"
#import "JSValue+Weex.h"
#import "WXSDKManager.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"
#import "WXLog.h"
#import "WXTracingManager.h"
#import "WXBridgeProtocol.h"
#import "WXUtility.h"
#import "WXAppConfiguration.h"
#import "WsonObject.h"

#include "base/CoreConstants.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/page/render_page.h"
#include "core/render/node/render_object.h"
#include "core/render/node/render_list.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/factory/render_creator.h"
#include "core/config/core_environment.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#include "base/TimeUtils.h"

#import <objc/runtime.h>

#define NSSTRING(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCString(NULL, (const char *)(cstr), kCFStringEncodingUTF8)))
#define NSSTRING_NO_COPY(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCStringNoCopy(NULL, (const char *)(cstr), kCFStringEncodingUTF8, kCFAllocatorNull)))

namespace WeexCore
{
    static NSMutableDictionary* NSDICTIONARY(std::map<std::string, std::string>* map)
    {
        if (map == nullptr || map->size() == 0)
            return [[NSMutableDictionary alloc] init];
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:map->size()];
        for (auto it = map->begin(); it != map->end(); it ++) {
            NSString* object = NSSTRING(it->second.c_str());
            if ([WXUtility isStringPossiblelyJSONContainer:object]) {
                object = [WXUtility objectFromJSON:object];
            }
            [result setObject:object forKey:NSSTRING(it->first.c_str())];
        }
        return result;
    }
    
    static NSMutableDictionary* NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* vec)
    {
        if (vec == nullptr || vec->size() == 0)
            return [[NSMutableDictionary alloc] init];
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:vec->size()];
        for (auto& p : *vec) {
            NSString* object = NSSTRING(p.second.c_str());
            if ([WXUtility isStringPossiblelyJSONContainer:object]) {
                object = [WXUtility objectFromJSON:object];
            }
            [result setObject:object forKey:NSSTRING(p.first.c_str())];
        }
        return result;
    }
    
    static NSMutableArray* NSARRAY(std::set<std::string>* set)
    {
        if (set == nullptr || set->size() == 0)
            return [[NSMutableArray alloc] init];
        
        NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:set->size()];
        for (auto& s : *set) {
            NSString* object = NSSTRING(s.c_str());
            if ([WXUtility isStringPossiblelyJSONContainer:object]) {
                object = [WXUtility objectFromJSON:object];
            }
            [result addObject:object];
        }
        return result;
    }
    
    static void MergeBorderWidthValues(NSMutableDictionary* dict, const WXCoreBorderWidth & borders, bool isUpdate)
    {
        if (borders.getBorderWidth(kBorderWidthTop) != (float)0.0f || isUpdate) {
            dict[@"borderTopWidth"] = @(borders.getBorderWidth(kBorderWidthTop));
        }
        if (borders.getBorderWidth(kBorderWidthLeft) != (float)0.0f || isUpdate) {
            dict[@"borderLeftWidth"] = @(borders.getBorderWidth(kBorderWidthLeft));
        }
        if (borders.getBorderWidth(kBorderWidthBottom) != (float)0.0f || isUpdate) {
            dict[@"borderBottomWidth"] = @(borders.getBorderWidth(kBorderWidthBottom));
        }
        if (borders.getBorderWidth(kBorderWidthRight) != (float)0.0f || isUpdate) {
            dict[@"borderRightWidth"] = @(borders.getBorderWidth(kBorderWidthRight));
        }
    }
    
    static void MergeBorderWidthValues(NSMutableDictionary* dict, std::vector<std::pair<std::string, std::string>>* borders)
    {
        if (borders == nullptr) {
            return;
        }
        
        for (auto& p : *borders) {
            dict[NSSTRING(p.first.c_str())] = NSSTRING(p.second.c_str());
        }
    }
    
    void WXCoreBridge::setJSVersion(const char* version)
    {
        NSString *jsVersion = NSSTRING(version);
        if (jsVersion.length > 0) {
            [WXAppConfiguration setJSFrameworkVersion:jsVersion];
        }
    }
    
    void WXCoreBridge::reportException(const char* pageId, const char *func, const char *exception_string)
    {
        // should not enter this function
        assert(false);
    }
    
    int WXCoreBridge::callNative(const char* pageId, const char *task, const char *callback)
    {
        // should not enter this function
        assert(false);
    }
    
    void* WXCoreBridge::callNativeModule(const char* pageId, const char *module, const char *method,
                                         const char *arguments, int argumentsLength,
                                         const char *options, int optionsLength)
    {
        // should not enter this function
        assert(false);
    }
        
    void WXCoreBridge::callNativeComponent(const char* pageId, const char* ref, const char *method,
                                           const char *arguments, int argumentsLength,
                                           const char *options, int optionsLength)
    {
        // should not enter this function
        assert(false);
    }
        
    void WXCoreBridge::setTimeout(const char* callbackID, const char* time)
    {
        // should not enter this function
        assert(false);
    }
        
    void WXCoreBridge::callNativeLog(const char* str_array)
    {
        // should not enter this function
        assert(false);
    }
        
    int WXCoreBridge::callUpdateFinish(const char* pageId, const char *task, const char *callback)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        NSString* ns_instanceId = NSSTRING(pageId);
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager updateFinish];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateFinish" options:@{@"threadName":WXTDOMThread}];
        
        return 0;
    }
        
    int WXCoreBridge::callRefreshFinish(const char* pageId, const char *task, const char *callback)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        NSString* ns_instanceId = NSSTRING(pageId);
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager refreshFinish];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"refreshFinish" options:@{@"threadName":WXTDOMThread}];
        
        return 0;
    }
        
    int WXCoreBridge::callAddEvent(const char* pageId, const char* ref, const char *event)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_event = NSSTRING(event);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: addEvent ref:%@", ns_ref);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager wxcore_AddEvent:ns_event toElement:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"addEvent" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callRemoveEvent(const char* pageId, const char* ref, const char *event)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_event = NSSTRING(event);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action :removeEvent ref:%@", ns_ref);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager wxcore_RemoveEvent:ns_event fromElement:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"removeEvent" options:@{@"threadName":WXTDOMThread}];
    
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callCreateBody(const char* pageId, const char *componentType, const char* ref,
                                     std::map<std::string, std::string> *styles,
                                     std::map<std::string, std::string> *attributes,
                                     std::set<std::string> *events,
                                     const WXCoreMargin &margins,
                                     const WXCorePadding &paddings,
                                     const WXCoreBorderWidth &borders)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        RenderObject* renderObject = page->GetRenderObject(ref);
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_type = NSSTRING(componentType);
        NSMutableDictionary* ns_styles = NSDICTIONARY(styles);
        NSDictionary* ns_attributes = NSDICTIONARY(attributes);
        NSArray* ns_events = NSARRAY(events);
        MergeBorderWidthValues(ns_styles, borders, false);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: createBody %@ ref:%@", ns_type, ns_ref);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager wxcore_CreateBody:ns_ref type:ns_type styles:ns_styles attributes:ns_attributes events:ns_events renderObject:renderObject];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callAddElement(const char* pageId, const char *componentType, const char* ref,
                           int &index, const char* parentRef,
                           std::map<std::string, std::string> *styles,
                           std::map<std::string, std::string> *attributes,
                           std::set<std::string> *events,
                           const WXCoreMargin &margins,
                           const WXCorePadding &paddings,
                           const WXCoreBorderWidth &borders,
                           bool willLayout)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        RenderObject* renderObject = page->GetRenderObject(ref);
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_componentType = NSSTRING(componentType);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_parentRef = NSSTRING(parentRef);
        NSMutableDictionary* ns_styles = NSDICTIONARY(styles);
        NSDictionary* ns_attributes = NSDICTIONARY(attributes);
        NSArray* ns_events = NSARRAY(events);
        NSInteger ns_index = index;
        MergeBorderWidthValues(ns_styles, borders, false);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: addElement : %@", ns_componentType);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        [manager startComponentTasks];
        [manager wxcore_AddElement:ns_ref type:ns_componentType parentRef:ns_parentRef styles:ns_styles attributes:ns_attributes events:ns_events index:ns_index renderObject:renderObject];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"addElement" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int WXCoreBridge::callLayout(const char* pageId, const char* ref,
                       float top, float bottom, float left, float right,
                       float height, float width, int index)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        RenderObject* renderObject = page->GetRenderObject(ref);
        WXComponent* component = (__bridge WXComponent *)(renderObject->getContext());
        NSString* ns_instanceId = NSSTRING(pageId);
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        CGRect frame = CGRectMake(isnan(WXRoundPixelValue(left))?0:WXRoundPixelValue(left),
                                  isnan(WXRoundPixelValue(top))?0:WXRoundPixelValue(top),
                                  isnan(WXRoundPixelValue(width))?0:WXRoundPixelValue(width),
                                  isnan(WXRoundPixelValue(height))?0:WXRoundPixelValue(height));
        [manager wxcore_Layout:component frame:frame innerMainSize:renderObject->getLargestMainSize()];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int WXCoreBridge::callUpdateStyle(const char* pageId, const char* ref,
                            std::vector<std::pair<std::string, std::string>> *style,
                            std::vector<std::pair<std::string, std::string>> *margin,
                            std::vector<std::pair<std::string, std::string>> *padding,
                            std::vector<std::pair<std::string, std::string>> *border)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSMutableDictionary* ns_style = NSDICTIONARY(style);
        MergeBorderWidthValues(ns_style, border);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: updateStyles ref:%@, styles:%@", ns_ref, ns_style);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        [manager startComponentTasks];
        [manager wxcore_UpdateStyles:ns_style forElement:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateStyles" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callUpdateAttr(const char* pageId, const char* ref,
                           std::vector<std::pair<std::string, std::string>> *attrs)
    {
        if (attrs == nullptr) {
            return 0;
        }
        if (attrs->size() == 0) {
            return 0;
        }
        
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSDictionary* ns_attributes = NSDICTIONARY(attrs);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: updateAttrs ref:%@, attr:%@", ns_ref, ns_attributes);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager wxcore_UpdateAttributes:ns_attributes forElement:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callCreateFinish(const char* pageId)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);

#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: createFinish :%@", ns_instanceId);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager createFinish];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createFinish" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int WXCoreBridge::callRenderSuccess(const char* pageId)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: renderFinish :%@", ns_instanceId);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager renderFinish];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"renderFinish" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callRemoveElement(const char* pageId, const char* ref)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: removeElement ref:%@", ns_ref);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        [manager startComponentTasks];
        [manager wxcore_RemoveElement:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"removeElement" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_parentRef = NSSTRING(parentRef);
        NSInteger ns_index = index;
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: moveElement, ref:%@ to ref:%@", ns_ref, ns_parentRef);
#endif
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        [manager startComponentTasks];
        [manager wxcore_MoveElement:ns_ref toSuper:ns_parentRef atIndex:ns_index];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"moveElement" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callAppendTreeCreateFinish(const char* pageId, const char* ref)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);

        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager wxcore_AppendTreeCreateFinish:ns_ref];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int WXCoreBridge::callHasTransitionPros(const char* pageId, const char* ref,
                              std::vector<std::pair<std::string, std::string>> *style)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        // if transition is none, return directly, avoiding convert style
        if ([manager wxcore_IsTransitionNoneOfElement:ns_ref]) {
            return 0;
        }
        
        int result = [manager wxcore_HasTransitionPropertyInStyles:NSDICTIONARY(style) forElement:ns_ref] ? 1 : 0;
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return result;
    }
    
#pragma mark - Layout Impl
    
    WXCoreSize WXCoreMeasureFunctionBridge::Measure(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                                                    float height, MeasureMode heightMeasureMode)
    {
        // should not enter this function
        assert(false);
    }
    
    void WXCoreMeasureFunctionBridge::LayoutBefore(WXCoreLayoutNode *node)
    {
        
    }
    
    void WXCoreMeasureFunctionBridge::LayoutAfter(WXCoreLayoutNode *node, float width, float height)
    {
        
    }
}

@implementation WXCoreBridge

static WeexCore::WXCoreBridge* platformBridge = nullptr;
static WeexCore::JSBridge* jsBridge = nullptr;

+ (void)install
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        WeexCore::WXCoreEnvironment::getInstance()->SetPlatform(OS_iOS);
        
        CGSize screenSize = [UIScreen mainScreen].bounds.size;
        WeexCore::WXCoreEnvironment::getInstance()->SetDeviceWidth(std::to_string(screenSize.width));
        WeexCore::WXCoreEnvironment::getInstance()->SetDeviceHeight(std::to_string(screenSize.height));
        
        platformBridge = new WeexCore::WXCoreBridge();
        WeexCore::WeexCoreManager::getInstance()->setPlatformBridge(platformBridge);
        
        jsBridge = new WeexCore::JSBridge();
        WeexCore::WeexCoreManager::getInstance()->setJSBridge(jsBridge);
        
        WeexCore::WeexCoreManager::getInstance()->SetMeasureFunctionAdapter(new WeexCore::WXCoreMeasureFunctionBridge());
    });
}

+ (void)createInstance:(NSString *)pageId
              template:(NSString *)jsBundleString
               options:(NSDictionary *)options
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    NSString *optionsString = [WXUtility JSONString:options];
    
    node_manager->CreatePage([jsBundleString UTF8String], [pageId UTF8String], [optionsString UTF8String]);
}

+ (void)destroyInstance:(NSString *)pageId
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    node_manager->ClosePage([pageId UTF8String]);
}

+ (void)refreshInstance:(NSString *)pageId data:(id)data;
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    node_manager->RefreshPage([pageId UTF8String], [data UTF8String]);
}


+ (void)setDefaultDimensionIntoRoot:(NSString*)pageId width:(CGFloat)width height:(CGFloat)height
                 isWidthWrapContent:(BOOL)isWidthWrapContent
                isHeightWrapContent:(BOOL)isHeightWrapContent
{
    if (platformBridge) {
        platformBridge->setDefaultHeightAndWidthIntoRootDom([pageId UTF8String], (float)width, (float)height, (bool)isWidthWrapContent, (bool)isHeightWrapContent);
    }
}

+ (void)setViewportWidth:(NSString*)pageId width:(CGFloat)width
{
    if (platformBridge) {
        platformBridge->setViewportWidth([pageId UTF8String], (float)width);
    }
}

+ (void)layoutPage:(NSString*)pageId size:(CGSize)size forced:(BOOL)forced
{
    if (platformBridge) {
        const char* page = [pageId UTF8String];
        if (forced) {
            platformBridge->setPageDirty(page);
        }
        
        if (platformBridge->notifyLayout(page)) {
            platformBridge->setDefaultHeightAndWidthIntoRootDom(page, size.width, size.height, false, false);
            platformBridge->forceLayout(page);
        }
    }
}

static void _traverseTree(WeexCore::RenderObject *render, int index, const char* pageId)
{
    using namespace WeexCore;
    if (render == nullptr) return;

    if (render->hasNewLayout()) {
        /* do not call bridge->callLayout because render is not registered to page, so that
         page->GetRenderObject will not give the correct object. */
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page != nullptr) {
            WXComponent* component = (__bridge WXComponent *)(render->getContext());
            NSString* ns_instanceId = NSSTRING(pageId);
            
            float top = render->getLayoutPositionTop();
            float left = render->getLayoutPositionLeft();
            float height = render->getLayoutHeight();
            float width = render->getLayoutWidth();
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            CGRect frame = CGRectMake(isnan(WXRoundPixelValue(left))?0:WXRoundPixelValue(left),
                                      isnan(WXRoundPixelValue(top))?0:WXRoundPixelValue(top),
                                      isnan(WXRoundPixelValue(width))?0:WXRoundPixelValue(width),
                                      isnan(WXRoundPixelValue(height))?0:WXRoundPixelValue(height));
            [manager wxcore_Layout:component frame:frame innerMainSize:render->getLargestMainSize()];
        }
        render->setHasNewLayout(false);
    }

    for (auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it ++) {
        WeexCore::RenderObject *child = static_cast<WeexCore::RenderObject *>(*it);
        if (child != nullptr) {
            _traverseTree(child, (int)(it - render->ChildListIterBegin()), pageId);
        }
    }
}

+ (void)layoutRenderObject:(void*)object size:(CGSize)size page:(NSString*)pageId
{
    using namespace WeexCore;
    RenderObject* render = static_cast<RenderObject*>(object);
    std::pair<float, float> renderPageSize(size.width, size.height);
    
    render->LayoutBeforeImpl();
    render->calculateLayout(renderPageSize);
    render->LayoutAfterImpl();
    _traverseTree(render, 0, [pageId UTF8String]);
}

+ (void*)copyRenderObject:(void*)source replacedRef:(NSString*)ref
{
    using namespace WeexCore;
    RenderObject* sourceObject = static_cast<RenderObject*>(source);
    RenderObject* copyObject = static_cast<RenderObject*>(RenderCreator::GetInstance()->CreateRender(sourceObject->type(), sourceObject->ref()));
                                                          
    copyObject->CopyFrom(sourceObject);
    if (ref != nil) {
        copyObject->set_ref([ref UTF8String]);
    }

    if (sourceObject->type() == kRenderCellSlot || sourceObject->type() == kRenderCell) {
        RenderList* renderList = static_cast<RenderList*>(sourceObject->getParent());
        if (renderList != nullptr) {
            renderList->AddCellSlotCopyTrack(copyObject);
        }
    }
    return copyObject;
}

+ (void)addChildRenderObject:(void*)child toParent:(void*)parent
{
    (static_cast<WeexCore::RenderObject*>(parent))->AddRenderObject(-1, (static_cast<WeexCore::RenderObject*>(child)));
}

static WeexCore::RenderObject* _parseRenderObject(NSDictionary* data, WeexCore::RenderObject* parent,
                                                  int index, const std::string& pageId)
{
    using namespace WeexCore;
    
    const char* type = [data[@"type"] UTF8String];
    const char* ref = [data[@"ref"] UTF8String];
    if (type != nullptr && ref != nullptr) {
        RenderObject* render = (RenderObject *)RenderCreator::GetInstance()->CreateRender(type, ref);
        render->set_page_id(pageId);
        if (parent != nullptr){
            parent->AddRenderObject(index, render);
        }
        
        [data[@"attr"] enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
            if ([obj isKindOfClass:[NSString class]]) {
                render->AddAttr([key UTF8String], [obj UTF8String]);
            }
            else if ([obj isKindOfClass:[NSNumber class]]) {
                NSNumber* num = (NSNumber*)obj;
                char objcType = [num objCType][0];
                char buffer[128];
                switch (objcType) {
                    case _C_DBL:
                    case _C_FLT:
                        snprintf(buffer, sizeof(buffer), "%f", [num doubleValue]);
                        render->AddAttr([key UTF8String], buffer);
                        break;
                    case _C_INT:
                    case _C_CHR:
                    case _C_SHT:
                        snprintf(buffer, sizeof(buffer), "%d", [num intValue]);
                        render->AddAttr([key UTF8String], buffer);
                        break;
                    case _C_USHT:
                    case _C_UINT:
                    case _C_UCHR:
                        snprintf(buffer, sizeof(buffer), "%u", [num unsignedIntValue]);
                        render->AddAttr([key UTF8String], buffer);
                        break;
                    case _C_LNG:
                    case _C_LNG_LNG:
                        snprintf(buffer, sizeof(buffer), "%lld", [num longLongValue]);
                        render->AddAttr([key UTF8String], buffer);
                        break;
                    case _C_ULNG:
                    case _C_ULNG_LNG:
                        snprintf(buffer, sizeof(buffer), "%llu", [num unsignedLongLongValue]);
                        render->AddAttr([key UTF8String], buffer);
                        break;
                    case _C_BFLD:
                    case _C_BOOL:
                        render->AddAttr([key UTF8String], [num boolValue] ? "true" : "false");
                        break;
                    default:
                        render->AddAttr([key UTF8String], [[num stringValue] UTF8String]);
                        break;
                }
            }
            else {
                render->AddAttr([key UTF8String], [[WXUtility JSONString:obj] UTF8String]);
            }
        }];
        
        [data[@"style"] enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
            if ([obj isKindOfClass:[NSString class]]) {
                render->AddStyle([key UTF8String], [obj UTF8String]);
            }
            else if ([obj isKindOfClass:[NSNumber class]]) {
                NSNumber* num = (NSNumber*)obj;
                char objcType = [num objCType][0];
                char buffer[128];
                switch (objcType) {
                    case _C_DBL:
                    case _C_FLT:
                        snprintf(buffer, sizeof(buffer), "%f", [num doubleValue]);
                        render->AddStyle([key UTF8String], buffer);
                        break;
                    case _C_INT:
                    case _C_CHR:
                    case _C_SHT:
                        snprintf(buffer, sizeof(buffer), "%d", [num intValue]);
                        render->AddStyle([key UTF8String], buffer);
                        break;
                    case _C_USHT:
                    case _C_UINT:
                    case _C_UCHR:
                        snprintf(buffer, sizeof(buffer), "%u", [num unsignedIntValue]);
                        render->AddStyle([key UTF8String], buffer);
                        break;
                    case _C_LNG:
                    case _C_LNG_LNG:
                        snprintf(buffer, sizeof(buffer), "%lld", [num longLongValue]);
                        render->AddStyle([key UTF8String], buffer);
                        break;
                    case _C_ULNG:
                    case _C_ULNG_LNG:
                        snprintf(buffer, sizeof(buffer), "%llu", [num unsignedLongLongValue]);
                        render->AddStyle([key UTF8String], buffer);
                        break;
                    case _C_BFLD:
                    case _C_BOOL:
                        render->AddStyle([key UTF8String], [num boolValue] ? "true" : "false");
                        break;
                    default:
                        render->AddStyle([key UTF8String], [[num stringValue] UTF8String]);
                        break;
                }
            }
            else {
                render->AddStyle([key UTF8String], [[WXUtility JSONString:obj] UTF8String]);
            }
        }];
        
        for (id obj in data[@"event"]) {
            if ([obj isKindOfClass:[NSString class]]) {
                render->AddEvent([obj UTF8String]);
            }
            else {
                render->AddEvent([[WXUtility JSONString:obj] UTF8String]);
            }
        }
        
        int childIndex = 0;
        for (NSDictionary* obj in data[@"children"]) {
            _parseRenderObject(obj, render, childIndex ++, pageId);
        }
        
        render->ApplyDefaultStyle();
        render->ApplyDefaultAttr();
        
        return render;
    }
    return nullptr;
}

+ (void)callAddElement:(NSString*)pageId parentRef:(NSString*)parentRef data:(NSDictionary*)data index:(int)index
{
#if 0
    char indexBuffer[25];
    sprintf(indexBuffer, "%d", index);
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallAddElement([pageId UTF8String], [parentRef UTF8String], [[WsonObject fromObject:data] data], indexBuffer);
#else
    using namespace WeexCore;
    const std::string page([pageId UTF8String]);
    RenderObject* child = _parseRenderObject(data, nullptr, 0, page);
    RenderManager::GetInstance()->AddRenderObject(page, [parentRef UTF8String], index, child);
#endif
}

+ (void)callCreateBody:(NSString*)pageId data:(NSDictionary*)data
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallCreateBody([pageId UTF8String], [[WsonObject fromObject:data] data]);
}

+ (void)callRemoveElement:(NSString*)pageId ref:(NSString*)ref
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallRemoveElement([pageId UTF8String], [ref UTF8String]);
}

+ (void)callMoveElement:(NSString*)pageId ref:(NSString*)ref parentRef:(NSString*)parentRef index:(int)index
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallMoveElement([pageId UTF8String], [ref UTF8String], [parentRef UTF8String], index);
}

+ (void)callUpdateAttrs:(NSString*)pageId ref:(NSString*)ref data:(NSDictionary*)data
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateAttrs([pageId UTF8String], [ref UTF8String], [[WsonObject fromObject:data] data]);
}

+ (void)callUpdateStyle:(NSString*)pageId ref:(NSString*)ref data:(NSDictionary*)data
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateStyle([pageId UTF8String], [ref UTF8String], [[WsonObject fromObject:data] data]);
}

+ (void)callAddEvent:(NSString*)pageId ref:(NSString*)ref event:(NSString*)event
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallAddEvent([pageId UTF8String], [ref UTF8String], [event UTF8String]);
}

+ (void)callRemoveEvent:(NSString*)pageId ref:(NSString*)ref event:(NSString*)event
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallRemoveEvent([pageId UTF8String], [ref UTF8String], [event UTF8String]);
}

+ (void)callCreateFinish:(NSString*)pageId
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallCreateFinish([pageId UTF8String]);
}

+ (void)callRefreshFinish:(NSString*)pageId
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallRefreshFinish([pageId UTF8String], nullptr, nullptr);
}

+ (void)callUpdateFinish:(NSString*)pageId
{
    WeexCore::WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateFinish([pageId UTF8String], nullptr, nullptr);
}

@end
