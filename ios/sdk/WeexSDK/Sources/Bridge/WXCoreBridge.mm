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

#ifdef WX_IMPORT_WEEXCORE

#import "WXCoreBridge.h"
#import "JSValue+Weex.h"
#import "WXSDKManager.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"
#import "WXBridgeManager_private.h"
#import "WXBridgeContext_private.h"
#import "WXLog.h"
#import "WXTracingManager.h"
#import "WXBridgeProtocol.h"
#import "WXUtility.h"
#import "WXAppConfiguration.h"
#import "WXLayoutConstraint.h"

#include <base/CoreConstants.h>
#include <core/manager/weex_core_manager.h>
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <core/render/node/render_object.h>
#include <core/config/core_environment.h>
#include <base/TimeUtils.h>

#define NSSTRING(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCString(NULL, (const char *)(cstr), kCFStringEncodingUTF8)))
#define NSSTRING_NO_COPY(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCStringNoCopy(NULL, (const char *)(cstr), kCFStringEncodingUTF8, kCFAllocatorNull)))

namespace WeexCore
{
    static NSMutableDictionary* NSDICTIONARY(std::map<std::string, std::string>* map)
    {
        if (map == nullptr)
            return @{}.mutableCopy;
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:map->size()];
        for (auto it = map->begin(); it != map->end(); it ++) {
            [result setObject:NSSTRING(it->second.c_str()) forKey:NSSTRING(it->first.c_str())];
        }
        return result;
    }
    
    static NSMutableDictionary* NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* vec)
    {
        if (vec == nullptr)
            return @{}.mutableCopy;
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:vec->size()];
        for (auto& p : *vec) {
            [result setObject:NSSTRING(p.second.c_str()) forKey:NSSTRING(p.first.c_str())];
        }
        return result;
    }
    
    static NSMutableArray* NSARRAY(std::set<std::string>* set)
    {
        if (set == nullptr)
            return @[].mutableCopy;
        
        NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:set->size()];
        for (auto& s : *set) {
            [result addObject:NSSTRING(s.c_str())];
        }
        return result;
    }
    
    static void MergeBorderWidthValues(NSMutableDictionary* dict, const WXCoreBorderWidth & borders, bool isUpdate)
    {
        if (borders.getBorderWidth(kBorderWidthTop) != (float)0.0f || isUpdate) {
            dict[@"borderTopWidth"] = [NSString stringWithFormat:@"%f", borders.getBorderWidth(kBorderWidthTop)];
        }
        if (borders.getBorderWidth(kBorderWidthLeft) != (float)0.0f || isUpdate) {
            dict[@"borderLeftWidth"] = [NSString stringWithFormat:@"%f", borders.getBorderWidth(kBorderWidthLeft)];
        }
        if (borders.getBorderWidth(kBorderWidthBottom) != (float)0.0f || isUpdate) {
            dict[@"borderBottomWidth"] = [NSString stringWithFormat:@"%f", borders.getBorderWidth(kBorderWidthBottom)];
        }
        if (borders.getBorderWidth(kBorderWidthRight) != (float)0.0f || isUpdate) {
            dict[@"borderRightWidth"] = [NSString stringWithFormat:@"%f", borders.getBorderWidth(kBorderWidthRight)];
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
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
#warning todo task type
        //NSString* ns_task = NSSTRING(task);
        NSString* ns_callback = NSSTRING(callback);
        
        [[WXBridgeManager sharedManager].bridgeCtx invokeNative:ns_instanceId tasks:@[] callback:ns_callback];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
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
        
//        if (str_array != nullptr) {
//            WXLogDebug(@"jsLog: %s", str_array);
//        }
    }
        
    int WXCoreBridge::callUpdateFinish(const char* pageId, const char *task, const char *callback)
    {
        // should not enter this function
        assert(false);
        
//        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
//        if (page == nullptr) {
//            return -1;
//        }
//
//        long long startTime = getCurrentTime();
//
//        NSString* ns_instanceId = NSSTRING(pageId);
//        WXSDKInstance* instance = [WXSDKManager instanceForID:ns_instanceId];
//
//        if (instance) {
//            WXPerformBlockOnComponentThread(^{
//                WXComponentManager* manager = instance.componentManager;
//                if (!manager.isValid) {
//                    return;
//                }
//                [manager startComponentTasks];
//                [manager updateFinish];
//            });
//
//            NSString* ns_callback = NSSTRING(callback);
//            if (ns_callback.length > 0) {
//                JSContext* context = instance.instanceJavaScriptContext.javaScriptContext;
//                JSValue* callbackFunction = context[ns_callback];
//                if (callbackFunction) {
//                    [callbackFunction callWithArguments:@[]];
//                }
//            }
//        }
//
//        page->CallBridgeTime(getCurrentTime() - startTime);
//        return 0;
    }
        
    int WXCoreBridge::callRefreshFinish(const char* pageId, const char *task, const char *callback)
    {
        // should not enter this function
        assert(false);
        
//        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
//        if (page == nullptr) {
//            return -1;
//        }
//
//        long long startTime = getCurrentTime();
//
//        NSString* ns_instanceId = NSSTRING(pageId);
//        WXSDKInstance* instance = [WXSDKManager instanceForID:ns_instanceId];
//
//        if (instance) {
//            WXPerformBlockOnComponentThread(^{
//                WXComponentManager* manager = instance.componentManager;
//                if (!manager.isValid) {
//                    return;
//                }
//                [manager startComponentTasks];
//                [manager refreshFinish];
//            });
//
//            NSString* ns_callback = NSSTRING(callback);
//            if (ns_callback.length > 0) {
//                JSContext* context = instance.instanceJavaScriptContext.javaScriptContext;
//                JSValue* callbackFunction = context[ns_callback];
//                if (callbackFunction) {
//                    [callbackFunction callWithArguments:@[]];
//                }
//            }
//        }
//
//        page->CallBridgeTime(getCurrentTime() - startTime);
//        return 0;
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
        [manager wxcore_CreateBody:@{@"ref": ns_ref, @"type": ns_type,
                                     @"style": ns_styles, @"attr": ns_attributes, @"event": ns_events}
                      renderObject:renderObject];
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
        [manager wxcore_AddElement:@{@"ref": ns_ref, @"type": ns_componentType,
                                     @"style": ns_styles, @"attr": ns_attributes, @"event": ns_events}
                           toSuper:ns_parentRef
                           atIndex:ns_index
                      renderObject:renderObject];
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
        [manager wxcore_Layout:component frame:frame];

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
        [manager wxcore_CreateFinish];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createFinish" options:@{@"threadName":WXTDOMThread}];

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
    WeexCore::WXCoreEnvironment::getInstance()->SetPlatform(OS_iOS);
    
    CGSize screenSize = [UIScreen mainScreen].bounds.size;
    WeexCore::WXCoreEnvironment::getInstance()->SetDeviceWidth(std::to_string(screenSize.width));
    WeexCore::WXCoreEnvironment::getInstance()->SetDeviceHeight(std::to_string(screenSize.height));
    
    platformBridge = new WeexCore::WXCoreBridge();
    WeexCore::WeexCoreManager::getInstance()->setPlatformBridge(platformBridge);
    
    jsBridge = new WeexCore::JSBridge();
    WeexCore::WeexCoreManager::getInstance()->setJSBridge(jsBridge);
    
    WeexCore::WeexCoreManager::getInstance()->SetMeasureFunctionAdapter(new WeexCore::WXCoreMeasureFunctionBridge());
}

+ (void)setDefaultDimensionIntoRoot:(NSString*)instanceId width:(CGFloat)width height:(CGFloat)height
                 isWidthWrapContent:(BOOL)isWidthWrapContent
                isHeightWrapContent:(BOOL)isHeightWrapContent
{
    if (platformBridge) {
        platformBridge->setDefaultHeightAndWidthIntoRootDom([instanceId UTF8String], (float)width, (float)height, (bool)isWidthWrapContent, (bool)isHeightWrapContent);
    }
}

+ (void)setViewportWidth:(NSString*)instanceId width:(CGFloat)width
{
    if (platformBridge) {
        platformBridge->setViewportWidth([instanceId UTF8String], (float)width);
    }
}

+ (void)triggerLayout:(NSString*)instanceId size:(CGSize)size
{
    const char* page = [instanceId UTF8String];
    if (platformBridge->notifyLayout(page)) {
        platformBridge->setDefaultHeightAndWidthIntoRootDom(page, size.width, size.height, false, false);
        platformBridge->forceLayout(page);
    }
}

@end

#endif
