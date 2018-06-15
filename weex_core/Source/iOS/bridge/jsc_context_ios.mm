#include "jsc_context_ios.h"
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import <iOS/parser/wson_parser_ios.h>
#include <core/manager/weex_core_manager.h>

namespace WeexCore {
    
    JSContext* jsContextImpl;
    JSCRunTimeIOS* mVM;
    
    JSCContextIOS::~JSCContextIOS(){
        jsContextImpl = nil;
        mVM = nil;
    }
    
    uint32_t JSCContextIOS::getContextId(){
        return contextId;
    }
    
    BaseJSRunTime* JSCContextIOS::getJsRunTime(){
        return mVM;
    }
    
    void JSCContextIOS::execJSMethod(char *methodName, wson_buffer *args){
        NSString* ocMethodName = [[NSString alloc] initWithUTF8String:methodName];
        id ocArgs = [WsonParser toVal:args];
        [[jsContextImpl globalObject] invokeMethod:ocMethodName withArguments:ocArgs];
    }
    
    wson_buffer *JSCContextIOS::execJSMethodWithResult(char *methodName, wson_buffer *args){
        NSString* ocMethodName = [[NSString alloc] initWithUTF8String:methodName];
        id ocArgs = [WsonParser toVal:args];
        JSValue * result =[[jsContextImpl globalObject] invokeMethod:ocMethodName withArguments:ocArgs];
    
        return [WsonParser toWson:[result toString]];
    }
    
    bool JSCContextIOS::executeJavascript(char *script){
        NSString* ocScript = [[NSString alloc] initWithUTF8String:script];
        [jsContextImpl evaluateScript:ocScript];
        return true;
    }
    
    void JSCContextIOS::reigsterJSVale(char *name, wson_buffer *valuse){
        NSString* ocName = [[NSString alloc] initWithUTF8String:name];
        id ocValue = [WsonParser toVal:valuse];
        jsContextImpl[ocName] = ocValue;
    }
    
    wson_buffer *JSCContextIOS::getJSVale(char *name){
        NSString* ocName = [[NSString alloc] initWithUTF8String:name];
        JSValue * result = jsContextImpl[ocName] ;
        return [WsonParser toWson:[result toString]];
    }
    
