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
#import "WXAssert.h"
#import "WXAppConfiguration.h"
#import "WXSDKEngine.h"
#import "WXAppMonitorProtocol.h"

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
#include "core/bridge/platform/core_side_in_platform.h"
#include "core/bridge/script/core_side_in_script.h"
#include "base/TimeUtils.h"

#import <objc/runtime.h>
#include <fstream>


#define NSSTRING(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCString(NULL, (const char *)(cstr), kCFStringEncodingUTF8)))
#define NSSTRING_NO_COPY(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCStringNoCopy(NULL, (const char *)(cstr), kCFStringEncodingUTF8, kCFAllocatorNull)))

namespace WeexCore
{
    static NSString* const JSONSTRING_SUFFIX = @"\t\n\t\r";
    
    static NSString* TO_JSON(id object)
    {
        if (object == nil) {
            return nil;
        }
        
        @try {
            if ([object isKindOfClass:[NSArray class]] || [object isKindOfClass:[NSDictionary class]]) {
                NSError *error = nil;
                NSData *data = [NSJSONSerialization dataWithJSONObject:object
                                                               options:0
                                                                 error:&error];
                
                if (error) {
                    WXLogError(@"%@", error);
                    WXAssert(NO, @"Fail to convert object to json. %@", error);
                    return nil;
                }
                
                return [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] stringByAppendingString:JSONSTRING_SUFFIX]; // add suffix so that we know this is a json string
            }
        } @catch (NSException *exception) {
            WXLogError(@"%@", exception);
            WXAssert(NO, @"Fail to convert object to json. %@", exception);
            return nil;
        }
        
