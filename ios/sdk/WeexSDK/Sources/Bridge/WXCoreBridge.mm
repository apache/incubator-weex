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
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <base/TimeUtils.h>


#pragma mark - OC related
@interface WXCoreBridgeOCImpl:NSObject

@end

@implementation WXCoreBridgeOCImpl

#pragma mark interface
- (void)implSetTimeOutWithCallback:(const char *)callbackID timeout:(const char *)timeout{
    NSString *timeoutString = [NSString stringWithCString:timeout encoding:NSUnicodeStringEncoding];
    __weak typeof(self) weakSelf = self;
    [self performSelector: @selector(triggerTimeout:) withObject:^() {
        JSContext *context = [weakSelf defaultJsContext];
        NSString *callbackIdString = [NSString stringWithCString:callbackID encoding:NSUnicodeStringEncoding];
        JSValue *targetFunction = context[callbackIdString];
        [targetFunction callWithArguments:@[]];
    } afterDelay:[timeoutString doubleValue] / 1000];
}

- (int)implCallUpdateFinishWithPageId:(const char*)pageId task:(const char *)task callback:(const char *)callback{
    WXSDKInstance *instance = [self instanceWithId:[NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding]];
    if (instance) {
        WXComponentManager *componentManager = instance.componentManager;
        [componentManager updateFinish];
    }else{
        return -1;
    }
    [self callJsCallBackWithInstanceId:pageId callback:callback];
    return 0;
}

- (int)implCallRefreshFinishWithPageId:(const char*)pageId task:(const char *)task callback:(const char *)callback{
    WXSDKInstance *instance = [self instanceWithId:[NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding]];
    if (instance) {
        WXComponentManager *componentManager = instance.componentManager;
        [componentManager refreshFinish];
    }else{
        return -1;
    }
    [self callJsCallBackWithInstanceId:pageId callback:callback];
    return 0;
}

- (void)implCallAddEventWithPageId:(const char*)pageId ref:(const char*)ref event:(const char *)event{
    
}

#pragma mark inner method
- (void)triggerTimeout:(void(^)(void))block{
    block();
}

- (WXSDKInstance *)defaultInstance{
    return [self instanceWithId:@""];
}

- (WXSDKInstance *)instanceWithId:(NSString *)instanceId{
    return [WXSDKManager instanceForID:instanceId];
}

- (JSContext *)defaultJsContext{
    return [self defaultInstance].instanceJavaScriptContext;
}

- (void)callJsCallBackWithInstanceId:(const char*)instanceId callback:(const char *)callback{
    WXSDKInstance *instance = [self instanceWithId:[NSString stringWithCString:instanceId encoding:NSUnicodeStringEncoding]];
    if (!instance) {
        return;
    }
    NSString *callbackString = [NSString stringWithCString:callback encoding:NSUnicodeStringEncoding];
    if (callbackString && callbackString.length>0) {
        JSContext *context = instance.instanceJavaScriptContext;
        JSValue *callbackFunction = context[callbackString];
        [callbackFunction callWithArguments:@[]];
    }
}

#pragma mark - C++ related
namespace WeexCore {
    
    struct WXCoreBridgeImpl{
        WXCoreBridgeOCImpl *ocImpl;
        std::map<WeexCoreEventBlockType,void *> blockMap;
    };
    
    WXCoreBridge::WXCoreBridge(){
        impl = new WXCoreBridgeImpl();
        impl->ocImpl = [[WXCoreBridgeOCImpl alloc] init];
    }
    
    WXCoreBridge::~WXCoreBridge(){
        if(impl){
            impl->ocImpl = nil;
            impl->blockMap.clear();
        }
        delete impl;
    }
    
    void WXCoreBridge::registerEventWithType(WeexCoreEventBlockType type,void* block){
        impl->blockMap[type] = block;
    }
    
    void WXCoreBridge::setJSVersion(const char* version){
#warning todo
    }
    
    void WXCoreBridge::reportException(const char* pageId, const char *func, const char *exception_string){
#warning todo
    }
    
    int WXCoreBridge::callNative(const char* pageId, const char *task, const char *callback){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        int flag = -1;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallNative];
        if(func != nullptr){
            WXJSCallNative targetFunc = (__bridge WXJSCallNative)func;
            NSString *pageIDString = [NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding];
            NSData *taskData = [NSData dataWithBytes:task length:strlen(task)];
            NSError *error = nil;
            NSArray *taskArray = [WXUtility JSONObject:taskData error:&error];
            NSString *callBackString = [NSString stringWithCString:callback encoding:NSUnicodeStringEncoding];
            flag = (int)targetFunc(pageIDString,taskArray,callBackString);
        }
        if (flag == -1) {
            WXLog(@"%@",@"instance destroy JFM must stop callNative");
        }
        
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
    
