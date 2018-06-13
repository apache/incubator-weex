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
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <base/TimeUtils.h>

#pragma mark - OC related
@interface WXCoreBridgeOCImpl:NSObject

@end

@implementation WXCoreBridgeOCImpl

#pragma mark interface
- (void)implSetTimeOutWithCallback:(const char *)callbackID timeout:(const char *)timeout{
    NSString *timeoutString = [NSString stringWithCString:timeout encoding:NSUTF8StringEncoding];
    __weak typeof(self) weakSelf = self;
    [self performSelector: @selector(triggerTimeout:) withObject:^() {
        JSContext *context = [weakSelf defaultJsContext];
        NSString *callbackIdString = [NSString stringWithCString:callbackID encoding:NSUTF8StringEncoding];
        JSValue *targetFunction = context[callbackIdString];
        [targetFunction callWithArguments:@[]];
    } afterDelay:[timeoutString doubleValue] / 1000];
}

- (int)implCallUpdateFinishWithPageId:(const char*)pageId task:(const char *)task callback:(const char *)callback{
    WXSDKInstance *instance = [self instanceWithId:[NSString stringWithCString:pageId encoding:NSUTF8StringEncoding]];
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
    WXSDKInstance *instance = [self instanceWithId:[NSString stringWithCString:pageId encoding:NSUTF8StringEncoding]];
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
    WXSDKInstance *instance = [self instanceWithId:[NSString stringWithCString:instanceId encoding:NSUTF8StringEncoding]];
    if (!instance) {
        return;
    }
    NSString *callbackString = [NSString stringWithCString:callback encoding:NSUTF8StringEncoding];
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
    
    void static cpyCMap2OCMap(std::map<std::string, std::string> *cMap, NSMutableDictionary *targetDic) {
        std::map<std::string, std::string>::const_iterator it = cMap->begin();
        std::map<std::string, std::string>::const_iterator end = cMap->end();
        for (; it != end; ++it) {
            NSString *key = [NSString stringWithCString:it->first.c_str() encoding:NSUTF8StringEncoding];
            NSString *value = [NSString stringWithCString:it->second.c_str() encoding:NSUTF8StringEncoding];
            [targetDic setValue:value forKey:key];
        }
    }
    
    void static cpyCVector2OCMap(std::vector<std::pair<std::string, std::string>> *cVector, NSMutableDictionary *targetDic) {
        for (int i = 0; i < cVector->size(); ++i) {
            NSString *key = [NSString stringWithCString:(*cVector)[i].first.c_str()
                                               encoding:NSUTF8StringEncoding];
            NSString *value = [NSString stringWithCString:(*cVector)[i].second.c_str()
                                                 encoding:NSUTF8StringEncoding];
            [targetDic setValue:value forKey:key];
        }
    }
    
    void WXCoreBridge::registerEventWithType(WeexCoreEventBlockType type,void* block){
        impl->blockMap[type] = block;
    }
    
    void WXCoreBridge::setJSVersion(const char* version){
        NSString *jsVersion = [NSString stringWithCString:version encoding:NSUTF8StringEncoding];
        if (jsVersion && jsVersion.length>0 ) {
            [WXAppConfiguration setJSFrameworkVersion:jsVersion];
        }
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
            NSString *pageIDString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSData *taskData = [NSData dataWithBytes:task length:strlen(task)];
            NSError *error = nil;
            NSArray *taskArray = [WXUtility JSONObject:taskData error:&error];
            NSString *callBackString = [NSString stringWithCString:callback encoding:NSUTF8StringEncoding];
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
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *moduleString = [NSString stringWithCString:module encoding:NSUTF8StringEncoding];
            NSString *methodString = [NSString stringWithCString:method encoding:NSUTF8StringEncoding];
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
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            NSString *methodString = [NSString stringWithCString:method encoding:NSUTF8StringEncoding];
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
        NSString *logString = [NSString stringWithCString:str_array encoding:NSUTF8StringEncoding];
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
            NSString *pageIDString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            NSString *eventString = [NSString stringWithCString:event encoding:NSUTF8StringEncoding];
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
            NSString *pageIDString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            NSString *eventString = [NSString stringWithCString:event encoding:NSUTF8StringEncoding];
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
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSMutableDictionary *bodyData = [[NSMutableDictionary alloc] init];
            // attr\style\type\ref
            
            NSMutableDictionary *attrDic = [[NSMutableDictionary alloc] init];
            cpyCMap2OCMap(attributes,attrDic);
            [bodyData setValue:attrDic forKey:@"attr"];
            
            NSMutableDictionary *styleDic = [[NSMutableDictionary alloc] init];
            cpyCMap2OCMap(styles,styleDic);
            [bodyData setValue:styleDic forKey:@"style"];
            
            NSString *type = [NSString stringWithCString:componentType encoding:NSUTF8StringEncoding];
            [bodyData setValue:type forKey:@"type"];
            
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            [bodyData setValue:refString forKey:@"type"];
            
            flag = (int)targetFunc(pageIdString,bodyData);
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
        
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallCreateBody];
        int flag = 0;
        if(func != nullptr){
            WXJSCallAddElement targetFunc = (__bridge WXJSCallAddElement)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *parentRefString = [NSString stringWithCString:parentRef encoding:NSUTF8StringEncoding];
            
            NSMutableDictionary *elementData = [[NSMutableDictionary alloc] init];
            // attr\style\type\ref
            
            NSMutableDictionary *attrDic = [[NSMutableDictionary alloc] init];
            cpyCMap2OCMap(attributes,attrDic);
            [elementData setValue:attrDic forKey:@"attr"];
            
            NSMutableDictionary *styleDic = [[NSMutableDictionary alloc] init];
            cpyCMap2OCMap(styles,styleDic);
            [elementData setValue:styleDic forKey:@"style"];
            
            NSString *type = [NSString stringWithCString:componentType encoding:NSUTF8StringEncoding];
            [elementData setValue:type forKey:@"type"];
            
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            [elementData setValue:refString forKey:@"type"];
            
            flag = (int)targetFunc(pageIdString,parentRefString,elementData,index);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callLayout(const char* pageId, const char* ref,
                       int top, int bottom, int left, int right,
                       int height, int width, int index){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallLayout];
        if(func != nullptr){
            WXCoreCallLayout targetFunc = (__bridge WXCoreCallLayout)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            
            flag = (int)targetFunc(pageIdString,refString,(NSInteger)top,(NSInteger)bottom,(NSInteger)left,(NSInteger)right,(NSInteger)height,(NSInteger)width,(NSInteger)index);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callUpdateStyle(const char* pageId, const char* ref,
                            std::vector<std::pair<std::string, std::string>> *style,
                            std::vector<std::pair<std::string, std::string>> *margin,
                            std::vector<std::pair<std::string, std::string>> *padding,
                            std::vector<std::pair<std::string, std::string>> *border){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallUpdateStyle];
        if(func != nullptr){
            WXJSCallUpdateStyle targetFunc = (__bridge WXJSCallUpdateStyle)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            
            NSMutableDictionary *stylesDic = [[NSMutableDictionary alloc] init];
            cpyCVector2OCMap(style,stylesDic);
            
            flag = (int)targetFunc(pageIdString,refString,stylesDic);
        }
        
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callUpdateAttr(const char* pageId, const char* ref,
                           std::vector<std::pair<std::string, std::string>> *attrs){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallUpdateAttr];
        if(func != nullptr){
            WXJSCallUpdateAttrs targetFunc = (__bridge WXJSCallUpdateAttrs)func;
            
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            NSMutableDictionary *attrDic = [[NSMutableDictionary alloc] init];
            cpyCVector2OCMap(attrs,attrDic);
            
            flag = (int)targetFunc(pageIdString,refString,attrDic);
        }
        
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callCreateFinish(const char* pageId){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallCreateFinish];
        if(func != nullptr){
            WXJSCallCreateFinish targetFunc = (__bridge WXJSCallCreateFinish)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            flag = (int)targetFunc(pageIdString);
        }
        
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callRemoveElement(const char* pageId, const char* ref){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallRemoveElement];
        if(func != nullptr){
            WXJSCallRemoveElement targetFunc = (__bridge WXJSCallRemoveElement)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            flag = (int)targetFunc(pageIdString,refString);
        }
        
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallMoveElement];
        if(func != nullptr){
            WXJSCallMoveElement targetFunc = (__bridge WXJSCallMoveElement)func;
            NSString *pageIdString = [NSString stringWithCString:pageId encoding:NSUTF8StringEncoding];
            NSString *refString = [NSString stringWithCString:ref encoding:NSUTF8StringEncoding];
            NSString *parentRefString = [NSString stringWithCString:parentRef encoding:NSUTF8StringEncoding];
            NSInteger indexValue = (NSInteger)index;
            flag = (int)targetFunc(pageIdString,refString,parentRefString,indexValue);
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
        
    int WXCoreBridge::callAppendTreeCreateFinish(const char* pageId, const char* ref){
        RenderPage *page = RenderManager::GetInstance()->GetPage(pageId);
        long long startTime = getCurrentTime();
        int flag = 0;
        void *func = impl->blockMap[WeexCoreEventBlockTypeCallAppendTreeCreateFinish];
        if(func != nullptr){
#warning todo 给recycler做hack用，待补充
            
            
        }
        if (page != nullptr)
            page->CallBridgeTime(getCurrentTime() - startTime);
        return flag;
    }
}

@end