        return nil;
    }
    
    static id TO_OBJECT(NSString* s)
    {
        if ([s hasSuffix:JSONSTRING_SUFFIX]) {
            if ([s length] == [JSONSTRING_SUFFIX length]) {
                return [NSNull null];
            }
            
            // s is a json string
            @try {
                NSError* error = nil;
                id jsonObj = [NSJSONSerialization JSONObjectWithData:[s dataUsingEncoding:NSUTF8StringEncoding]
                                                             options:NSJSONReadingMutableContainers | NSJSONReadingMutableLeaves
                                                               error:&error];
                
                if (jsonObj == nil) {
                    WXLogError(@"%@", error);
                    WXAssert(NO, @"Fail to convert json to object. %@", error);
                }
                else {
                    return jsonObj;
                }
            } @catch (NSException *exception) {
                WXLogError(@"%@", exception);
                WXAssert(NO, @"Fail to convert json to object. %@", exception);
            }
        }
        return s; // return s instead
    }
    
    static NSMutableDictionary* NSDICTIONARY(std::map<std::string, std::string>* map)
    {
        if (map == nullptr || map->size() == 0)
            return [[NSMutableDictionary alloc] init];
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:map->size()];
        for (auto it = map->begin(); it != map->end(); it ++) {
            id object = TO_OBJECT(NSSTRING(it->second.c_str()));
            if (object) {
                [result setObject:object forKey:NSSTRING(it->first.c_str())];
            }
        }
        return result;
    }
    
    static NSMutableDictionary* NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* vec)
    {
        if (vec == nullptr || vec->size() == 0)
            return [[NSMutableDictionary alloc] init];
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:vec->size()];
        for (auto& p : *vec) {
            id object = TO_OBJECT(NSSTRING(p.second.c_str()));
            if (object) {
                [result setObject:object forKey:NSSTRING(p.first.c_str())];
            }
        }
        return result;
    }
    
    static NSMutableArray* NSARRAY(std::set<std::string>* set)
    {
        if (set == nullptr || set->size() == 0)
            return [[NSMutableArray alloc] init];
        
        NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:set->size()];
        for (auto& s : *set) {
            id object = TO_OBJECT(NSSTRING(s.c_str()));
            if (object) {
                [result addObject:object];
            }
        }
        return result;
    }
    
    static void consoleWithArguments(NSArray *arguments, WXLogFlag logLevel)
    {
        NSMutableString *jsLog = [NSMutableString string];
        [jsLog appendString:@"jsLog: "];
        [arguments enumerateObjectsUsingBlock:^(NSString *jsVal, NSUInteger idx, BOOL *stop) {
            if (idx == arguments.count - 1) {
                if (logLevel) {
                    if (WXLogFlagWarning == logLevel || WXLogFlagError == logLevel) {
                        id<WXAppMonitorProtocol> appMonitorHandler = [WXSDKEngine handlerForProtocol:@protocol(WXAppMonitorProtocol)];
                        if ([appMonitorHandler respondsToSelector:@selector(commitAppMonitorAlarm:monitorPoint:success:errorCode:errorMsg:arg:)]) {
                            [appMonitorHandler commitAppMonitorAlarm:@"weex" monitorPoint:@"jswarning" success:NO errorCode:@"99999" errorMsg:jsLog arg:[WXSDKEngine topInstance].pageName];
                        }
                    }
                    WX_LOG(logLevel, @"%@", jsLog);
                }
                else {
                    [jsLog appendFormat:@"%@ ", jsVal];
                    WXLogInfo(@"%@", jsLog);
                }
            }
            else {
                [jsLog appendFormat:@"%@ ", jsVal];
            }
        }];
    }
    
    static void MergeBorderWidthValues(NSMutableDictionary* dict,
                                       const WXCoreBorderWidth & borders,
                                       bool isUpdate, float pixelScaleFactor)
    {
        if (pixelScaleFactor <= 0) {
            pixelScaleFactor = 1.0f;
        }
        if (borders.getBorderWidth(kBorderWidthTop) != (float)0.0f || isUpdate) {
            dict[@"borderTopWidth"] = @(borders.getBorderWidth(kBorderWidthTop) / pixelScaleFactor);
        }
        if (borders.getBorderWidth(kBorderWidthLeft) != (float)0.0f || isUpdate) {
            dict[@"borderLeftWidth"] = @(borders.getBorderWidth(kBorderWidthLeft) / pixelScaleFactor);
        }
        if (borders.getBorderWidth(kBorderWidthBottom) != (float)0.0f || isUpdate) {
            dict[@"borderBottomWidth"] = @(borders.getBorderWidth(kBorderWidthBottom) / pixelScaleFactor);
        }
        if (borders.getBorderWidth(kBorderWidthRight) != (float)0.0f || isUpdate) {
            dict[@"borderRightWidth"] = @(borders.getBorderWidth(kBorderWidthRight) / pixelScaleFactor);
        }
    }
    
    static void MergeBorderWidthValues(NSMutableDictionary* dict,
                                       std::vector<std::pair<std::string, std::string>>* borders,
                                       float pixelScaleFactor)
    {
        if (borders == nullptr) {
            return;
        }
        if (pixelScaleFactor <= 0) {
            pixelScaleFactor = 1.0f;
        }
        
        for (auto& p : *borders) {
            dict[NSSTRING(p.first.c_str())] = @(atof(p.second.c_str()) / pixelScaleFactor);
        }
    }
    
    void IOSSide::SetJSVersion(const char* version)
    {
        NSString *jsVersion = NSSTRING(version);
        if (jsVersion.length > 0) {
            [WXAppConfiguration setJSFrameworkVersion:jsVersion];
        }
    }
    
    void IOSSide::ReportException(const char* pageId, const char *func, const char *exception_string)
    {
        NSString* ns_instanceId = NSSTRING(pageId);

        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return;
        }

        int wxErrorCode = 9999;
        NSError * error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:wxErrorCode userInfo:@{@"message":[NSString stringWithUTF8String:exception_string]}];
        [manager renderFailed:error];
    }
    
    int IOSSide::CallNative(const char* pageId, const char *task, const char *callback)
    {
        // should not enter this function
        assert(false);
    }
    
    std::unique_ptr<ValueWithType> IOSSide::CallNativeModule(const char *page_id, const char *module, const char *method, const char *args, int args_length, const char *options, int options_length)
    {
        // should not enter this function
        do {
            NSString *instanceId = NSSTRING(page_id);
            WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
            if (!instance) {
                break;
            }
            NSString *moduleName = [NSString stringWithUTF8String:module];
            NSString *methodName = [NSString stringWithUTF8String:method];
            NSArray *newArguments;
            if (args && args_length > 0) {
                NSString *arguments = [NSString stringWithUTF8String:args];
                newArguments = [WXUtility objectFromJSON:arguments];
            }
            LOGD("CallNativeModule:[%s]:[%s]=>%s \n", module, method, args);
            WXModuleMethod *method = [[WXModuleMethod alloc] initWithModuleName:moduleName methodName:methodName arguments:newArguments options:nil instance:instance];
            [method invoke];
            
        } while (0);
        
        return std::unique_ptr<ValueWithType>();
    }
        
    void IOSSide::CallNativeComponent(const char* pageId, const char* ref, const char *method,
                                           const char *arguments, int argumentsLength,
                                           const char *options, int optionsLength)
    {
        // should not enter this function
        assert(false);
    }
        
    void IOSSide::SetTimeout(const char* callbackID, const char* time)
    {
        // should not enter this function
        assert(false);
    }

    void IOSSide::NativeLog(const char *args)
    {
        // should not enter this function
        do {
            if (!args) {
                break;
            }
            NSArray *newArguments;
            if (args) {
                NSString *arguments = [NSString stringWithUTF8String:args];
                newArguments = [WXUtility objectFromJSON:arguments];
            }
            if (![newArguments isKindOfClass:[NSArray class]] || !newArguments.count) {
                break;
            }
            static NSDictionary *levelMap;
            static dispatch_once_t onceToken;
            dispatch_once(&onceToken, ^{
                levelMap = @{@"__ERROR": @(WXLogFlagError),
                          @"__WARN": @(WXLogFlagWarning),
                          @"__INFO": @(WXLogFlagInfo),
                          @"__DEBUG": @(WXLogFlagDebug),
                          @"__LOG": @(WXLogFlagLog)};
            });
            NSString *levelStr = [newArguments lastObject];
            consoleWithArguments(newArguments, (WXLogFlag)[levelMap[levelStr] integerValue]);
            
        } while (0);
    }
    
    void IOSSide::TriggerVSync(const char* page_id)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(page_id);
        if (page == nullptr) {
            return;
        }
        
        NSString* ns_instanceId = NSSTRING(page_id);
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return;
        }
        [manager startComponentTasks];
    }
        
    int IOSSide::UpdateFinish(const char* page_id, const char* task, int taskLen,
                                   const char* callback, int callbackLen)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(page_id);
        if (page == nullptr) {
            return -1;
        }
        
        NSString* ns_instanceId = NSSTRING(page_id);
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager updateFinish];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateFinish" options:@{@"threadName":WXTDOMThread}];
        
        return 0;
    }
        
    int IOSSide::RefreshFinish(const char* pageId, const char *task, const char *callback)
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
        
    int IOSSide::AddEvent(const char* pageId, const char* ref, const char *event)
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
        [manager addEvent:ns_event toComponent:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"addEvent" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::RemoveEvent(const char* pageId, const char* ref, const char *event)
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
        [manager removeEvent:ns_event fromComponent:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"removeEvent" options:@{@"threadName":WXTDOMThread}];
    
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::CreateBody(const char* pageId, const char *componentType, const char* ref,
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
        
        WXSDKInstance* sdkInstance = [WXSDKManager instanceForID:ns_instanceId];
        MergeBorderWidthValues(ns_styles, borders, false, sdkInstance.pixelScaleFactor);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: createBody %@ ref:%@", ns_type, ns_ref);
#endif
        
        WXComponentManager* manager = sdkInstance.componentManager;
        if (!manager.isValid) {
            return -1;
        }
        [manager startComponentTasks];
        [manager createBody:ns_ref type:ns_type styles:ns_styles attributes:ns_attributes events:ns_events renderObject:renderObject];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::AddElement(const char* pageId, const char *componentType, const char* ref,
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
        
        WXSDKInstance* sdkInstance = [WXSDKManager instanceForID:ns_instanceId];
        MergeBorderWidthValues(ns_styles, borders, false, sdkInstance.pixelScaleFactor);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: addElement : %@", ns_componentType);
#endif
        
        WXComponentManager* manager = sdkInstance.componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        [manager startComponentTasks];
        [manager addComponent:ns_ref type:ns_componentType parentRef:ns_parentRef styles:ns_styles attributes:ns_attributes events:ns_events index:ns_index renderObject:renderObject];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"addElement" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int IOSSide::Layout(const char* pageId, const char* ref,
                       float top, float bottom, float left, float right,
                       float height, float width, bool isRTL, int index)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        RenderObject* renderObject = page->GetRenderObject(ref);
        if (renderObject->getContext() == nullptr) {
            return -1;
        }
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
        [manager layoutComponent:component frame:frame isRTL:isRTL innerMainSize:renderObject->getLargestMainSize()];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    void IOSSide::InvokeLayoutPlatform(const char* page_id, long render_ptr)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(page_id);
        if (page == nullptr) {
            return;
        }
        
        long long startTime = getCurrentTime();
        
        RenderObject* renderObject = reinterpret_cast<RenderObject*>(render_ptr);
        if (renderObject->getContext() == nullptr) {
            return;
        }
        WXComponent* component = (__bridge WXComponent *)(renderObject->getContext());
        NSString* ns_instanceId = NSSTRING(page_id);
        
        WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
        if (!manager.isValid) {
            return;
        }
        
        [manager layoutComponent:component];

        page->CallBridgeTime(getCurrentTime() - startTime);
    }
    
    int IOSSide::UpdateStyle(const char* pageId, const char* ref,
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
        
        WXSDKInstance* sdkInstance = [WXSDKManager instanceForID:ns_instanceId];
        MergeBorderWidthValues(ns_style, border, sdkInstance.pixelScaleFactor);
        
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: updateStyles ref:%@, styles:%@", ns_ref, ns_style);
#endif
        
        WXComponentManager* manager = sdkInstance.componentManager;
        if (!manager.isValid) {
            return -1;
        }
        
        [manager startComponentTasks];
        [manager updateStyles:ns_style forComponent:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateStyles" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::UpdateAttr(const char* pageId, const char* ref,
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
        [manager updateAttributes:ns_attributes forComponent:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTDOMThread}];

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::CreateFinish(const char* pageId)
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
    
    int IOSSide::RenderSuccess(const char* pageId)
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
        
    int IOSSide::RemoveElement(const char* pageId, const char* ref)
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
        [manager removeComponent:ns_ref];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"removeElement" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::MoveElement(const char* pageId, const char* ref, const char* parentRef, int index)
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
        [manager moveComponent:ns_ref toSuper:ns_parentRef atIndex:ns_index];
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"moveElement" options:@{@"threadName":WXTDOMThread}];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int IOSSide::AppendTreeCreateFinish(const char* pageId, const char* ref)
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
        [manager appendTreeCreateFinish:ns_ref];
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int IOSSide::HasTransitionPros(const char* pageId, const char* ref,
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
        if ([manager isTransitionNoneOfComponent:ns_ref]) {
            return 0;
        }
        
        int result = [manager hasTransitionPropertyInStyles:NSDICTIONARY(style) forComponent:ns_ref] ? 1 : 0;
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return result;
    }
    
#pragma mark - Layout Impl
    
    WXCoreSize WXCoreMeasureFunctionBridge::Measure(const char* page_id, long render_ptr, float width, MeasureMode widthMeasureMode, float height, MeasureMode heightMeasureMode)
    {
        // should not enter this function
        assert(false);
    }
    
    void WXCoreMeasureFunctionBridge::LayoutBefore(const char* page_id, long render_ptr)
    {
        
    }
    
    void WXCoreMeasureFunctionBridge::LayoutAfter(const char* page_id, long render_ptr, float width, float height)
    {
        
    }
}

@implementation WXCoreBridge

static WeexCore::PlatformBridge* platformBridge = nullptr;
static WeexCore::ScriptBridge* jsBridge = nullptr;

+ (void)install
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        WeexCore::WXCoreEnvironment* env = WeexCore::WXCoreEnvironment::getInstance();
        env->SetPlatform(OS_iOS);
        env->AddOption("scale", "1");
        
        CGSize screenSize = [UIScreen mainScreen].bounds.size;
        env->SetDeviceWidth(std::to_string(screenSize.width));
        env->SetDeviceHeight(std::to_string(screenSize.height));
        env->AddOption("screen_width_pixels", std::to_string(screenSize.width));
        env->AddOption("screen_height_pixels", std::to_string(screenSize.height));
        env->AddOption("status_bar_height", std::to_string([[UIApplication sharedApplication] statusBarFrame].size.height));
        
        platformBridge = new WeexCore::PlatformBridge();
        platformBridge->set_platform_side(new WeexCore::IOSSide());
        platformBridge->set_core_side(new WeexCore::CoreSideInPlatform());
        WeexCore::WeexCoreManager::Instance()->set_platform_bridge(platformBridge);
        
        jsBridge = new WeexCore::ScriptBridge();
        jsBridge->set_core_side(new WeexCore::CoreSideInScript());
        WeexCore::WeexCoreManager::Instance()->set_script_bridge(jsBridge);
        
        WeexCore::WeexCoreManager::Instance()->set_measure_function_adapter(new WeexCore::WXCoreMeasureFunctionBridge());
    });
}