    void* WXCoreBridge::callNativeModule(const char* pageId, const char *module, const char *method,
                                         const char *arguments, int argumentsLength, const char *options, int optionsLength){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallNativeModule];
        void *result = nullptr;
        if(func != nullptr){
            WXJSCallNativeModule targetFunc = (__bridge WXJSCallNativeModule)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding];
            NSString *moduleString = [NSString stringWithCString:module encoding:NSUnicodeStringEncoding];
            NSString *methodString = [NSString stringWithCString:method encoding:NSUnicodeStringEncoding];
            NSError *error = nil;
            NSData *argumentsData = [NSData dataWithBytes:arguments length:argumentsLength];
            NSArray *argumentsArray = [WXUtility JSONObject:argumentsData error:&error];
            NSData *optionsData = [NSData dataWithBytes:options length:optionsLength];
            NSDictionary *optinionsDic = [WXUtility JSONObject:optionsData error:&error];
            result = (__bridge void *) targetFunc(pageIdString,moduleString,methodString,argumentsArray,optinionsDic);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return result;
    }
        
    void WXCoreBridge::callNativeComponent(const char* pageId, const char* ref, const char *method,
                                           const char *arguments, int argumentsLength, const char *options, int optionsLength){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallNativeComponent];
        if(func != nullptr){
            WXJSCallNativeComponent targetFunc = (__bridge WXJSCallNativeComponent)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUnicodeStringEncoding];
            NSString *methodString = [NSString stringWithCString:method encoding:NSUnicodeStringEncoding];
            NSError *error = nil;
            NSData *argumentsData = [NSData dataWithBytes:arguments length:argumentsLength];
            NSArray *argumentsArray = [WXUtility JSONObject:argumentsData error:&error];
            NSData *optionsData = [NSData dataWithBytes:options length:optionsLength];
            NSDictionary *optionsDic = [WXUtility JSONObject:optionsData error:&error];
            targetFunc(pageIdString,refString,methodString,argumentsArray,optionsDic);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
    }
        
    void WXCoreBridge::setTimeout(const char* callbackID, const char* time){
            // this setTimeout is used by internal logic in JS framework, normal setTimeout called by users will call WXTimerModule's method;
        [impl->ocImpl implSetTimeOutWithCallback:callbackID timeout:time];
    }
        
    void WXCoreBridge::callNativeLog(const char* str_array){
        NSString *logString = [NSString stringWithCString:str_array encoding:NSUnicodeStringEncoding];
        NSLog(@"%@",logString);
    }
        
    int WXCoreBridge::callUpdateFinish(const char* pageId, const char *task, const char *callback){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = [impl->ocImpl implCallUpdateFinishWithPageId:pageId task:task callback:callback];
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callRefreshFinish(const char* pageId, const char *task, const char *callback){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = [impl->ocImpl implCallRefreshFinishWithPageId:pageId task:task callback:callback];
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callAddEvent(const char* pageId, const char* ref, const char *event){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallAddEvent];
        int flag = -1;
        if(func != nullptr){
            WXJSCallAddEvent targetFunc = (__bridge WXJSCallAddEvent)func;
            NSString *pageIDString = [NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUnicodeStringEncoding];
            NSString *eventString = [NSString stringWithCString:event encoding:NSUnicodeStringEncoding];
            flag = (int)targetFunc(pageIDString,refString,eventString);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callRemoveEvent(const char* pageId, const char* ref, const char *event){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = -1;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallRemoveEvent];
        if(func != nullptr){
            WXJSCallRemoveEvent targetFunc = (__bridge WXJSCallRemoveEvent)func;
            NSString *pageIDString = [NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUnicodeStringEncoding];
            NSString *eventString = [NSString stringWithCString:event encoding:NSUnicodeStringEncoding];
            flag = (int)targetFunc(pageIDString,refString,eventString);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callCreateBody(const char* pageId, const char *componentType, const char* ref,
                                     std::map<std::string, std::string> *styles,
                                     std::map<std::string, std::string> *attributes,
                                     std::set<std::string> *events,
                                     const WXCoreMargin &margins,
                                     const WXCorePadding &paddings,
                                     const WXCoreBorderWidth &borders){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallCreateBody];
        int flag = 0;
        if(func != nullptr){
            WXJSCallCreateBody targetFunc = (__bridge WXJSCallCreateBody)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUnicodeStringEncoding];

            
            
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callAddElement(const char* pageId, const char *componentType, const char* ref,
                           int &index, const char* parentRef,
                           std::map<std::string, std::string> *styles,
                           std::map<std::string, std::string> *attributes,
                           std::set<std::string> *events,
                           const WXCoreMargin &margins,
                           const WXCorePadding &paddings,
                           const WXCoreBorderWidth &borders,
                           bool willLayout){
#warning todo
    }
        
    int WXCoreBridge::callLayout(const char* pageId, const char* ref,
                       int top, int bottom, int left, int right,
                       int height, int width, int index){
#warning todo
    }
        
    int WXCoreBridge::callUpdateStyle(const char* pageId, const char* ref,
                            std::vector<std::pair<std::string, std::string>> *style,
                            std::vector<std::pair<std::string, std::string>> *margin,
                            std::vector<std::pair<std::string, std::string>> *padding,
                            std::vector<std::pair<std::string, std::string>> *border){
#warning todo
    }
        
    int WXCoreBridge::callUpdateAttr(const char* pageId, const char* ref,
                           std::vector<std::pair<std::string, std::string>> *attrs){
#warning todo
    }
        
    int WXCoreBridge::callCreateFinish(const char* pageId){
#warning todo
    }
        
    int WXCoreBridge::callRemoveElement(const char* pageId, const char* ref){
#warning todo
    }
        
    int WXCoreBridge::callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index){
#warning todo
    }
        
    int WXCoreBridge::callAppendTreeCreateFinish(const char* pageId, const char* ref){
#warning todo
    }
        
    
}

@end





