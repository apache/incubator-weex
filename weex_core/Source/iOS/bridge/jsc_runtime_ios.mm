

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#include "jsc_runtime_ios.h"
#include "jsc_context_ios.h"

namespace WeexCore {
    
    static uint32_t baseVMId = 0;
    uint32_t baseContextInVmId = 0;
    
    JSVirtualMachine* vm;
    uint32_t vmId;
    
    std::map<uint32_t,JSCContextIOS*> contextMap;
    
    
    JSCRunTimeIOS::~JSCRunTimeIOS(){
        destroy();
    }
    
    bool JSCRunTimeIOS::initRunTime(){
        vmId = ++baseVMId;
        vm = [[JSVirtualMachine alloc] init];
        return true;
    }

    void JSCRunTimeIOS::destroy(){
        vm = nil;
        contextMap.clear();
    }

    BaseJSContext *JSCRunTimeIOS::createContext(){
        JSContext* contextImpl = [[JSContext alloc] initWithVirtualMachine:vm];
        JSCContextIOS* contextWrapper = new JSCContextIOS();
        contextWrapper->onInit(++baseContextInVmId, (__bridge_retained void*)contextImpl, this);
        contextMap.insert(std::pair<uint32_t, JSCContextIOS*>(contextWrapper->getContextId(),contextWrapper));
        return contextWrapper;
    }

    void JSCRunTimeIOS::destroyContext(BaseJSContext *context){
        JSCContextIOS* contextWrapper = contextMap.at(context->getContextId());
        if (nullptr == contextWrapper) {
            return;
        }
        contextMap.erase(context->getContextId());
        delete contextWrapper;
    }
    
    uint32_t JSCRunTimeIOS::getRunTimeId(){
        return vmId;
    }
}