+ (void)createDataRenderInstance:(NSString *)pageId template:(NSString *)jsBundleString options:(NSDictionary *)options  data:(id)data
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    NSString *optionsString = [WXUtility JSONString:options];
    NSString *dataString = [WXUtility JSONString:data];
    node_manager->CreatePage([jsBundleString UTF8String] ?: "", [pageId UTF8String] ?: "", [optionsString UTF8String] ?: "", [dataString UTF8String] ?: "");
}

+ (void)createDataRenderInstance:(NSString *)pageId contents:(NSData *)contents options:(NSDictionary *)options data:(id)data
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    NSString *optionsString = [WXUtility JSONString:options];
    NSString *dataString = [WXUtility JSONString:data];
    node_manager->CreatePage(static_cast<const char *>(contents.bytes), contents.length, [pageId UTF8String], [optionsString UTF8String], dataString ? [dataString UTF8String] : "");
}

+ (void)destroyDataRenderInstance:(NSString *)pageId
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    node_manager->ClosePage([pageId UTF8String] ?: "");
}

+ (void)refreshDataRenderInstance:(NSString *)pageId data:(NSString *)data;
{
    auto node_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    node_manager->RefreshPage([pageId UTF8String] ?: "", [data UTF8String] ?: "");
}

+ (void)fireEvent:(NSString *)pageId ref:(NSString *)ref event:(NSString *)event args:(NSDictionary *)args
{
    NSString *params = [WXUtility JSONString:args];
    auto vnode_manager = weex::core::data_render::VNodeRenderManager::GetInstance();
    vnode_manager->FireEvent([pageId UTF8String] ? : "", [ref UTF8String] ? : "", [event UTF8String] ? : "", [params UTF8String] ? : "");
}

