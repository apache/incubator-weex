/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
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

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>

@class WXBridgeMethod;
@class WXSDKInstance;

#ifdef __cplusplus
extern "C" {
#endif
    void WXPerformBlockOnBridgeThread(void (^block)(void));
    void WXPerformBlockSyncOnBridgeThread(void (^block) (void));
#ifdef __cplusplus
}
#endif

@interface WXBridgeManager : NSObject

/**
 *  return instance at the top of the stack.
 **/
@property (nonatomic, weak, readonly) WXSDKInstance *topInstance;

/**
 *  Create Instance Method
 *  @param instance  :   instance id
 *  @param temp  :   template data
 *  @param options   :   parameters
 *  @param data      :   external data
 **/
- (void)createInstanceForJS:(NSString *)instance
              template:(NSString *)temp
               options:(NSDictionary *)options
                  data:(id)data;

/**
 *  Create Instance Method
 *  @param instance  :   instance id
 *  @param temp  :   template data
 *  @param options   :   parameters
 *  @param data      :   external data
 **/
- (void)createInstance:(NSString *)instance
              template:(NSString *)temp
               options:(NSDictionary *)options
                  data:(id)data;

/**
 *  Create Instance with opcode
 *  @param instance  :   instance id
 *  @param contents  :   opcode data
 *  @param options   :   parameters
 *  @param data      :   external data
 **/
- (void)createInstance:(NSString *)instance
              contents:(NSData *)contents
               options:(NSDictionary *)options
                  data:(id)data;

/**
 * @abstract return currentInstanceId
 **/
- (NSArray *)getInstanceIdStack;

/**
 *  Destroy Instance Method
 *  @param instance  :   instance id
 **/
- (void)destroyInstance:(NSString *)instance;

/**
 * Trigger full GC, for dev and debug only.
 **/
- (void)forceGarbageCollection;

/**
 *  Refresh Instance Method
 *  @param instance  :   instance id
 *  @param data      :   external data
 **/
- (void)refreshInstance:(NSString *)instance data:(id)data;

/**
 *  Unload
 **/
- (void)unload;

/**
 *  Update Instance State Method
 *  @param instance  :   instance id
 *  @param data      :   parameters
 **/
- (void)updateState:(NSString *)instance data:(id)data;

/**
 *  Execute JSFramework Script
 *  @param script    :   script code
 **/
- (void)executeJsFramework:(NSString *)script;

/**
 + *  download JS Script
 + *  @param instance    :   instance id
 + *  @param scriptUrl   :   script url
 + **/
- (void)DownloadJS:(NSString *)instance url:(NSURL *)scriptUrl completion:(void (^)(NSString *script))complection;

/**
 *  Register JS service Script
 *  @param name      :   service name
 *  @param serviceScript    :   script code
 *  @param options   :   service options
 *  @param completion : completion callback
 **/
- (void)registerService:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options completion:(void(^)(BOOL result))completion;


/**
 *  Register JS service Script
 *  @param name         :   service name
 *  @param serviceScriptUrl    :   script url
 *  @param options      :   service options
 *  @param completion : completion callback
 **/
-(void)registerService:(NSString *)name withServiceUrl:(NSURL *)serviceScriptUrl withOptions:(NSDictionary *)options completion:(void(^)(BOOL result))completion;

/**
 *  Unregister JS service Script
 *  @param name    :   script code
 **/
- (void)unregisterService:(NSString *)name;

/**
 *  Register Modules Method
 *  @param modules   :   module list
 **/
- (void)registerModules:(NSDictionary *)modules;

/**
 *  Register Components Method
 *  @param components   component list
 **/
- (void)registerComponents:(NSArray* )components;

/**
 *  FireEvent
 *  @param instanceId   instance id
 *  @param ref       :   node reference
 *  @param type      :   event type
 *  @param params    :   parameters in event object
 *  @param domChanges   dom value changes, used for two-way data binding
 **/
- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges;

/**
 *  FireEvent
 *  @param instanceId   instance id
 *  @param ref       :   node reference
 *  @param type      :   event type
 *  @param params    :   parameters in event object
 *  @param domChanges   dom value changes, used for two-way data binding
 *  @param handlerArguments : arguments passed to event handler
 **/
- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges handlerArguments:(NSArray *)handlerArguments;

- (JSValue *)fireEventWithResult:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges;

/**
 * componentHook
 * @param instanceId  : instance id
 * @param componentId : compoent id
 * @param type        : component hook Type, such as life-cycle
 * @param hookPhase   : hook phase
 */
- (void)callComponentHook:(NSString*)instanceId componentId:(NSString*)componentId type:(NSString*)type hook:(NSString*)hookPhase args:(NSArray*)args competion:(void (^)(JSValue * value))complection;
/**
 *  callBack
 *
 *  @param instanceId instanceId
 *  @param funcId     funcId
 *  @param params     params
 *  @param keepAlive     indicate that whether this func will be reused
 */
- (void)callBack:(NSString *)instanceId funcId:(NSString *)funcId params:(id)params keepAlive:(BOOL)keepAlive;

/**
 *  Connect To WebSocket for devtool debug
 *  @param url       :   url to connect
 **/
- (void)connectToDevToolWithUrl:(NSURL *)url;

/**
 *  CallBack
 *  @param instanceId   instance id
 *  @param funcId    :   callback id
 *  @param params    :   parameters
 **/
- (void)callBack:(NSString *)instanceId funcId:(NSString *)funcId params:(id)params;

/**
 *  Connect To WebSocket for collecting log
 *  @param url       :   url to connect
 **/
- (void)connectToWebSocket:(NSURL *)url;

/**
 *  Log To WebSocket
 *  @param flag      :   the tag to identify
 *  @param message   :   message to output
 **/
- (void)logToWebSocket:(NSString *)flag message:(NSString *)message;

/**
 *  Reset Environment
 **/
- (void)resetEnvironment;

- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params DEPRECATED_MSG_ATTRIBUTE("Use fireEvent:ref:type:params:domChanges: method instead.");
- (void)executeJsMethod:(WXBridgeMethod *)method DEPRECATED_MSG_ATTRIBUTE();

- (void)callJSMethod:(NSString *)method args:(NSArray *)args;

@end
