
#import "js_bridge_impl_ios.h"
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import <vector>
#import "wson_parser_ios.h"

@interface VmRecorder :NSObject
@property (nonatomic,strong) NSNumber* vmId;
@property (nonatomic,strong) JSVirtualMachine * vm;
@property (nonatomic,strong) NSMutableDictionary* contextMap;
@end


@implementation VmRecorder

- (instancetype) init
{
    self = [super init];
    if (self) {
        _vm = [[JSVirtualMachine alloc] init];
        _contextMap = [[NSMutableDictionary alloc] init];
    }
    return self;
}

-(void)dealloc{
//    _runTime = nil;
//    [_contextMap removeAllObjects];
//    _contextMap = nil;
}

- (bool) createContext:(NSNumber *) contextId{
    if ([self.contextMap objectForKey:contextId]) {
        return false;
    }
    JSContext* context = [[JSContext alloc] initWithVirtualMachine:self.vm];
    [self.contextMap setObject:context forKey:contextId];
    return true;
}

- (void) destroyContext:(NSNumber *) contextId{
    JSContext* context =[self.contextMap objectForKey:contextId];
    if (nil == context) {
        return;
    }
    [self.contextMap removeObjectForKey:contextId];
}

@end

@interface JSBridgeIOSImpl :NSObject
@end

@implementation JSBridgeIOSImpl

+ (bool) createJSVM:(NSNumber*) vmId withParams:(NSDictionary*) params{
    VmRecorder* vm = [[self getVMDic] objectForKey:vmId];
    if (nil != vm) {
        return false;
    }
    VmRecorder* recorder = [[VmRecorder alloc] init];
    recorder.vmId= vmId;
    [[JSBridgeIOSImpl getVMDic] setObject:recorder forKey: recorder.vmId];
    return true;
}

+ (void) destroyJSVM:(NSNumber*) vmId {
    VmRecorder* vm = [[self getVMDic] objectForKey:vmId];
    if (nil == vm) {
        return;
    }
    [[self getVMDic] removeObjectForKey:vmId];
    //do sth ?
}

+ (JSContext*) findContext:(NSNumber*)contextId atVM:(NSNumber*) vmId {
    VmRecorder* vm = [[self getVMDic] objectForKey:vmId];
    if (nil == vm) {
        return nil;
    }
    return [vm.contextMap objectForKey:contextId];
}

+ (bool) createJSContext:(NSNumber*)contextId atVM:(NSNumber*) vmId {
    VmRecorder* vm = [[JSBridgeIOSImpl getVMDic] objectForKey:vmId];
    if (nil == vm) {
        return false;
    }
    [vm createContext:contextId];
    return true;
}

+ (void) destroyJSContext:(NSNumber*)contextId atVM:(NSNumber*) vmId {
    VmRecorder* vm = [[JSBridgeIOSImpl getVMDic] objectForKey:vmId];
    if (nil == vm) {
        return;
    }
    [vm destroyContext:contextId];
}

+(JSValue*) doExecJSMethodInContext:(NSNumber*)contextId atVM:(NSNumber*) vmId method:(NSString*)name withArgs:(NSArray*)args {
    JSContext* jsContext = [JSBridgeIOSImpl findContext:contextId atVM:vmId];
    if (nil == jsContext) {
        return nil;
    }
    return [[jsContext globalObject] invokeMethod:name withArguments:args];
}

+ (void) executeJavascriptInContext:(NSNumber*)contextId atVM:(NSNumber*) vmId withScript:(NSString*)script{
    JSContext* jsContext = [JSBridgeIOSImpl findContext:contextId atVM:vmId];
    if (nil == jsContext) {
        return;
    }
    [jsContext evaluateScript:script];
}

+ (void) reigsterJSValeInContext:(NSNumber*)contextId atVM:(NSNumber*) vmId name:(NSString*)name val:(id)val {
    JSContext* jsContext = [JSBridgeIOSImpl findContext:contextId atVM:vmId];
    if (nil == jsContext) {
        return;
    }
    jsContext[name]=val;
}

+(id) getJSValeInContext:(NSNumber*)contextId atVM:(NSNumber*) vmId name:(NSString*)name  {
    JSContext* jsContext = [JSBridgeIOSImpl findContext:contextId atVM:vmId];
    if (nil == jsContext) {
        return nil;
    }
    return jsContext[name];
}

+(void) reigsterJSFuncInContext:(NSNumber*)contextId atVM:(NSNumber*) vmId methodName:(NSString*)method args:(NSArray*)args {
    
}


+ (NSMutableDictionary *) getVMDic{
    static NSMutableDictionary* vmMap;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        vmMap = [[NSMutableDictionary alloc] init];
    });
    return vmMap;
}
@end


namespace WeexCore {
    
    NSNumber* convertId(uint32_t val){
        return [NSNumber numberWithUnsignedInt:val];
    }
   
    bool JSBridgeIOS::createJSRunTime(uint32_t runTimeId, std::map<std::string, std::string> *params) {
        NSNumber* ocVmId = convertId(runTimeId);
       return [JSBridgeIOSImpl createJSVM:ocVmId withParams:nil];
    }

    void JSBridgeIOS::destroyJSRunTime(uint32_t runTimeId) {
        NSNumber* ocVmId = convertId(runTimeId);
        [JSBridgeIOSImpl destroyJSVM:ocVmId];
    }

    bool JSBridgeIOS::createJSContext(uint32_t runTimeId, uint32_t contextId) {
        NSNumber* ocVmId =convertId(runTimeId);
        NSNumber* ocContextId = convertId(contextId);
        return  [JSBridgeIOSImpl createJSContext:ocContextId atVM:ocVmId];
    }

    void JSBridgeIOS::destroyJSContext(uint32_t runTimeId, uint32_t contextId) {
        NSNumber* ocVmId =convertId(runTimeId);
        NSNumber* ocContextId = convertId(contextId);
        [JSBridgeIOSImpl destroyJSContext:ocContextId atVM:ocVmId];
    }
    
    JSValue* doExecJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName,wson_buffer* args){
        NSNumber* ocVmId =convertId(runTimeId);
        NSNumber* ocContextId = convertId(contextId);
        NSString* ocMethod=[[NSString alloc] initWithUTF8String:methodName];
        id jsArgs = [WsonParser toVal:args];
        if (![jsArgs isKindOfClass:[NSArray class]]) {
            return nil;
        }
        NSArray* ocArrayArgs = (NSArray*)jsArgs;
        return [JSBridgeIOSImpl doExecJSMethodInContext:ocContextId atVM:ocVmId method:ocMethod withArgs:ocArrayArgs];
    }

    void JSBridgeIOS::execJSMethod(uint32_t runTimeId, uint32_t contextId, char *methodName,wson_buffer* args) {
        doExecJSMethod(runTimeId,contextId,methodName,args);
    }
    
    wson_buffer* JSBridgeIOS::execJSMethodWithResult(uint32_t runTimeId, uint32_t contextId, char *methodName, wson_buffer* args){
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