+ (void)registerModules:(NSDictionary *)modules {
    NSString *setting = [WXUtility JSONString:modules];
    if (setting.length > 0) {
        weex::core::data_render::VNodeRenderManager::GetInstance()->RegisterModules([setting UTF8String] ? : "");
    }
}

+ (void)registerComponentAffineType:(NSString *)type asType:(NSString *)baseType
{
    WeexCore::RenderCreator::GetInstance()->RegisterAffineType([type UTF8String] ?: "", [baseType UTF8String] ?: "");
}

+ (BOOL)isComponentAffineType:(NSString *)type asType:(NSString *)baseType
{
    return WeexCore::RenderCreator::GetInstance()->IsAffineType([type UTF8String] ?: "", [baseType UTF8String] ?: "");
}

+ (void)setDefaultDimensionIntoRoot:(NSString*)pageId width:(CGFloat)width height:(CGFloat)height
                 isWidthWrapContent:(BOOL)isWidthWrapContent
                isHeightWrapContent:(BOOL)isHeightWrapContent
{
    if (platformBridge) {
        if (width == 0 && !isWidthWrapContent) {
            return;
        }
        
        if (height == 0 && !isHeightWrapContent) {
            return;
        }
        
        platformBridge->core_side()->SetDefaultHeightAndWidthIntoRootDom([pageId UTF8String] ?: "", (float)width, (float)height, (bool)isWidthWrapContent, (bool)isHeightWrapContent);
    }
}

