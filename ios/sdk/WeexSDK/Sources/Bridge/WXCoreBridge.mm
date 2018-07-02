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
#import "WXLog.h"
#import "WXTracingManager.h"
#import "WXBridgeProtocol.h"
#import "WXUtility.h"
#import "WXAppConfiguration.h"
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <base/TimeUtils.h>
#import "WXLayoutConstraint.h"

#define NSSTRING(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCString(NULL, (const char *)(cstr), kCFStringEncodingUTF8)))
#define NSSTRING_NO_COPY(cstr) ((__bridge_transfer NSString*)(CFStringCreateWithCStringNoCopy(NULL, (const char *)(cstr), kCFStringEncodingUTF8, kCFAllocatorNull)))

#pragma mark - C++ related
namespace WeexCore
{
    WXCoreBridge::WXCoreBridge()
    {
    }
    
    WXCoreBridge::~WXCoreBridge()
    {
    }
    
    static NSDictionary* NSDICTIONARY(std::map<std::string, std::string>* map)
    {
        if (map == nullptr)
            return @{};
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:map->size()];
        for (auto it = map->begin(); it != map->end(); it ++) {
            [result setObject:NSSTRING(it->second.c_str()) forKey:NSSTRING(it->first.c_str())];
        }
        return result;
    }
    
    static NSDictionary* NSDICTIONARY(std::vector<std::pair<std::string, std::string>>* vec)
    {
        if (vec == nullptr)
            return @{};
        
        NSMutableDictionary* result = [[NSMutableDictionary alloc] initWithCapacity:vec->size()];
        for (auto& p : *vec) {
            [result setObject:NSSTRING(p.second.c_str()) forKey:NSSTRING(p.first.c_str())];
        }
        return result;
    }
    
    static NSArray* NSARRAY(std::set<std::string>* set)
    {
        if (set == nullptr)
            return @[];
        
        NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:set->size()];
        for (auto& s : *set) {
            [result addObject:NSSTRING(s.c_str())];
        }
        return result;
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
        
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: addEvent ref:%@", ns_ref);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager startComponentTasks];
            [manager wxcore_AddEvent:ns_event toComponent:ns_ref];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"addEvent" options:nil];
        });
        
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
        
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action :removeEvent ref:%@", ns_ref);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager startComponentTasks];
            [manager wxcore_RemoveEvent:ns_event fromComponent:ns_ref];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"removeEvent" options:nil];
        });
    
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
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_type = NSSTRING(componentType);
        NSDictionary* ns_styles = NSDICTIONARY(styles);
        NSDictionary* ns_attributes = NSDICTIONARY(attributes);
        NSArray* ns_events = NSARRAY(events);
        
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: createBody %@ ref:%@", ns_type, ns_ref);
#endif

            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"createBody" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager wxcore_CreateBody:@{@"ref": ns_ref, @"type": ns_type,
                                         @"style": ns_styles, @"attr": ns_attributes, @"event": ns_events}];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTDOMThread}];
        });

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
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_componentType = NSSTRING(componentType);
        NSString* ns_ref = NSSTRING(ref);
        NSString* ns_parentRef = NSSTRING(parentRef);
        NSDictionary* ns_styles = NSDICTIONARY(styles);
        NSDictionary* ns_attributes = NSDICTIONARY(attributes);
        NSArray* ns_events = NSARRAY(events);
        NSInteger ns_index = index;
        
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"addElement" options:nil];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: addElement : %@", ns_componentType);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"addElement" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager wxcore_AddElement:@{@"ref": ns_ref, @"type": ns_componentType,
                                         @"style": ns_styles, @"attr": ns_attributes, @"event": ns_events}
                      toSupercomponent:ns_parentRef atIndex:ns_index];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"addElement" options:@{@"threadName":WXTDOMThread}];
        });

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int WXCoreBridge::callLayout(const char* pageId, const char* ref,
                       int top, int bottom, int left, int right,
                       int height, int width, int index)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        
        WXPerformBlockOnComponentThread(^{
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager wxcore_Layout:ns_ref frame:CGRectMake(left, top, width, height)];
        });

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
        NSDictionary* ns_style = NSDICTIONARY(style);
//        NSDictionary* ns_margin = NSDICTIONARY(margin);
//        NSDictionary* ns_padding = NSDICTIONARY(padding);
//        NSDictionary* ns_border = NSDICTIONARY(border);
        
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"updateStyles" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: updateStyles ref:%@, styles:%@", ns_ref, ns_style);
#endif

            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            
            [manager startComponentTasks];
            [manager wxcore_UpdateStyles:ns_style forComponent:ns_ref];
        });
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callUpdateAttr(const char* pageId, const char* ref,
                           std::vector<std::pair<std::string, std::string>> *attrs)
    {
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        NSDictionary* ns_attributes = NSDICTIONARY(attrs);
        
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: updateAttrs ref:%@, attr:%@", ns_ref, ns_attributes);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"updateAttrs" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager wxcore_UpdateAttributes:ns_attributes forComponent:ns_ref];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTDOMThread}];
        });
        
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
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"createFinish" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: createFinish :%@", ns_instanceId);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"createFinish" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager wxcore_CreateFinish];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createFinish" options:@{@"threadName":WXTDOMThread}];
        });

        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
        
    int WXCoreBridge::callRemoveElement(const char* pageId, const char* ref){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        if (page == nullptr) {
            return -1;
        }
        
        long long startTime = getCurrentTime();
        
        NSString* ns_instanceId = NSSTRING(pageId);
        NSString* ns_ref = NSSTRING(ref);
        
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"removeElement" options:nil];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: removeElement ref:%@", ns_ref);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"removeElement" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager wxcore_RemoveElement:ns_ref];
            [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"removeElement" options:@{@"threadName":WXTDOMThread}];
        });
        
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
        
        [WXTracingManager startTracingWithInstanceId:ns_instanceId ref:ns_ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"moveElement" options:nil];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: moveElement, ref:%@ to ref:%@", ns_ref, ns_parentRef);
#endif
            
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }
            
            [manager startComponentTasks];
            [manager wxcore_MoveComponent:ns_ref toSuper:ns_parentRef atIndex:ns_index];
        });
        
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
        WXPerformBlockOnComponentThread(^{
            WXComponentManager* manager = [WXSDKManager instanceForID:ns_instanceId].componentManager;
            if (!manager.isValid) {
                return;
            }

            [manager startComponentTasks];
            [manager wxcore_AppendTreeCreateFinish:ns_ref];
        });
        
        page->CallBridgeTime(getCurrentTime() - startTime);
        return 0;
    }
    
    int WXCoreBridge::callHasTransitionPros(const char* pageId, const char* ref,
                              std::vector<std::pair<std::string, std::string>> *style){
        
        NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
        NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
#warning todo apply for dom operation
        return 0;
    }
}

#endif