    void JSCContextIOS::reigsterJSFunc(wson_buffer *func){
        return;
    }
    
    
    void registerFunc(){
        jsContextImpl.exceptionHandler= ^(JSContext *context, JSValue *exception){
            const char* cException = [[exception toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onReportException("p", "func", cException);
        };
        
        jsContextImpl[@"setJSVersion"]=^(JSValue* jsVersion){
            const char* cJSVersion =[[jsVersion toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onSetJSVersion(cJSVersion);
        };
        
        jsContextImpl[@"callNative"]=^(JSValue* pageId,JSValue* task,JSValue *callBack){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cTask = [[task toString] UTF8String];
            const char* cCallBack = [[callBack toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onCallNative(cPageId, cTask, cCallBack);
        };
        
        jsContextImpl[@"callCreabody"]=^(JSValue* pageId,JSValue* task,JSValue *callBack,JSValue *AAAA){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cTask = [[task toString] UTF8String];
            const char* cCallBack = [[callBack toString] UTF8String];
            
            WeexCoreManager::getInstance()->getJSBridge()->onCallNative(cPageId, cTask, cCallBack);
        };
        
        jsContextImpl[@"callNativeModule"]=^(JSValue* pageId,JSValue* module,JSValue *method,JSValue *arguments,JSValue *argumentsLength,
                                             JSValue *options,JSValue* optionsLength){
            
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cModule = [[module toString] UTF8String];
            const char* cMethod = [[method toString] UTF8String];
            const char* cArguments = [[arguments toString] UTF8String];
            int  cArgumentsLength = [argumentsLength toInt32];
            
            const char* cOptions = [[options toString] UTF8String];
            int cOptionLenth = [optionsLength toInt32];

            
            WeexCoreManager::getInstance()->getJSBridge()->onCallNativeModule(
                cPageId, cModule, cMethod, cArguments, cArgumentsLength,cOptions, cOptionLenth
            );
        };
        
        jsContextImpl[@"callNativeComponent"]=^(JSValue* pageId,JSValue* ref,JSValue *method,JSValue *arguments,JSValue *argumentsLength,
                                             JSValue *options,JSValue* optionsLength){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];
            const char* cMethod = [[method toString] UTF8String];
            const char* cArguments = [[arguments toString] UTF8String];
            int  cArgumentsLength = [argumentsLength toInt32];
            
            const char* cOptions = [[options toString] UTF8String];
            int cOptionLenth = [optionsLength toInt32];
            
            WeexCoreManager::getInstance()->getJSBridge()->onCallNativeModule(
                    cPageId, cRef, cMethod, cArguments, cArgumentsLength,cOptions, cOptionLenth
            );
        };
        
         jsContextImpl[@"callAddElement"]= ^void(JSValue *pageId, JSValue *parentRef,JSValue *domStr,JSValue *index_cstr) {
            
             const char* cPageId = [[pageId toString] UTF8String];
             const char* cParentRef = [[parentRef toString] UTF8String];
             const char* cDomStr = [[domStr toString] UTF8String];
             const char* cIndexCstr = [[index_cstr toString] UTF8String];
             WeexCoreManager::getInstance()->getJSBridge()->onCallAddElement(
                   cPageId, cParentRef, cDomStr, cIndexCstr
            );
         };
        
        jsContextImpl[@"setTimeout"]= ^void(JSValue *callbackId, JSValue *time) {
            
            const char* cCallbackId = [[callbackId toString] UTF8String];
            const char* cTime= [[time toString] UTF8String];
 
            WeexCoreManager::getInstance()->getJSBridge()->onSetTimeout(cCallbackId, cTime);
        };
        
        jsContextImpl[@"callAddElement"]= ^void(JSValue *pageId, JSValue *parentRef,JSValue *domStr,JSValue *index_cstr) {
            
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cParentRef = [[parentRef toString] UTF8String];
            const char* cDomStr = [[domStr toString] UTF8String];
            const char* cIndexCstr = [[index_cstr toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onCallAddElement(
                  cPageId, cParentRef, cDomStr, cIndexCstr
            );
        };
        
        jsContextImpl[@"callNativeLog"]= ^void(JSValue *str_array) {
            const char*  cStrArray = [[str_array toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onCallNativeLog(cStrArray);
        };
        
        jsContextImpl[@"callCreateBody"]= ^void(JSValue *pageId,JSValue* domStr) {
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cDomStr = [[domStr toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onCallCreateBody(cPageId, cDomStr);
        };
        
        jsContextImpl[@"callUpdateFinish"]=^JSValue*(JSValue *pageId,JSValue* task,JSValue* callback) {
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cTask = [[task toString] UTF8String];
            const char* cCallBack = [[task toString] UTF8String];
            int result =WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateFinish(cPageId, cTask, cCallBack);
            JSValue* value = [JSValue valueWithInt32:result inContext:jsContextImpl];
            return value;
        };
        
        jsContextImpl[@"callCreateFinish"]=^void(JSValue *pageId) {
            const char* cPageId = [[pageId toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onCallCreateFinish(cPageId);
        };
        
        jsContextImpl[@"callRefreshFinish"]=^JSValue*(JSValue *pageId,JSValue* task,JSValue* callback){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cTask = [[task toString] UTF8String];
            const char* cCallBack = [[task toString] UTF8String];
            
            int result= WeexCoreManager::getInstance()->getJSBridge()->onCallRefreshFinish(cPageId, cTask, cCallBack);

            JSValue* value = [JSValue valueWithInt32:result inContext:jsContextImpl];
            return value;
        };
        
        jsContextImpl[@"callUpdateAttrs"]=^void(JSValue *pageId,JSValue* ref,JSValue* data){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];
            const char* cData = [[data toString] UTF8String];
            
            WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateAttrs(cPageId, cRef, cData);
        };
        
        jsContextImpl[@"callUpdateStyle"]=^void(JSValue *pageId,JSValue* ref,JSValue* data){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];
            const char* cData = [[data toString] UTF8String];
        
            WeexCoreManager::getInstance()->getJSBridge()->onCallUpdateStyle(cPageId, cRef, cData);
        };
        
        jsContextImpl[@"callRemoveElement"]=^void(JSValue *pageId,JSValue* ref){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];

            WeexCoreManager::getInstance()->getJSBridge()->onCallRemoveElement(cPageId, cRef);
        };
        
        jsContextImpl[@"callMoveElement"]=^void(JSValue *pageId,JSValue* ref,JSValue* parentRef,JSValue* index){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];
            const char* cParentRef = [[ref toString] UTF8String];
            int cIndex = [index toInt32];
            
            WeexCoreManager::getInstance()->getJSBridge()->onCallMoveElement(cPageId, cRef,cParentRef,cIndex);
        };
        
        jsContextImpl[@"callAddEvent"]=^void(JSValue *pageId,JSValue* ref,JSValue* event){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];
            const char* cEvent = [[event toString] UTF8String];
    
            WeexCoreManager::getInstance()->getJSBridge()->onCallAddEvent(cPageId, cRef,cEvent);
        };
        
        jsContextImpl[@"callRemoveEvent"]=^void(JSValue *pageId,JSValue* ref,JSValue* event){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cRef = [[ref toString] UTF8String];
            const char* cEvent = [[event toString] UTF8String];
            
            WeexCoreManager::getInstance()->getJSBridge()->onCallRemoveEvent(cPageId, cRef,cEvent);
        };
        
        jsContextImpl[@"setInterval"]=^JSValue*(JSValue *pageId,JSValue* callbackId,JSValue* _time){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cCallbackId = [[callbackId toString] UTF8String];
            const char* cTime = [[_time toString] UTF8String];
            
            int result =WeexCoreManager::getInstance()->getJSBridge()->onSetInterval(cPageId, cCallbackId,cTime);
            
            JSValue* value = [JSValue valueWithInt32:result inContext:jsContextImpl];
            return value;
        };
        
        jsContextImpl[@"clearInterval"]=^void(JSValue *pageId,JSValue* callbackId){
            const char* cPageId = [[pageId toString] UTF8String];
            const char* cCallbackId = [[callbackId toString] UTF8String];
            WeexCoreManager::getInstance()->getJSBridge()->onClearInterval(cPageId, cCallbackId);
        };
    }
    
    void JSCContextIOS::onInit(uint32_t contextId,void* impl, JSCRunTimeIOS* runTime){
        mVM = runTime;
        jsContextImpl = (__bridge_transfer JSContext*) impl;
        this->contextId = contextId;
        registerFunc();
    }
}

