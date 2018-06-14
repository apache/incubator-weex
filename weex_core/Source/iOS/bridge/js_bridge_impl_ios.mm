
#import "js_bridge_impl_ios.h"
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import <vector>
#import "wson_parser_ios.h"

@interface VmRecorder :NSObject
@property (nonatomic,assign) uint32_t vmId;
@property (nonatomic,strong) JSVirtualMachine * runTime;
@property (nonatomic,strong) NSMutableDictionary* contextMap;
@end


@implementation VmRecorder

- (instancetype) init
{
    self = [super init];
    if (self) {
        _runTime = [[JSVirtualMachine alloc] init];
        _contextMap = [[NSMutableDictionary alloc] init];
    }
    return self;
}

-(void)dealloc{
//    _runTime = nil;
//    [_contextMap removeAllObjects];
//    _contextMap = nil;
}

- (JSContext*)finContext:(uint32_t) contextId{
    
}
- (bool) createContext:(uint32_t) contextId{
    JSContext* context = [[JSContext alloc] initWithVirtualMachine:self.runTime];
    NSNumber* key = [NSNumber numberWithUnsignedInt:contextId];
    [self.contextMap setObject:context forKey:key];
    return true;
}

- (void) destroyContext:(uint32_t) contextId{
    NSNumber* key = [NSNumber numberWithUnsignedInt:contextId];
    JSContext* context =[self.contextMap objectForKey:key];
    [self.contextMap removeObjectForKey:key];
}

@end







namespace WeexCore {

    typedef struct JSContextStruct {
        int32_t contextId;
        JSContext *context;
        int32_t vmId;
    } JSContextRecorder;
    typedef struct JSVMStruct {
        int32_t id;
        JSVirtualMachine *jsVM;
        std::vector<JSContextRecorder *> contextList;
    } JSVMRecorder;

    std::map<uint32_t, JSVMStruct *> vmMap;
    
    JSBridgeIOS::~JSBridgeIOS(){
        vmMap.clear();
    }
    
    JSContextRecorder* findContext(uint32_t runTimeId, uint32_t contextId)
    {
        auto iter = vmMap.find(runTimeId);
        if (iter == vmMap.end()) {
            return nullptr;
        }
        JSVMRecorder *vmRecorder = vmMap.at(runTimeId);
        JSContextRecorder *targetContextRecorder = nullptr;
        for (size_t i =0; i<vmRecorder->contextList.size(); i++) {
            targetContextRecorder=vmRecorder->contextList[i];
            if (contextId == targetContextRecorder->contextId) {
                break;
            }
        }
        return targetContextRecorder;
    }
    
    bool JSBridgeIOS::createJSRunTime(uint32_t runTimeId, std::map<std::string, std::string> *params) {
        auto iter = vmMap.find(runTimeId);
        if (iter == vmMap.end()) {
            return true;
        }
        JSVirtualMachine * runTime = [[JSVirtualMachine alloc] init];
        JSVMRecorder *vm = new JSVMRecorder();
        vm->id = runTimeId;
        vm->jsVM = runTime;
        vmMap.insert(std::pair<uint32_t, JSVMStruct *>(runTimeId, vm));
        return true;
    }

    bool JSBridgeIOS::destroyJSRunTime(uint32_t runTimeId) {
        auto iter = vmMap.find(runTimeId);
        if (iter == vmMap.end()) {
            return true;
        }
        JSVMRecorder* vm = vmMap.at(runTimeId);
        while (vm->contextList.size()>0) {
            destroyJSContext(runTimeId, vm->contextList[0]->contextId);
        }

        vmMap.erase(runTimeId);
        vm->contextList.clear();
        delete vm;
        vm = nullptr;
        //todo call js destroy vm ?
        return true;
    }

    bool JSBridgeIOS::createJSContext(uint32_t runTimeId, uint32_t contextId) {
        auto iter = vmMap.find(runTimeId);
        if (iter == vmMap.end()) {
            return false;
        }
        JSVMRecorder *vmRecorder = vmMap.at(runTimeId);
        for (size_t i =0; i<vmRecorder->contextList.size(); i++) {
            if (contextId == vmRecorder->contextList[i]->contextId) {
                return false;
            }
        }
        JSContext * context = [[JSContext alloc] initWithVirtualMachine:vmRecorder->jsVM];
        JSContextRecorder *contextRecorder = new JSContextRecorder();
        contextRecorder->vmId=runTimeId;
        contextRecorder->context=context;
        contextRecorder->contextId=contextId;
        vmRecorder->contextList.push_back(contextRecorder);
        return true;
    }

