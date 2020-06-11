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

@class JSContext;

@protocol WXReactorProtocol <NSObject>

@required

/**
Weex should register a JSContext to reactor
*/
- (void)registerJSContext:(NSString *)instanceId;

/**
 Reactor execute js source
*/
- (void)render:(NSString *)instanceId source:(NSString*)source data:(NSDictionary* _Nullable)data;

- (void)unregisterJSContext:(NSString *)instanceId;

/**
 When js call Weex NativeModule, invoke callback function
 
 @param instanceId : weex instance id
 @param callbackId : callback function id
 @param args       : args
*/
- (void)invokeCallBack:(NSString *)instanceId function:(NSString *)callbackId args:(NSArray * _Nullable)args;

/**
Native event to js
 
@param instanceId :   instance id
@param ref        :   node reference
@param event      :   event type
@param args       :   parameters in event object
@param domChanges :  dom value changes, used for two-way data binding
*/
- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref event:(NSString *)event args:(NSDictionary * _Nullable)args domChanges:(NSDictionary * _Nullable)domChanges;

@end

NS_ASSUME_NONNULL_END