+ (void)setViewportWidth:(NSString*)pageId width:(CGFloat)width
{
    if (platformBridge) {
        platformBridge->core_side()->SetViewPortWidth([pageId UTF8String] ?: "", (float)width);
    }
}

+ (void)layoutPage:(NSString*)pageId forced:(BOOL)forced
{
    if (platformBridge) {
        const char* page = [pageId UTF8String] ?: "";
        if (forced) {
            platformBridge->core_side()->SetPageDirty(page);
        }
        
        if (platformBridge->core_side()->NotifyLayout(page)) {
            platformBridge->core_side()->ForceLayout(page);
        }
    }
}

+ (void)closePage:(NSString*)pageId
{
    if (platformBridge) {
        platformBridge->core_side()->OnInstanceClose([pageId UTF8String] ?: "");
    }
}

+ (void)_traverseTree:(WeexCore::RenderObject *)render index:(int)index pageId:(const char *)pageId
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
            BOOL isRTL = render->getLayoutDirectionFromPathNode() == WeexCore::kDirectionRTL;
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            CGRect frame = CGRectMake(isnan(WXRoundPixelValue(left))?0:WXRoundPixelValue(left),
                                      isnan(WXRoundPixelValue(top))?0:WXRoundPixelValue(top),
                                      isnan(WXRoundPixelValue(width))?0:WXRoundPixelValue(width),
                                      isnan(WXRoundPixelValue(height))?0:WXRoundPixelValue(height));
            [manager layoutComponent:component frame:frame isRTL:isRTL innerMainSize:render->getLargestMainSize()];
        }
        render->setHasNewLayout(false);
    }

    for (auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it ++) {
        WeexCore::RenderObject *child = static_cast<WeexCore::RenderObject *>(*it);
        if (child != nullptr) {
            [self _traverseTree:child index:(int)(it - render->ChildListIterBegin()) pageId:pageId];
        }
    }
}

