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

NS_ASSUME_NONNULL_BEGIN

@class WXSDKInstance;

@interface WXSDKEngine : NSObject

/**
 *  @abstract Register default modules/components/handlers, they will be registered only once.
 **/
+ (void)registerDefaults;

/**
 *  @abstract Register a module for a given name
 *
 *  @param name The module name to register
 *
 *  @param clazz  The module class to register
 *
 **/
+ (void)registerModule:(NSString *)name withClass:(Class)clazz;

/**
 * @abstract Registers a component for a given name
 *
 * @param name The component name to register
 *
 * @param clazz The WXComponent subclass to register
 *
 **/
+ (void)registerComponent:(NSString *)name withClass:(Class)clazz;

/**
 * @abstract Registers a extendCallNative Class for a given name
 *
 * @param name The extendCallNative name to register
 *
 * @param clazz The extendCallNative subclass to register
 *
 **/
+ (void)registerExtendCallNative:(NSString *)name withClass:(Class)clazz;

/**
 * @abstract Registers a component for a given name and specific properties
 *
 * @param name The component name to register
 *
 * @param clazz The WXComponent subclass to register
 *
 * @param properties properties to apply to the component
 *
 */
+ (void)registerComponent:(NSString *)name withClass:(Class)clazz withProperties:(NSDictionary * _Nullable)properties;

/**
 * @abstract Registers a component for a given name, options and js code
 *
 * @param name The service name to register
 *
 * @param options The service options to register
 *
 * @param serviceScript service js code to invoke
 *
 */
+ (void)registerService:(NSString *)name withScript:(NSString *)serviceScript withOptions:(NSDictionary * _Nullable)options;

/**
 * @abstract Registers a component for a given name, options and js code
 *
 * @param name The service name to register
 *
 * @param options The service options to register
 *
 * @param serviceScript service js code to invoke
 *
 * @param completion Completion callback. JS is executed in asynchronously.
 *
 */
+ (void)registerService:(NSString *)name withScript:(NSString *)serviceScript withOptions:(NSDictionary * _Nullable)options completion:(nullable void(^)(BOOL result))completion;

/**
 * @abstract Registers a component for a given name, options and js url
 *
 * @param name The service name to register
 *
 * @param options The service options to register
 *
 * @param serviceScriptUrl The service url to register
 *
 */
+ (void)registerService:(NSString *)name withScriptUrl:(NSURL *)serviceScriptUrl withOptions:(NSDictionary * _Nullable)options;

/**
 * @abstract Registers a component for a given name, options and js url
 *
 * @param name The service name to register
 *
 * @param options The service options to register
 *
 * @param serviceScriptUrl The service url to register
 *
 * @param completion Completion callback. JS is executed in asynchronously.
 *
 */
+ (void)registerService:(NSString *)name withScriptUrl:(NSURL *)serviceScriptUrl withOptions:(NSDictionary * _Nullable)options completion:(nullable void (^)(BOOL result))completion;

/**
 * @abstract Registers a component for a given name, options and js code
 *
 * @param name The name of register service
 *
 */
+ (void)unregisterService:(NSString *)name;

/**
 * @abstract Registers a handler for a given handler instance and specific protocol
 *
 * @param handler The handler instance to register
 *
 * @param protocol The protocol to confirm
 *
 */
+ (void)registerHandler:(id)handler withProtocol:(Protocol *)protocol;


/**
 * @abstract Returns a given handler instance for specific protocol
 *
 * @param protocol The protocol to confirm
 *
 */
+ (id)handlerForProtocol:(Protocol *)protocol;

/**
 * @abstract Initializes the global sdk environment
 *
 * @discussion Injects main.js in app bundle as default JSFramework script.
 *
 **/
+ (void)initSDKEnvironment;

/**
 * @abstract Initializes the environment with a given JSFramework script.
 *
 **/
+ (void)initSDKEnvironment:(NSString *)script;

/**
 * @abstract Unloads the bridge context
 *
 **/
+ (void)unload;

/**
 * @abstract restart Weex Engine.
 **/
+ (void)restart;

/**
 * @abstract restart Weex Engine with specify jsfm.
 **/
+ (void)restartWithScript:(NSString*)script;

/**
 * @abstract Returns the version of SDK
 *
 **/
+ (NSString*)SDKEngineVersion;

/**
 * @abstract The Instance at the top of the rendering stack. 
 *
 **/
+ (WXSDKInstance *)topInstance;

/**
 * @abstract Add custom environment variables
 * @discuss These variables can be obtained by $getConfig().env
 *
 **/
+ (void)setCustomEnvironment:(NSDictionary *)environment;
+ (NSDictionary *)customEnvironment;

/**
 * @abstract Connects to websocket for collecting log
 *
 * @param URL The URL of websocket to connect
 *
 */
+ (void)connectDebugServer:(NSString*)URL;

/**
 * @abstract Connects to websocket for devtool debug
 *
 * @param URL The URL of websocket to connect
 *
 */
+ (void)connectDevToolServer:(NSString *)URL;

/**
 * @abstract Set the global device size of all pages.
 * You could also set unique device size used by a page using interface of WXSDKInstance.
 *
 * @param size, the device size in UIKit system.
 *
 */
+ (void)setGlobalDeviceSize:(CGSize)size;

/**
 * @abstract Get the current global device size used by pages.
 *
 */
+ (CGSize)getGlobalDeviceSize;

@end

@interface WXSDKEngine (Deprecated)

+ (void)initSDKEnviroment DEPRECATED_MSG_ATTRIBUTE("To fix typo, use initSDKEnvironment method instead.");
+ (void)initSDKEnviroment:(NSString *)script DEPRECATED_MSG_ATTRIBUTE("To fix typo,  use initSDKEnvironment: method instead.");

@end

NS_ASSUME_NONNULL_END