    bool JSBridgeIOS::destroyJSContext(uint32_t runTimeId, uint32_t contextId) {
        auto iter = vmMap.find(runTimeId);
        if (iter == vmMap.end()) {
            return true;
        }
        JSVMRecorder *vmRecorder = vmMap.at(runTimeId);
        JSContextRecorder *targetContextRecorder = nullptr;
        
        size_t targetIndex = 0;
        while (targetIndex < vmRecorder->contextList.size()) {
            targetContextRecorder = vmRecorder->contextList[targetIndex];
            if (contextId == targetContextRecorder->contextId) {
                break;
            }
            targetIndex++;
        }
    
        if (targetIndex < vmRecorder->contextList.size()) {
            targetContextRecorder->context = nullptr;
            vmRecorder->contextList.erase(vmRecorder->contextList.begin() + targetIndex);
            delete targetContextRecorder;
            targetContextRecorder = nullptr;
            //execJSMethod
        }
        return true;
    }
    
    JSValue* doExecJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName,wson_buffer* args){
        JSContextRecorder *targetContextRecorder = findContext(runTimeId,contextId);
        if (nullptr == targetContextRecorder) {
            return nil;
        }
        JSContext* jsContext = targetContextRecorder->context;
        
        NSString* jsMethod=[[NSString alloc] initWithUTF8String:methodName];
        
        id jsArgs = [WsonParser toVal:args];
        if (![jsArgs isKindOfClass:[NSArray class]]) {
            return nil;
        }
        NSArray* arrayArgs = (NSArray*)jsArgs;
        
        // WXLogDebug(@"Calling JS... method:%@, args:%@", jsMethod, jsArgs);
        
       return [[jsContext globalObject] invokeMethod:jsMethod withArguments:arrayArgs];
    }

    void JSBridgeIOS::execJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName,wson_buffer* args) {
        doExecJSMethod(runTimeId,contextId,methodName,args);
    }
    
    wson_buffer*
    JSBridgeIOS::execJSMethodWithResult(uint32_t runTimeId, uint32_t contextId, char *methodName, wson_buffer* args){
        JSValue* result = doExecJSMethod(runTimeId, contextId, methodName, args);
        if (result == nil) {
            return nullptr;
        }
        wson_buffer* buffer = [WsonParser toWson:[result toString]];
        return buffer;
    }

    bool JSBridgeIOS::executeJavascript(uint32_t runTimeId, uint32_t contextId, char *script) {
        if (nullptr == script) {
            return false;
        }
        JSContextRecorder *targetContextRecorder = findContext(runTimeId,contextId);
        if (nullptr == targetContextRecorder) {
            return false;
        }
        JSContext* jsContext = targetContextRecorder->context;
        NSString* nsScript = [[NSString alloc] initWithUTF8String:script];
        
        [jsContext evaluateScript:nsScript];
        return true;
    }

    void JSBridgeIOS::reigsterJSVale(uint32_t runTimeId, uint32_t contextId, char *name, wson_buffer* value) {
        JSContextRecorder *recorder = findContext(runTimeId, contextId);
        if (nullptr == recorder) {
            return;
        }
        JSContext * jsContext = recorder->context;
        NSString * protoName = [[NSString alloc] initWithUTF8String:name];
        id ocValue = [WsonParser toVal:value];
        jsContext[protoName] = ocValue;
    }

    wson_buffer*  JSBridgeIOS::getJSVale(uint32_t runTimeId, uint32_t contextId, char *name) {
        JSContextRecorder *recorder = findContext(runTimeId, contextId);
        if (nullptr == recorder) {
            return nullptr;
        }
        JSContext * jsContext = recorder->context;
        NSString * protoName = [[NSString alloc] initWithUTF8String:name];
        id ocValue = jsContext[protoName];
        
        return [WsonParser toWson:ocValue];
    }

    void JSBridgeIOS::reigsterJSFunc(uint32_t runTimeId, uint32_t contextId, wson_buffer*  func) {
        
    }
}