+ (void)layoutRenderObject:(void*)object size:(CGSize)size page:(NSString*)pageId
{
    using namespace WeexCore;
    RenderObject* render = static_cast<RenderObject*>(object);
    std::pair<float, float> renderPageSize(size.width, size.height);
    
    render->calculateLayout(renderPageSize);
    [self _traverseTree:render index:0 pageId:[pageId UTF8String] ?: ""];
}

+ (void*)copyRenderObject:(void*)source replacedRef:(NSString*)ref
{
    using namespace WeexCore;
    RenderObject* sourceObject = static_cast<RenderObject*>(source);
    RenderObject* copyObject = static_cast<RenderObject*>(RenderCreator::GetInstance()->CreateRender(sourceObject->type(), sourceObject->ref()));
                                                          
    copyObject->CopyFrom(sourceObject);
    if (ref != nil) {
        copyObject->set_ref([ref UTF8String] ?: "");
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

+ (void)removeRenderObjectFromMap:(NSString*)pageId object:(void*)object
{
    using namespace WeexCore;
    RenderPage* page = RenderManager::GetInstance()->GetPage([pageId UTF8String] ?: "");
    if (page != nullptr) {
        page->RemoveRenderFromRegisterMap(static_cast<RenderObject*>(object));
    }
}

static void _convertToCString(id _Nonnull obj, void (^callback)(const char*))
{
    if ([obj isKindOfClass:[NSString class]]) {
        callback([obj UTF8String]);
    }
    else if ([obj isKindOfClass:[NSNumber class]]) {
        callback([[(NSNumber*)obj stringValue] UTF8String]);
    }
    else if ([obj isKindOfClass:[NSNull class]]) {
        callback([JSONSTRING_SUFFIX UTF8String]);
    }
    else {
        NSString* jsonstring = WeexCore::TO_JSON(obj);
        if (jsonstring != nil) {
            callback([jsonstring UTF8String]);
        }
    }
}

+ (void)_parseStyleBeforehand:(NSDictionary *)styles key:(NSString *)key render:(WeexCore::RenderObject*)render
{
    id data = styles[key];
    if (data) {
        _convertToCString(data, ^(const char * value) {
            if (value != nullptr) {
                render->AddStyle([key UTF8String], value);
            }
        });
    }
}

+ (WeexCore::RenderObject*)_parseRenderObject:(NSDictionary *)data parent:(WeexCore::RenderObject *)parent index:(int)index pageId:(const std::string&)pageId
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
            _convertToCString(obj, ^(const char * value) {
                if (value != nullptr) {
                    render->AddAttr([key UTF8String], value);
                }
            });
        }];
        
        // margin/padding/borderWidth should be handled beforehand. Because maringLeft should override margin.
        NSDictionary* styles = data[@"style"];
        [self _parseStyleBeforehand:styles key:@"margin" render:render];
        [self _parseStyleBeforehand:styles key:@"padding" render:render];
        [self _parseStyleBeforehand:styles key:@"borderWidth" render:render];
        [styles enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
            if ([key isEqualToString:@"margin"] || [key isEqualToString:@"padding"] || [key isEqualToString:@"borderWidth"]) {
                return;
            }
            _convertToCString(obj, ^(const char * value) {
                if (value != nullptr) {
                    render->AddStyle([key UTF8String], value);
                }
            });
        }];
        
        for (id obj in data[@"event"]) {
            _convertToCString(obj, ^(const char * value) {
                if (value != nullptr) {
                    render->AddEvent(value);
                }
            });
        }
        
        int childIndex = 0;
        for (NSDictionary* obj in data[@"children"]) {
            [self _parseRenderObject:obj parent:render index:childIndex ++ pageId:pageId];
        }
        
        render->ApplyDefaultStyle();
        render->ApplyDefaultAttr();
        
        return render;
    }
    return nullptr;
}

+ (std::vector<std::pair<std::string, std::string>>*)_parseMapValuePairs:(NSDictionary *)data
{
    std::vector<std::pair<std::string, std::string>>* result = new std::vector<std::pair<std::string, std::string>>();
    [data enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        _convertToCString(obj, ^(const char * value) {
            if (value != nullptr) {
                result->emplace_back([key UTF8String], value);
            }
        });
    }];
    return result;
}

+ (void)callAddElement:(NSString*)pageId parentRef:(NSString*)parentRef data:(NSDictionary*)data index:(int)index
{
    using namespace WeexCore;
    const std::string page([pageId UTF8String] ?: "");
    RenderObject* child = [self _parseRenderObject:data parent:nullptr index:0 pageId:page];
    RenderManager::GetInstance()->AddRenderObject(page, [parentRef UTF8String] ?: "", index, child);
}

+ (void)callCreateBody:(NSString*)pageId data:(NSDictionary*)data
{
    using namespace WeexCore;
    const std::string page([pageId UTF8String] ?: "");
    RenderManager::GetInstance()->CreatePage(page, [&] (RenderPage* pageInstance) -> RenderObject* {
        pageInstance->set_before_layout_needed(false); // we do not need before and after layout
        pageInstance->set_after_layout_needed(false);
        pageInstance->set_platform_layout_needed(true);
        return [self _parseRenderObject:data parent:nullptr index:0 pageId:page];
    });
}

+ (void)callRemoveElement:(NSString*)pageId ref:(NSString*)ref
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveElement([pageId UTF8String] ?: "", [ref UTF8String] ?: "");
}

+ (void)callMoveElement:(NSString*)pageId ref:(NSString*)ref parentRef:(NSString*)parentRef index:(int)index
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->MoveElement([pageId UTF8String] ?: "", [ref UTF8String] ?: "", [parentRef UTF8String] ?: "", index);
}

+ (void)callUpdateAttrs:(NSString*)pageId ref:(NSString*)ref data:(NSDictionary*)data
{
    WeexCore::RenderManager::GetInstance()->UpdateAttr([pageId UTF8String] ?: "", [ref UTF8String] ?: "", [self _parseMapValuePairs:data]);
}

+ (void)callUpdateStyle:(NSString*)pageId ref:(NSString*)ref data:(NSDictionary*)data
{
    WeexCore::RenderManager::GetInstance()->UpdateStyle([pageId UTF8String] ?: "", [ref UTF8String] ?: "", [self _parseMapValuePairs:data]);
}

+ (void)callAddEvent:(NSString*)pageId ref:(NSString*)ref event:(NSString*)event
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->AddEvent([pageId UTF8String] ?: "", [ref UTF8String] ?: "", [event UTF8String] ?: "");
}

+ (void)callRemoveEvent:(NSString*)pageId ref:(NSString*)ref event:(NSString*)event
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveEvent([pageId UTF8String] ?: "", [ref UTF8String] ?: "", [event UTF8String] ?: "");
}

+ (void)callCreateFinish:(NSString*)pageId
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish([pageId UTF8String] ?: "");
}

+ (void)callRefreshFinish:(NSString*)pageId
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->RefreshFinish([pageId UTF8String] ?: "", nullptr, nullptr);
}

+ (void)callUpdateFinish:(NSString*)pageId
{
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->UpdateFinish([pageId UTF8String] ?: "", nullptr, 0, nullptr, 0);
}

+ (void)registerCoreEnv:(NSString*)key withValue:(NSString*)value
{
    WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->core_side()->RegisterCoreEnv([key UTF8String]?:"", [value UTF8String]?:"");
}

@end
